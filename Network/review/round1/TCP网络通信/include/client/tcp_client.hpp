#include "tcp_protocol.hpp"
#include <iostream> // cout, cin

class tcp_client : public tcp_protocol {
    public:
    tcp_client(const addr_t& server_addr = tcp_protocol::_server_addr,
               port_t server_port = tcp_protocol::_server_port) {

        int reconnection= 10;
        while(reconnection--)
        {
            bool ret = tcp_protocol::connect(get_proto_socket(), server_addr, server_port);
            if(ret == true) return;
            BOOST_LOG_TRIVIAL(info) << std::format("正在尝试重新连接至服务器");
            ::sleep(1);
        }
        BOOST_LOG_TRIVIAL(error) << std::format("重连的次数过多, 请等待网络好转后重试");
        ::exit(errno);
    }

    ~tcp_client() override = default;

    void run() override {
        const socket_t socket = get_proto_socket();
        std::string message;
        char buffer[4096] = {0};
        start_running();
        while(true)
        {
            std::cout << "请输入# ";
            std::getline(std::cin, message);
            ::write(socket, message.c_str(), message.size());

            ssize_t len = ::read(socket, buffer, sizeof(buffer) - 1);
            if(len > 0)
            {
                buffer[len] = '\0';
                std::cout<<buffer<<std::endl;
            }            
        }
        stop_running();
    }
};