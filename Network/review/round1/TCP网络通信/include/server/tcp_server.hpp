#pragma once

#include "tcp_protocol.hpp"

class tcp_server : public tcp_protocol {
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
        while(true)
        {
            tcp_protocol::bzero(user_sockaddr);
            auto user_socket = tcp_server::accept(listen_socket, user_sockaddr);
            if(user_socket < 0) continue;

            auto user_ip = tcp_protocol::inet_ntop(user_sockaddr);
            tcp_server::func(user_socket, user_ip);
        }
        stop_running();
    }

   private:
    static socket_t accept(socket_t socket, sockaddr_t& user_sockaddr) {
        socklen_t len = static_cast<socklen_t>(sizeof(user_sockaddr));
        auto user_socket = ::accept(socket, reinterpret_cast<::sockaddr*>(&user_sockaddr), &len);
        if (user_socket < 0) {
            BOOST_LOG_TRIVIAL(warning) << std::format("接触到一个异常的用户连接");
        } else {
            BOOST_LOG_TRIVIAL(info) << std::format("接收到一个描述符为{}的用户连接", user_socket);
        }
        return user_socket;
    }

    static void func(socket_t socket, const addr_t& user_ip)
    {
        (void)user_ip;
        char buffer[1024] = {0};
        while(true)
        {
            ssize_t len = ::read(socket, buffer, sizeof(buffer) - 1);
            if(len > 0)
            {
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