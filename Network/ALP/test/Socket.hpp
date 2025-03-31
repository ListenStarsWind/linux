#pragma once

#include "log.hpp"
#include "type.h"
#include <vector>
#include <string.h>
#include <unordered_map>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include "PacketHandler.hpp"
#include "AccountManager.hpp"

namespace wind
{
    class Socket_Base
    {
        typedef std::unordered_map<UniqueIdType, int> OnlineListType;

    protected:
        virtual void init() = 0;
        Socket_Base(AccountManager_Base &manager, const char *port) : _manager(manager), _packet(PacketProcessor_Server::getInstance()), _port(port) {}

        inline void socket_init()
        {
            _sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (_sockfd < 0)
            {
                _log(Fatal, "create socket error: %s", strerror(errno));
                exit(SOCKET_CREATE_ERROR);
            }
            // _log(Info, "create socket succsee: %d", _sockfd);
        }

        inline void AccountManager_init()
        {
            _manager.init();
        }

        inline void reuse_port_address()
        {
            int opt = 1;
            if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
            {
                _log(Warning, "set socket error: %s", strerror(errno));
            }
        }

        inline const int &get_raw_socket_fd()
        {
            return _sockfd;
        }

        inline void bind_port(const EndpointIdType &sock_in)
        {
            if (bind(_sockfd, reinterpret_cast<const struct sockaddr *>(&sock_in), static_cast<socklen_t>(sizeof(sock_in))) != 0)
            {
                _log(Fatal, "bind error: %s", strerror(errno));
                exit(BIND_PORT_ERROR);
            }
            // _log(Info, "bind success");
        }

        inline void start_listening()
        {
            if (listen(_sockfd, DEFAULT_BACKLOG) == -1)
            {
                _log(Fatal, "listen error: %s", strerror(errno));
                exit(LISTEN_SOCK_ERROR);
            }
            // _log(Info, "start listening");
        }

        inline EndpointIdType list_sock_init(UniqueIdType userID)
        {
            EndpointIdType result;
            uint16_t port = std::stoi(_port);
            result.sin_family = AF_INET;
            result.sin_addr = query(userID);
            result.sin_port = htons(port);
            return result;
        }

        inline struct in_addr query(UniqueIdType userID)
        {
            return _manager.query_left(userID);
        }

        std::string sockaddr_to_string(const struct sockaddr_in &addr)
        {
            // 将 IP 地址从二进制转换为字符串
            char ip_str[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN) == nullptr)
            {
                return "Invalid IP address";
            }

            // 获取端口号（网络字节序转换为主机字节序）
            uint16_t port = ntohs(addr.sin_port);

            // 拼接 IP 和端口
            return std::string(ip_str) + ":" + std::to_string(port);
        }

        // 套接字层
        inline UniqueIdType app_first_handshake()
        {
            EndpointIdType client;
            socklen_t len = static_cast<socklen_t>(sizeof(client));
            int sockfd = accept(_sockfd, reinterpret_cast<struct sockaddr *>(&client), &len);
            if (sockfd < 0)
            {
                _log(Warning, "accept error: %s", strerror(errno));
                return -1;
            }

            UniqueIdType uniqueId = 0;
            string hi;
            char buffer[BUFFER_SIZE];
            string hello;
            while (true)
            {
                ssize_t size = read(sockfd, buffer, sizeof(buffer) - 1);
                if (size <= 0)
                {
                    // 读出错, 或者一直收到错误信息, 一直没有握手, 视为失败
                    const char *temp = size == 0 ? "用户断连" : strerror(errno);
                    _log(Warning, "read error: %s", temp);
                    close(sockfd);
                    return -1;
                }

                buffer[size] = '\0';
                hello += buffer;
                try
                {
                    std::cout << hello << std::endl;
                    hi = _packet.app_first_handshake(hello);
                    uniqueId = std::stoi(hi);
                    break;
                }
                catch (const std::exception &e)
                {
                    // 没有完整握手继续接收
                    _log(Warning, "%s", e.what());
                }
            }

            // 告诉客户端自己的账号
            ssize_t size = write(sockfd, hi.c_str(), hi.size());
            if (size < 0)
            {
                _log(Warning, "write error: %s", strerror(errno));
                close(sockfd);
                return -1;
            }

            // 第一次握手结束, 将关键数据扔进哈希表
            _onlines.emplace(uniqueId, sockfd);

            // 返回句柄
            return uniqueId;
        }

        inline void disconnect(UniqueIdType userID)
        {
            int fd = _onlines[userID];
            // _log(Debug, "close, %d", fd);
            _onlines.erase(fd);
            close(fd);
        }

        // inline int accept_connection(int& sockfd, EndpointIdType& client)
        // {
        //     socklen_t len = static_cast<socklen_t>(sizeof(client));
        //     sockfd = accept(_sockfd, reinterpret_cast<struct sockaddr*>(&client), &len);
        //     if(sockfd < 0)
        //     {
        //         _log(Warning, "accert error: %s", strerror(errno));
        //         return -1;
        //     }
        //     _log(Info, "accept a new connection, sockfd: %d", sockfd);

        //     return 0;
        // }

    private:
        int _sockfd;
        const std::string _port;
        AccountManager_Base &_manager;
        PacketProcessor_Base &_packet;
        OnlineListType _onlines;
        static int DEFAULT_BACKLOG;
        static int BUFFER_SIZE;
        static char SPACE;

    protected:
        Log &_log = Log::getInstance();
    };

    int Socket_Base::DEFAULT_BACKLOG = 5;
    int Socket_Base::BUFFER_SIZE = 4096;
    char Socket_Base::SPACE = '\n';

    class Socket_Server : public Socket_Base
    {
    public:
        Socket_Server(const char *port) : Socket_Base(AccountManager_server::getInstance(), port), _listenSocket(Socket_Base::get_raw_socket_fd()) {}

        void init()
        {
            socket_init();
            AccountManager_init(); // 初始化账号管理器

            EndpointIdType meForServer;
            try
            {
                meForServer = list_sock_init(meForServerID);
            }
            catch (const std::exception &e)
            {
                _log(Fatal, "%s", e.what());
                exit(SOCKET_INIT_ERROR);
            }

            reuse_port_address(); // 地址端口复用
            bind_port(meForServer);
            start_listening();
        }

        void run()
        {
            print_netstat();
            _log(Info, "tcpserver is running....");
            while (true)
            {
                auto i = app_first_handshake();
                if (i == -1)
                    continue;
            }
        }

    private:
        const int &_listenSocket;
        const UniqueIdType meForServerID = 0;
    };

    class Socket_Client : public Socket_Base
    {
    public:
        Socket_Client() : Socket_Base(AccountManager_client::getInstance()), _sockfd(Socket_Base::get_raw_socket_fd()) {}

        void init()
        {
            socket_init();
        }

        private:
        void link_to(const struct sockaddr_in &server)
        {
            struct sockaddr_in
            if (connect(_sockfd, reinterpret_cast<const struct sockaddr *>(&server), static_cast<socklen_t>(sizeof(server))) != 0)
            {
                throw system_error(errno, system_category(), "connect error");
            }
        }

        const int &_sockfd;
    }
}