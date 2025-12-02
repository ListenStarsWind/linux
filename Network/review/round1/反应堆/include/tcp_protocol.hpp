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
    using addr_t = std::string;
    using socket_t = Connection;
    using port_t = ::in_port_t;
    using socket_ptr_t = std::unique_ptr<socket_t, void (*)(socket_t*)>;
    using sockaddr_t = ::sockaddr_in;

   public:
    tcp_protocol()
        : _is_running(false), _proto_socket(new socket_t, &tcp_protocol::socket_destroy) {}

    virtual ~tcp_protocol() = default;

    virtual void run() {
        start_running();
        stop_running();
    }

    void clean() {
        _proto_socket.reset();
    }

   private:
    static void socket_destroy(socket_t* p) {
        if (p != nullptr) {
            if (p->file() != 0) {
                close(p->file());
            }
            delete p;
        }
    }

   protected:
    // 这两个接口是给客户端控制短连接时机用的
    void close_proto_socket() {
        close(_proto_socket->file());
        _proto_socket->set_file(0);
    }

    void reset_proto_socket() {
        _proto_socket->set_file(tcp_protocol::socket_init());
    }

    // 以只读形式暴露原初套接字描述符
    socket_t& get_proto_socket() {
        return *_proto_socket;
    }

    bool is_running() {
        return _is_running;
    }

    void start_running() {
        BOOST_LOG_TRIVIAL(info) << std::format("程序正在运行, 已经进入网络IO主逻辑");
        _is_running = true;
    }

    void stop_running() {
        BOOST_LOG_TRIVIAL(info) << std::format("程序退出IO主逻辑");
        _is_running = false;
    }

    static void listen(const socket_t& socket, int backlog = 5) {
        int ret = ::listen(socket.file(), backlog);
        if (ret < 0) {
            throw std::system_error(errno, std::generic_category(), std::format("套接字描述符({})设置监听状态失败: ", socket.file()));
        }
        BOOST_LOG_TRIVIAL(info) << std::format("描述符为{}的套接字设置监听状态成功", socket.file());
    }

    static int accept(const socket_t& socket, sockaddr_t& user_sockaddr) {
        socklen_t len = static_cast<socklen_t>(sizeof(user_sockaddr));
        auto user_socket = ::accept(socket.file(), reinterpret_cast<::sockaddr*>(&user_sockaddr), &len);
        if (user_socket < 0) {
            BOOST_LOG_TRIVIAL(warning) << std::format("接触到一个异常的用户连接");
        } else {
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

    static addr_t inet_ntop(const sockaddr_t& sockaddr) {
        char buffer[32];
        ::inet_ntop(AF_INET, &sockaddr.sin_addr, buffer, sizeof(buffer));
        return buffer;
    }

    static void bzero(sockaddr_t& sockaddr) {
        ::bzero(&sockaddr, sizeof(sockaddr));
    }

    static void sockaddr_in_init(sockaddr_t& sockaddr, const addr_t& addr, port_t port) {
        tcp_protocol::bzero(sockaddr);
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = ::htons(port);
        ::inet_aton(addr.c_str(), &sockaddr.sin_addr);
        BOOST_LOG_TRIVIAL(info) << std::format("拼接了这样的一个套接字: \"{}:{}\"", addr,
                                               std::to_string(port));
    }

    static void bind_port(const socket_t& socket, const sockaddr_t& sockaddr) {
        int ret = ::bind(socket.file(), reinterpret_cast<const ::sockaddr*>(&sockaddr),
                         static_cast<::socklen_t>(sizeof(sockaddr)));
        if (ret != 0) {
            throw std::system_error(errno, std::generic_category(), std::format("套接字({})绑定端口错误: ", socket.file()));
        }

        BOOST_LOG_TRIVIAL(info) << std::format("对于{}套接字描述符已经绑定成功", socket.file());
    }

    static bool connect(const socket_t& socket, const addr_t& addr = tcp_protocol::_server_addr,
                        port_t port = tcp_protocol::_server_port) {
        sockaddr_t sockaddr;
        tcp_protocol::sockaddr_in_init(sockaddr, addr, port);
        int ret = ::connect(socket.file(), reinterpret_cast<const ::sockaddr*>(&sockaddr),
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
    socket_ptr_t _proto_socket;

   public:
    inline static addr_t _server_addr = "47.107.254.122";
    inline static port_t _server_port = 8080;
};