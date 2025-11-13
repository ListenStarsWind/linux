#include "boost_log.hpp"
#include "client/tcp_client.hpp"

int main(int argc, char* argv[]) {
    init_logging();
    std::string server_addr = tcp_protocol::_server_addr;
    in_port_t server_port = tcp_protocol::_server_port;
    if (argc != 3) {
        BOOST_LOG_TRIVIAL(warning) << std::format("未显示指定服务端地址, 将使用缺省值");
    } else {
        server_addr = *(argv + 1);
        server_port = std::stoi(*(argv + 2));
    }
    BOOST_LOG_TRIVIAL(info) << std::format("服务端地址确认: \"{}:{}\"", server_addr, server_port);
    tcp_client client(server_addr, server_port);
    while (true) {
        client.run();
    }
    return 0;
}
