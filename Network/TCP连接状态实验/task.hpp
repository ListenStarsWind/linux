#pragma once

#include "log.hpp"
#include "dict.hpp"
#include <string>
#include <unistd.h>
#include <iostream>
#include <signal.h>

#include<fstream>
#include<sstream>

#define BUFFER_SIZE      1024

// std::fstream fs ("/dev/pts/8", std::fstream::out);
// std::stringstream  oos;


namespace wind
{
    class task
    {
        typedef std::string string;

    public:
        task() :_log(Log::getInstance()) {}
        task(const int &sockfd, const char *ip, const uint16_t &port) : _sockfd(sockfd), _ip(ip), _port(port), _log(Log::getInstance()) {}
        void operator()()
        {
            // // std::fstream fs1("/dev/pts/8", std::fstream::out);
            // // fs1<<"here1"<<std::endl;
            // // fs << "hello"<<std::endl;
            // char buff[BUFFER_SIZE];
            // // dict::getInstance()
            // // std::cout << "我开始读数据了 " << _sockfd <<std::endl;
            // ssize_t len = read(_sockfd, buff, sizeof(buff) - 1);
            // // std::cout << "我开始读数据了 " << buff <<std::endl;

            // // fs1<<"here2"<<std::endl;

            // if (len > 0)
            // {
            //     buff[len] = 0;
            //     string k(buff);

            //     const string& v = dict::getInstance().translate(k);

            //     // trouble();
            //     // fs << "我开始读数据了 " << v.c_str() <<std::endl;
            //     len = write(_sockfd, v.c_str(), v.size());
            //     // fs << "我开始读数据了 " << v.size() <<std::endl;
            //     if(len < 0)
            //     {
            //         // fs1<<"here3"<<std::endl;
            //         _log(Warning, "write error: %s", strerror(errno));
            //     }
            // }

            
            _log(Info, "user quit, close sockfd: %d", _sockfd);
            close(_sockfd);

            while(1);
        }

        void trouble() {
            FILE* fp = popen("pidof -s tcpclient", "r");
            char buff[16];

            // 如果 fgets 返回 NULL，说明无输出，直接退出
            if (fgets(buff, sizeof(buff), fp) == NULL) {
                pclose(fp);
                return;
            }

            // 去掉末尾 \n
            buff[strlen(buff) - 1] = '\0';

            pid_t id = atoi(buff);
            kill(id, SIGUSR1);

            pclose(fp);

            sleep(5);
        }

    private:
        int _sockfd;
        string _ip;
        uint16_t _port;
        Log& _log;   // Log之前不是单例, 现在也不好改,改了前面不兼容 所以就用这种方式了
    };

}
