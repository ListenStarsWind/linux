#pragma once

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <signal.h>
#include "Log.hpp"
#include "ThreadPool.hpp"
#include "Task.hpp"
#include "Daemon.hpp"

const int defaultfd = -1;
const std::string defaultip = "0.0.0.0";
const int backlog = 10; // 但是一般不要设置的太大
extern Log lg;

enum
{
    UsageError = 1,
    SocketError,
    BindError,
    ListenError,
};

class TcpServer;

class ThreadData
{
public:
    ThreadData(int fd, const std::string &ip, const uint16_t &p, TcpServer *t): sockfd(fd), clientip(ip), clientport(p), tsvr(t)
    {}
public:
    int sockfd;
    std::string clientip;
    uint16_t clientport;
    TcpServer *tsvr;
};

class TcpServer
{
public:
    TcpServer(const uint16_t &port, const std::string &ip = defaultip) : listensock_(defaultfd), port_(port), ip_(ip)
    {
    }
    void InitServer()
    {
        listensock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listensock_ < 0)
        {
            lg(Fatal, "create socket, errno: %d, errstring: %s", errno, strerror(errno));
            exit(SocketError);
        }
        lg(Info, "create socket success, listensock_: %d", listensock_);

        int opt = 1;
        setsockopt(listensock_, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &opt, sizeof(opt)); // 防止偶发性的服务器无法进行立即重启(tcp协议的时候再说)

        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port_);
        inet_aton(ip_.c_str(), &(local.sin_addr));
        // local.sin_addr.s_addr = INADDR_ANY;

        if (bind(listensock_, (struct sockaddr *)&local, sizeof(local)) < 0)
        {
            lg(Fatal, "bind error, errno: %d, errstring: %s", errno, strerror(errno));
            exit(BindError);
        }

        lg(Info, "bind socket success, listensock_: %d", listensock_);

        // Tcp是面向连接的，服务器一般是比较“被动的”，服务器一直处于一种，一直在等待连接到来的状态
        if (listen(listensock_, backlog) < 0)
        {
            lg(Fatal, "listen error, errno: %d, errstring: %s", errno, strerror(errno));
            exit(ListenError);
        }

        lg(Info, "listen socket success, listensock_: %d", listensock_);
    }

    // static void *Routine(void *args)
    // {
    //     pthread_detach(pthread_self());
    //     ThreadData *td = static_cast<ThreadData *>(args);
    //     td->tsvr->Service(td->sockfd, td->clientip, td->clientport);//???
    //     delete td;
    //     return nullptr;
    // }

    void Start()
    {
        Daemon();
        ThreadPool<Task>::GetInstance()->Start();
        // for fork();
        // signal(SIGCHLD, SIG_IGN);
        lg(Info, "tcpServer is running....");
        for (;;)
        {
            // 1. 获取新连接
            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            int sockfd = accept(listensock_, (struct sockaddr *)&client, &len);
            if (sockfd < 0)
            {
                lg(Warning, "accept error, errno: %d, errstring: %s", errno, strerror(errno)); //?
                continue;
            }
            uint16_t clientport = ntohs(client.sin_port);
            char clientip[32];
            inet_ntop(AF_INET, &(client.sin_addr), clientip, sizeof(clientip));

            // 2. 根据新连接来进行通信
            lg(Info, "get a new link..., sockfd: %d, client ip: %s, client port: %d", sockfd, clientip, clientport);
            // std::cout << "hello world" << std::endl;
            // version 1 -- 单进程版
            // Service(sockfd, clientip, clientport);
            // close(sockfd);

            // version 2 -- 多进程版
            // pid_t id = fork();
            // if(id == 0)
            // {
            //     // child
            //     close(listensock_);
            //     if(fork() > 0) exit(0);
            //     Service(sockfd, clientip, clientport); //孙子进程， system 领养
            //     close(sockfd);
            //     exit(0);
            // }
            // close(sockfd);
            // // father
            // pid_t rid = waitpid(id, nullptr, 0);
            // (void)rid;

            // version 3 -- 多线程版本
            // ThreadData *td = new ThreadData(sockfd, clientip, clientport, this);
            // pthread_t tid;
            // pthread_create(&tid, nullptr, Routine, td);

            // version 4 --- 线程池版本
            Task t(sockfd, clientip, clientport);
            ThreadPool<Task>::GetInstance()->Push(t);
        }
    }
    // void Service(int sockfd, const std::string &clientip, const uint16_t &clientport)
    // {
    //     // 测试代码
    //     char buffer[4096];
    //     while (true)
    //     {
    //         ssize_t n = read(sockfd, buffer, sizeof(buffer));
    //         if (n > 0)
    //         {
    //             buffer[n] = 0;
    //             std::cout << "client say# " << buffer << std::endl;
    //             std::string echo_string = "tcpserver echo# ";
    //             echo_string += buffer;

    //             write(sockfd, echo_string.c_str(), echo_string.size());
    //         }
    //         else if (n == 0)
    //         {
    //             lg(Info, "%s:%d quit, server close sockfd: %d", clientip.c_str(), clientport, sockfd);
    //             break;
    //         }
    //         else
    //         {
    //             lg(Warning, "read error, sockfd: %d, client ip: %s, client port: %d", sockfd, clientip.c_str(), clientport);
    //             break;
    //         }
    //     }
    // }
    ~TcpServer() {}

private:
    int listensock_;
    uint16_t port_;
    std::string ip_;
};
