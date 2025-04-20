#pragma once

#include "log.hpp"
#include "task.hpp"
#include "dict.hpp"
#include "daemon.hpp"
#include <string>
#include <memory>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "thread_pool.hpp"

#include<fstream>
#include<sstream>

// std::fstream fs ("/dev/pts/8", std::fstream::out);
// std::stringstream  oos;

#define DEFAULT_PORT 8888
#define DEFAULT_ADDRESS "0.0.0.0"
#define DEFAULT_BACKLOG 1
// #define BUFFER_SIZE      1024

namespace wind
{
    enum
    {
        PARAMERROR = 1,
        SOCKERROR = 2,
        BINDERROR = 3,
        LISTENERROR = 4,
        FROKERROR = 5
    };

    class tcpserver
    {
        typedef std::string string;
        typedef thread_pool<task> threadPool;

    public:
        tcpserver(const int16_t &port = DEFAULT_PORT, const char *ip = DEFAULT_ADDRESS) : _port(port), _ip(ip), _log(Log::getInstance()) {}
        ~tcpserver() { close(_listeningSockfd); }

        void init()
        {
            // daemon();

            // signal(SIGPIPE, SIG_IGN);

            // 线程池已经改为单例模式
            threadPool::getInstance().start();

            socket_init();

            int opt = 1;
            // 防止服务端异常退出后出现不能立即重启的情况    复用地址  和    端口      讲原理的时候再细说
            setsockopt(_listeningSockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

            struct sockaddr_in local;
            sockaddr_in__init(local);

            bind_port(local);

            start_listening();

            dict::getInstance();
        }

        void run()
        {
            // fs << "hello"<<std::endl;
            // daemon();
            // print_netstat();
            // fs<<"tcpserver is running...."<<"\n";
            _log(Info, "tcpserver is running....");
            while (true)
            {
                int sockfd = 0;
                struct sockaddr_in client;
                if (accept_connection(sockfd, client) == -1)
                    continue;

                uint16_t port = ntohs(client.sin_port);
                char ipbuff[32];
                inet_ntop(AF_INET, &client.sin_addr, ipbuff, sizeof(ipbuff));

                // service(sockfd, ipbuff, port);    // 单进程版

                // service2(sockfd, ipbuff, port);   // 多进程版

                // service3(sockfd, ipbuff, port);   // 多线程基础版

                service4(sockfd, ipbuff, port); // 线程池
            }
        }

    private:
        // void print_netstat()
        // {
        //     FILE *pipe = popen("netstat -nltp", "r");
        //     if (!pipe)
        //     {
        //         std::cerr << "Error: Failed to run netstat -nltp" << std::endl;
        //         return;
        //     }

        //     std::unique_ptr<FILE, decltype(&pclose)> pipe_guard(pipe, &pclose);

        //     oos<<"\n";
        //     char buffer[1024];
        //     while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        //     {
        //         oos << buffer;
        //     }

        //     FILE *pipe1 = popen("ps ajx | head -1 && ps ajx | grep tcpserver | grep -v grep", "r");
        //     if (!pipe1)
        //     {
        //         std::cerr << "Error: Failed to run netstat -nltp" << std::endl;
        //         return;
        //     }

        //     std::unique_ptr<FILE, decltype(&pclose)> pipe_guard1(pipe1, &pclose);

        //     oos<<"\n";
        //     char buffer1[1024];
        //     while (fgets(buffer1, sizeof(buffer1), pipe1) != nullptr)
        //     {
        //         oos << buffer1;
        //     }


        //     fs<<oos.str()<<std::endl;;
        //     oos.clear();
        // }

        inline void socket_init()
        {
            _listeningSockfd = socket(AF_INET, SOCK_STREAM, 0);
            // std::cout << _listeningSockfd << std::endl;
            if (_listeningSockfd < 0)
            {
                _log(Fatal, "socket create error: %s", strerror(errno));
                exit(SOCKERROR);
            }
            _log(Info, "socket create success, sockfd: %d", _listeningSockfd);
        }

        inline void sockaddr_in__init(sockaddr_in &local)
        {
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            inet_aton(_ip.c_str(), &(local.sin_addr));
            _log(Info, "socket init success");
        }

        inline void bind_port(sockaddr_in &local)
        {
            // std::cout << _listeningSockfd << std::endl;
            if (bind(_listeningSockfd, (struct sockaddr *)(&local), static_cast<socklen_t>(sizeof(local))) != 0)
            {
                _log(Fatal, "bind error: %s", strerror(errno));
                exit(BINDERROR);
            }
            _log(Info, "bind success");
        }

        inline void start_listening()
        {
            if (listen(_listeningSockfd, DEFAULT_BACKLOG) == -1)
            {
                _log(Fatal, "listen error: %s", strerror(errno));
                exit(LISTENERROR);
            }
            _log(Info, "start listening");
        }

        inline int accept_connection(int &sockfd, struct sockaddr_in &client)
        {
            // while(1);
            socklen_t len = static_cast<socklen_t>(sizeof(client));
            sockfd = accept(_listeningSockfd, reinterpret_cast<struct sockaddr *>(&client), &len);
            if (sockfd < 0)
            {
                _log(Warning, "accert error: %s", strerror(errno));
                return -1;
            }
            _log(Info, "accept a new connection, sockfd: %d", sockfd);

            return 0;
        }

        inline void service(int sockfd, const char *ip, uint16_t port)
        {
            char buff[BUFFER_SIZE];
            while (true)
            {
                ssize_t len = read(sockfd, buff, sizeof(buff) - 1);
                if (len > 0)
                {
                    buff[len] = 0;
                    std::cout << "client say# " << buff << std::endl;

                    std::string echo("tcpserver echo# ");
                    echo += buff;

                    write(sockfd, echo.c_str(), echo.size());
                }
                else if (len == 0)
                {
                    _log(Info, "user quit, close sockfd: %d", sockfd);
                    close(sockfd);
                    break;
                }
                else
                {
                    _log(Warning, "read error: %s", strerror(errno));
                    close(sockfd);
                    break;
                }
            }
        }

        inline void service2(int sockfd, const char *ip, uint16_t port)
        {
            pid_t id = fork();
            if (id == 0)
            {
                close(_listeningSockfd);
                if (fork() > 0)
                    exit(0);
                service(sockfd, ip, port);
                exit(0);
            }
            else if (id > 0)
            {
                close(sockfd);
                waitpid(id, nullptr, 0);
            }
            else
            {
                _log(Fatal, "fork error: %s", strerror(errno));
                exit(FROKERROR);
            }
            _log(Debug, "fscsdcd");
        }

        struct Args
        {
            int fd_;
            string ip_;
            uint16_t port_;
            tcpserver *obj_;
        };

        inline void service3(const int &sockfd, const char *ip, const uint16_t &port)
        {
            auto args_ = new Args;
            args_->fd_ = sockfd;
            args_->ip_ = ip;
            args_->port_ = port;
            args_->obj_ = this;

            pthread_t t;
            pthread_create(&t, nullptr, interaction, args_);
        }

        static void *interaction(void *args_)
        {
            pthread_detach(pthread_self());
            std::shared_ptr<Args> args(reinterpret_cast<Args *>(args_));
            args->obj_->service(args->fd_, args->ip_.c_str(), args->port_);
            return nullptr;
        }

        inline void service4(const int &sockfd, const char *ip, const uint16_t &port)
        {
            threadPool::getInstance().push(sockfd, ip, port);
        }

    private:
        int _listeningSockfd;
        string _ip;
        int16_t _port;
        Log &_log;
    };
}
