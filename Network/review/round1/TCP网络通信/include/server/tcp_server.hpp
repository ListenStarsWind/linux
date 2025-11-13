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

   public:
    tcp_server(const addr_t& listen_addr = "0.0.0.0", port_t listen_port = 8080)
        : _listen_addr(listen_addr),
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
            func4(user_socket, user_ip);
        }
        stop_running();
    }

   private:
    // 第四版: 使用线程池
    void func4(socket_t socket, const addr_t& user_ip) {
        // 注意 lambda 要把参数复制拷贝进去, 引用在本次函数栈帧结束后会悬空
        task_t t = [socket = socket, user_ip = user_ip]() { tcp_server::func2(socket, user_ip); };
        _task_pool->push(t);
    }

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
                // tcp_server::close_client();
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
    addr_t _listen_addr;
    port_t _listen_port;
    tasks_ptr_t _task_pool;
};