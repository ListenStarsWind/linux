#pragma once

#include "tcp_protocol.hpp"
#include <thread>

class tcp_server : public tcp_protocol {
    using thread = std::thread;

   public:
    tcp_server(const addr_t& listen_addr = "0.0.0.0", port_t listen_port = 8080)
        : _listen_addr(listen_addr), _listen_port(listen_port) {
        sockaddr_t listen_sockaddr;
        tcp_protocol::sockaddr_in_init(listen_sockaddr, _listen_addr, _listen_port);
        tcp_protocol::bind_port(get_proto_socket(), listen_sockaddr);
        tcp_protocol::listen(get_proto_socket());
    }

    ~tcp_server() override = default;

    void run() override {
        sockaddr_t user_sockaddr;
        const socket_t listen_socket = get_proto_socket();
        start_running();
        while (true) {
            tcp_protocol::bzero(user_sockaddr);
            auto user_socket = tcp_protocol::accept(listen_socket, user_sockaddr);
            if (user_socket < 0) continue;

            auto user_ip = tcp_protocol::inet_ntop(user_sockaddr);
            tcp_server::func3(user_socket, user_ip);
        }
        stop_running();
    }

   private:
    // 第三版: 新引入了多线程
    static void func3(socket_t socket, const addr_t& user_ip) {
        thread handler(func2, socket, user_ip);
        handler.detach();
    }

    // 第二版: 仅新加入了查错处理
    static void func2(socket_t socket, const addr_t& user_ip) {
        (void)user_ip;
        char buffer[1024] = {0};
        while (true) {
            ssize_t len = ::read(socket, buffer, sizeof(buffer) - 1);
            // 大于零时正常通信
            if (len > 0) {
                buffer[len] = '\0';
                BOOST_LOG_TRIVIAL(info) << std::format("用户发出了这样的消息: {}", buffer);
                std::string echo = std::format("服务器受到了你的消息, \"{}\"", buffer);
                write(socket, echo.c_str(), echo.size());
                continue;
            }
            // 等于零是连接关闭
            else if (len == 0) {
                BOOST_LOG_TRIVIAL(info) << std::format("用户关闭了连接");
            }
            // IO 错误
            else {
                BOOST_LOG_TRIVIAL(warning) << std::format("读端出现了错误: {}", strerror(errno));
            }
            ::close(socket);
            return;
        }
    }

    // 第一版: 最基础的, 甚至没有差错处理
    static void func1(socket_t socket, const addr_t& user_ip) {
        (void)user_ip;
        char buffer[1024] = {0};
        while (true) {
            ssize_t len = ::read(socket, buffer, sizeof(buffer) - 1);
            if (len > 0) {
                buffer[len] = '\0';
                BOOST_LOG_TRIVIAL(info) << std::format("用户发出了这样的消息: {}", buffer);
                std::string echo = std::format("服务器受到了你的消息, \"{}\"", buffer);
                write(socket, echo.c_str(), echo.size());
            }
        }
    }

   private:
    addr_t _listen_addr;
    port_t _listen_port;
};