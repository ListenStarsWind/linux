#pragma once

#include "log.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <memory>

enum socket_errno
{
    CREATE_ERROR = 1,
    BIND_ERROR = 2,
    LISTEN_ERROR = 3,
    ACCECT_ERROR = 4
};

using namespace wind;

class socket_
{
public:
    socket_() {}
    ~socket_()
    {
        if (_sockfd > 0)
            close_();
    }

    void create_()
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            _log(Fatal, "socket create error: %s", strerror(errno));
            exit(CREATE_ERROR);
        }
    }

    void bind_(uint16_t port = 8888)
    {
        struct sockaddr_in sock_in;
        memset(&sock_in, 0, sizeof(sock_in));
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons(port);
        sock_in.sin_addr.s_addr = INADDR_ANY;
        if (bind(_sockfd, reinterpret_cast<const struct sockaddr *>(&sock_in), static_cast<socklen_t>(sizeof(sock_in))) != 0)
        {
            _log(Fatal, "bind error: %s", strerror(errno));
            exit(BIND_ERROR);
        }
    }

    void listen_()
    {
        if (listen(_sockfd, backlog) != 0)
        {
            _log(Fatal, "listen error: %s", strerror(errno));
            exit(LISTEN_ERROR);
        }
    }

    void connect_(uint16_t port, const char *addr)
    {
        struct sockaddr_in sock_in;
        memset(&sock_in, 0, sizeof(sock_in));
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons(port);
        if (inet_pton(AF_INET, addr, &sock_in.sin_addr) != 1)
        {
            _log(Fatal, "connect error: %s", strerror(errno));
            exit(ACCECT_ERROR);
        }
        if (connect(_sockfd, reinterpret_cast<const struct sockaddr *>(&sock_in), static_cast<socklen_t>(sizeof(sock_in))) != 0)
        {
            _log(Fatal, "connect error: %s", strerror(errno));
            exit(ACCECT_ERROR);
        }
    }

    int accept_(std::string *clientip, uint16_t *clientport)
    {
        struct sockaddr_in sock_in;
        socklen_t len = static_cast<socklen_t>(sizeof(sock_in));
        int fd = accept(_sockfd, reinterpret_cast<struct sockaddr *>(&sock_in), &len);
        if (fd < 0)
        {
            _log(Warning, "accept error: ", strerror(errno));
            return -1;
        }
        char buffer[64];
        inet_ntop(AF_INET, &sock_in.sin_addr, buffer, sizeof(buffer));
        *clientip = buffer;
        *clientport = ntohs(sock_in.sin_port);
        return fd;
    }

    void reuse_port_address()
    {
        int opt = 1;
        if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        {
            _log(Warning, "set socket error: %s", strerror(errno));
        }
    }

    operator int()
    {
        return _sockfd;
    }

    void close_()
    {
        close(_sockfd);
        _sockfd = -1;
    }

private:
    int _sockfd;
    Log &_log = Log::getInstance();
    static int backlog;
};

int socket_::backlog = 10;

void print_netstat()
{
    FILE *pipe = popen("netstat -nltp", "r");
    if (!pipe)
    {
        std::cerr << "Error: Failed to run netstat -nltp" << std::endl;
        return;
    }

    std::unique_ptr<FILE, decltype(&pclose)> pipe_guard(pipe, &pclose);

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        std::cout << buffer;
    }
}