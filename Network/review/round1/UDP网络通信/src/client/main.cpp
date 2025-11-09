#include "boost_log.hpp"
#include "client/udp_client.hpp"

int main(int argc, char* argv[]) {
    init_logging();
    in_port_t port = 8080;
    std::string ip = "47.107.254.122";
    if (argc != 3) {
        BOOST_LOG_TRIVIAL(warning) << std::format("没有指定服务端口号, 将使用缺省值: {}", port);
        BOOST_LOG_TRIVIAL(warning) << std::format("没有指定服务端ip, 将使用缺省值: {}", ip);
    } else {
        port = std::stoi(*(argv + 1));
        ip = *(argv + 2);
    }

    udp_client client(port, ip);
    client.run();

    return 0;
}