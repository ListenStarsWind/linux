#include <signal.h>  // kill

#include <iostream>  // cout, cin

#include "tcp_protocol.hpp"

class tcp_client : public tcp_protocol {
    using isstream = std::iostream;

   public:
    tcp_client(const addr_t& server_addr = tcp_protocol::_server_addr,
               port_t server_port = tcp_protocol::_server_port)
        : _server_addr(server_addr), _server_port(server_port) {}

    ~tcp_client() override = default;

    // 在客户端, 它是短连接的
    void run(istream& in = std::cin, ostream& out = std::cout) override {
        (void)out;
        reset_proto_socket();
        connect();
        const socket_t socket = get_proto_socket();
        std::string message{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
        char buffer[4096] = {0};
        start_running();

        ::write(socket, message.c_str(), message.size());

        ssize_t len = ::read(socket, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            std::cout << buffer << std::endl;
        }
        else {
            BOOST_LOG_TRIVIAL(error) << std::format("与服务器通信不畅: {}", strerror(errno));
        }

        stop_running();
        close_proto_socket();
    }

   private:
    void connect() {
        int reconnection = 10;
        while (reconnection--) {
            bool ret = tcp_protocol::connect(get_proto_socket(), _server_addr, _server_port);
            if (ret == true) return;
            BOOST_LOG_TRIVIAL(info) << std::format("正在尝试重新连接至服务器");
            ::sleep(1);
        }
        BOOST_LOG_TRIVIAL(error) << std::format("重连的次数过多, 请等待网络好转后重试");
        ::exit(errno);
    }

    static void handler(int event) {
        if (event == SIGUSR1) {
            socket_t fd = tcp_client::proto_fd;
            if (fd != 0) {
                ::close(fd);
                ::sleep(10);
            }
        }
    }

   private:
    addr_t _server_addr;
    port_t _server_port;

    inline static socket_t proto_fd = 0;
};