#pragma once

#include "log.hpp"
#include<exception>
#include <string>
#include <iostream>
#include <cstdio>
#include <memory>
#include <fstream>
#include <utility>
#include <netinet/in.h>

namespace wind
{
    typedef struct sockaddr_in EndpointIdType;
    typedef long long UniqueIdType;
    typedef std::string string;
    typedef std::fstream fstream;

    enum ErrorCode
    {
        FILE_OPEN_ERROR = 1,
        FILE_FORMAT_ERROR = 2,
        INVAL_CFG_ERROR = 3,
        SOCKET_CREATE_ERROR = 4,
        BIND_PORT_ERROR = 5,
        LISTEN_SOCK_ERROR = 6,
        SOCKET_INIT_ERROR = 7,
    };

    enum TaskCode
    {
        CREATE_NEW_ACCOUNT = 1,

    };


    struct Pack_Header
    {
        UniqueIdType _source;
        UniqueIdType _destination;
        TaskCode _task;
    };

    struct Pack_Message
    {
        Pack_Header _header;
        std::string _payload;
    };


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
}

// bool operator<(const sockaddr_in &lhs, const sockaddr_in &rhs)
// {
//     if (lhs.sin_addr.s_addr != rhs.sin_addr.s_addr)
//     {
//         return lhs.sin_addr.s_addr < rhs.sin_addr.s_addr;
//     }
//     return lhs.sin_port < rhs.sin_port;
// }

bool operator<(const struct in_addr& a, const struct in_addr& b)
{
    return a.s_addr < b.s_addr;
}