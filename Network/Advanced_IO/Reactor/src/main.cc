#include"TcpServer.hpp"

static const uint16_t default_port = 8080;

int main()
{
    std::shared_ptr<TcpServer> tcp_server_ptr = TcpServer::create(default_port);
    tcp_server_ptr->init();
    tcp_server_ptr->loop();
    return 0;
}