#pragma once

#include <arpa/inet.h>   // 引入 htons 接口, inet_aton 接口
#include <netinet/in.h>  // 引入 in_port_t, sockaddr_in
#include <sys/socket.h>  // 引入 socket 接口, AF_INET, SOCK_STREAM, bind 接口, listen 接口, accept 接口, connect 接口

#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <cstdlib>                // 引入 exit 接口
#include <cstring>                // bzero
#include <format>
#include <memory>  // 引入智能指针
#include <string>
#include <system_error>

#include "Connection.hpp"

class tcp_protocol {
   protected:
    using addr = std::string;
    using port = ::in_port_t;
    using connection = Connection;
    using sockaddr = ::sockaddr_in;
    using connect_shared_ptr = connection::self_shared_ptr;
    using connect_weak_ptr = connection::self_weak_ptr;

   public:
    tcp_protocol() : _is_running(false) {}

    virtual ~tcp_protocol() = default;

    virtual void run() {
        start_running();
        stop_running();
    }

    bool is_running() {
        return _is_running;
    }

   protected:
    void start_running() {
        BOOST_LOG_TRIVIAL(info) << std::format("程序正在运行, 已经进入网络IO主逻辑");
        _is_running = true;
    }

    void stop_running() {
        BOOST_LOG_TRIVIAL(info) << std::format("程序退出IO主逻辑");
        _is_running = false;
    }

    static void listen(connect_weak_ptr connect, int backlog = 5) {
        auto socket = connect.lock();
        int ret = ::listen(socket->file(), backlog);
        if (ret < 0) {
            throw std::system_error(
                errno, std::generic_category(),
                std::format("套接字描述符({})设置监听状态失败: ", socket->file()));
        }
        BOOST_LOG_TRIVIAL(info) << std::format("描述符为{}的套接字设置监听状态成功",
                                               socket->file());
    }

    static int accept(connect_weak_ptr connect, sockaddr& user_sockaddr) {
        auto socket = connect.lock();
        socklen_t len = static_cast<socklen_t>(sizeof(user_sockaddr));
        auto user_socket =
            ::accept(socket->file(), reinterpret_cast<::sockaddr*>(&user_sockaddr), &len);
        if (user_socket >= 0) {
            BOOST_LOG_TRIVIAL(info) << std::format("接收到一个描述符为{}的用户连接", user_socket);
        }
        return user_socket;
    }

    static int socket_init() {
        auto socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket < 0) {
            throw std::system_error(errno, std::generic_category(), "套接字打开失败");
        } else {
            BOOST_LOG_TRIVIAL(info) << std::format("套接字打开成功, 文件描述符为: {}", socket);
            int optval = 1;
            if (::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
                BOOST_LOG_TRIVIAL(error) << std::format("设置 套接字复用失败: {}", strerror(errno));
                ::close(socket);
                exit(errno);
            }
        }
        return socket;
    }

    static addr inet_ntop(const sockaddr& sockaddr) {
        char buffer[32];
        ::inet_ntop(AF_INET, &sockaddr.sin_addr, buffer, sizeof(buffer));
        return buffer;
    }

    static void bzero(sockaddr& sockaddr) {
        ::bzero(&sockaddr, sizeof(sockaddr));
    }

    static void sockaddr_in_init(sockaddr& sockaddr, const addr& addr, port port) {
        tcp_protocol::bzero(sockaddr);
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = ::htons(port);
        ::inet_aton(addr.c_str(), &sockaddr.sin_addr);
        BOOST_LOG_TRIVIAL(info) << std::format("拼接了这样的一个套接字: \"{}:{}\"", addr,
                                               std::to_string(port));
    }

    static void bind_port(connect_weak_ptr connect, const sockaddr& sockaddr) {
        auto socket = connect.lock();
        int ret = ::bind(socket->file(), reinterpret_cast<const ::sockaddr*>(&sockaddr),
                         static_cast<::socklen_t>(sizeof(sockaddr)));
        if (ret != 0) {
            throw std::system_error(errno, std::generic_category(),
                                    std::format("套接字({})绑定端口错误: ", socket->file()));
        }

        BOOST_LOG_TRIVIAL(info) << std::format("对于{}套接字描述符已经绑定成功", socket->file());
    }

    static bool connect(connect_weak_ptr connect, const addr& addr = tcp_protocol::_server_addr,
                        port port = tcp_protocol::_server_port) {
        auto socket = connect.lock();
        sockaddr sockaddr;
        tcp_protocol::sockaddr_in_init(sockaddr, addr, port);
        int ret = ::connect(socket->file(), reinterpret_cast<const ::sockaddr*>(&sockaddr),
                            static_cast<::socklen_t>(sizeof(sockaddr)));
        if (ret < 0) {
            BOOST_LOG_TRIVIAL(error) << std::format("对于服务器\"{}:{}\"的连接出现了错误: {}", addr,
                                                    port, strerror(errno));
            return false;
        }
        BOOST_LOG_TRIVIAL(info) << std::format("服务器\"{}:{}\"已经获取了我方连接", addr, port);
        return true;
    }

   private:
    bool _is_running;

   public:
    inline static addr _server_addr = "47.107.254.122";
    inline static port _server_port = 8080;
};
