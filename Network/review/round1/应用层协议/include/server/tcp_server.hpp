#pragma once

#include <signal.h>  // kill

#include <thread>

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
               port_t listen_port = 8080)
        : _call_back(call_back),
          _listen_addr(listen_addr),
          _listen_port(listen_port),
          _task_pool(std::make_unique<tasks_t>(5)) {
        reset_proto_socket();
        sockaddr_t listen_sockaddr;
        tcp_protocol::sockaddr_in_init(listen_sockaddr, _listen_addr, _listen_port);
        tcp_protocol::bind_port(get_proto_socket(), listen_sockaddr);
        tcp_protocol::listen(get_proto_socket());
        ::signal(SIGPIPE, SIG_IGN);
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
            func(user_socket, user_ip);
        }
        stop_running();
    }

   private:
    void func(socket_t socket, const addr_t& user_ip) {
        task_t t = [this, socket, user_ip]() {
            (void)user_ip;
            char buffer[1024] = {0};
            std::string messages;
            while (true) {
                ssize_t len = ::read(socket, buffer, sizeof(buffer) - 1);
                if (len > 0) {
                    buffer[len] = '\0';
                    BOOST_LOG_TRIVIAL(info) << std::format("用户发出了这样的消息: {}", buffer);
                    messages += buffer;
                    try {
                        std::string echo = _call_back(messages);
                        write(socket, echo.c_str(), echo.size());
                    } catch (...) {
                        BOOST_LOG_TRIVIAL(error) << std::format("应用层出现了无法处理的错误, 已将该连接视为非法");
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

    static void close_client() {
        FILE* fp = popen("pidof -s demo_client", "r");
        char buffer[32] = {0};
        if (::fgets(buffer, sizeof(buffer), fp) == nullptr) {
            BOOST_LOG_TRIVIAL(error) << std::format(
                "这个机器上没有demo_client, 是误使用了close_client接口吗? "
                "注意该接口仅仅是用来测试服务端对已经关闭的套接字进行写操作而可能触发的信号效果");
            ::exit(0);
        }

        // 抹去末尾的换行符
        buffer[::strlen(buffer) - 1] = '\0';

        pid_t id = atoi(buffer);
        ::kill(id, SIGUSR1);

        ::pclose(fp);
    }

   private:
    call_back_t _call_back;
    addr_t _listen_addr;
    port_t _listen_port;
    tasks_ptr_t _task_pool;
};