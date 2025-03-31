#pragma once

#include "log.hpp"
#include <string>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_PORT     8888
#define DEFAULT_ADDRESS  "0.0.0.0"

namespace wind
{
    enum{
        SOCKERROR = 2,
        BINDERROR = 3,
    };

    class tcpserver
    {
        typedef std::string string;
        public:
        tcpserver(const int16_t& port = DEFAULT_PORT, const char* ip = DEFAULT_ADDRESS) :_port(port), _ip(ip){}
        ~tcpserver() {close(_sockfd);}

        void init()
        {
            socket_init();

            struct sockaddr_in local;
            sockaddr_in__init(local);

            bind_port(local);
        }

        void run()
        {

        }

        private:
        inline void socket_init()
        {
            _sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(_sockfd < 0)
            {
                _log(Fatal, "socket create error: %s", strerror(errno));
                exit(SOCKERROR);
            }
            _log(Info, "socket create success, sockfd: %d", _sockfd);
        }

        inline void sockaddr_in__init(sockaddr_in& local)
        {
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            inet_aton(_ip.c_str(), &(local.sin_addr));
            _log(Info, "socket init success");
        }

        inline void bind_port(const sockaddr_in& local)
        {
            if(bind(_sockfd, reinterpret_cast<const sockaddr*>(&local), static_cast<socklen_t>(sizeof(local))) != 0)
            {
                _log(Fatal, "bind error: %s", strerror(errno));
                exit(BINDERROR);
            }
            _log(Info, "bind success");
        }

        private:
        int _sockfd;
        string _ip;
        int16_t _port;
        static Log _log;
    };
}