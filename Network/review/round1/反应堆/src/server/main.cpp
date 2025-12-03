#include "boost_log.hpp"
#include "server/SessionHandler.hpp"
#include "server/tcp_server.hpp"

int main(int argc, char* argv[])
{
    init_logging();
    std::string addr = "0.0.0.0";
    in_port_t port = tcp_protocol::_server_port;
    if (argc != 2) {
        BOOST_LOG_TRIVIAL(info) << std::format("未显式指明服务器端口, 将使用缺省值");
    } else {
        port = std::stoi(*(argv + 1));
    }
    BOOST_LOG_TRIVIAL(info) << std::format("服务器地址确认: \"{}:{}\"", addr, port);

    SessionHandler call_back;
    auto server = tcp_server::create(call_back, addr, port);
    server->run();

    return 0;
}
