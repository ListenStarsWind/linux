#include"TcpServer.hpp"
#include"Calculator.hpp"

static const uint16_t default_port = 8080;

calculator cal;

void testOnMessage(std::weak_ptr<Connection> connect)
{
    auto act = connect.lock();

    std::cout<<"我是应用协议层, 我读到了数据: "<<std::endl;
    printf("来自文件描述符[%d], 地址<%s:%d>\n", act->sock_fd(), act->_ip.c_str(), act->_port);
    std::cout<<act->_inbuffer.data();

    // 以前写的自定义协议使用字符串做缓冲区
    std::string buff(act->_inbuffer.begin(), act->_inbuffer.end());

    
}

int main()
{
    std::shared_ptr<TcpServer> tcp_server_ptr = TcpServer::create(default_port, testOnMessage);
    tcp_server_ptr->init();
    tcp_server_ptr->loop();
    return 0;
}