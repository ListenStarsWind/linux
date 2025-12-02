#pragma once

#include <signal.h>  // kill

#include <thread>

#include "Epoller.hpp"
#include "tcp_protocol.hpp"
#include "thread_pool.hpp"

class tcp_server : public tcp_protocol {
    using thread = std::thread;
    using task_t = std::function<void(void)>;
    using tasks_t = thread_pool<task_t>;
    using tasks_ptr_t = std::unique_ptr<tasks_t>;
    using call_back_t = std::function<std::string(std::string&)>;

   public:
    tcp_server(const call_back_t& call_back, const addr_t& listen_addr = "0.0.0.0",
               port_t listen_port = 8080, int epoll_size = 64)
        : _call_back(call_back),
          _listen_addr(listen_addr),
          _listen_port(listen_port),
          _task_pool(std::make_unique<tasks_t>(5)),
          _epoll(epoll_size) {
        try {
            reset_proto_socket();
            sockaddr_t listen_sockaddr;
            tcp_protocol::sockaddr_in_init(listen_sockaddr, _listen_addr, _listen_port);
            auto& listen_socket = get_proto_socket();
            tcp_protocol::bind_port(listen_socket, listen_sockaddr);
            tcp_protocol::listen(listen_socket);
            _epoll.add(listen_socket, Epoller::EpollerItem::inreadable);
            listen_socket.set_in_call([](){});
            ::signal(SIGPIPE, SIG_IGN);
        } catch (std::system_error& e) {
            BOOST_LOG_TRIVIAL(fatal) << std::format("服务初始化发生致命错误: {}", e.what());
            ::exit(e.code().value());
        }
    }

    ~tcp_server() override = default;

    void run() override {
        sockaddr_t user_sockaddr;
        const auto& listen_socket = get_proto_socket();
        start_running();
        while (true) {
            tcp_protocol::bzero(user_sockaddr);
            auto user_socket = tcp_protocol::accept(listen_socket, user_sockaddr);
            if (user_socket < 0) continue;

            auto user_ip = tcp_protocol::inet_ntop(user_sockaddr);
            func(user_socket, user_ip);
        }
        stop_running();
    }

   private:
    void func(int socket, const addr_t& user_ip) {
        task_t t = [this, socket, user_ip]() {
            (void)user_ip;
            char buffer[1024] = {0};
            std::string messages;
            std::string temp;
            while (true) {
                ssize_t len = ::read(socket, buffer, sizeof(buffer));
                if (len > 0) {
                    BOOST_LOG_TRIVIAL(info) << std::format("用户发出了这样的消息: {}", buffer);
                    temp.clear();
                    temp.append(buffer, len);
                    messages += temp;
                    try {
                        std::string echoes;
                        std::string echo;
                        do {
                            echo.clear();
                            echo = _call_back(messages);
                            echoes += echo;
                        } while (
                            !echo.empty() &&
                            !messages
                                 .empty());  // echo 空意味着剩下的messages不足以构成一个完整的报文,
                                             // 所以继续尝试读; messages空那就解析不了
                        ::write(socket, echoes.c_str(), echoes.size());
                    } catch (...) {
                        BOOST_LOG_TRIVIAL(error)
                            << std::format("应用层出现了无法处理的错误, 已将该连接视为非法");
                        ::close(socket);
                        return;
                    }
                    continue;
                } else if (len == 0) {
                    BOOST_LOG_TRIVIAL(info) << std::format("用户关闭了连接");
                } else {
                    BOOST_LOG_TRIVIAL(warning)
                        << std::format("读端出现了错误: {}", strerror(errno));
                }
                ::close(socket);
                return;
            }
        };

        _task_pool->push(t);
    }

   private:
    call_back_t _call_back;
    addr_t _listen_addr;
    port_t _listen_port;
    tasks_ptr_t _task_pool;
    Epoller _epoll;
};