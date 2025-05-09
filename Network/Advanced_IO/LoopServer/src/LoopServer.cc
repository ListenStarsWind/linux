#include"LoopServer.hpp"
#include"ServerCal.hpp"

static const uint16_t default_port = 8888;

calculator cal;

void DefaultOnMessage(std::weak_ptr<Connection> conn)
{
    auto me = conn.lock();

    std::cout<<"协议层得到了数据:"<<me->_inbuff;

    std::string response_str = cal(me->_inbuff);

    if(response_str.empty()) return;

    std::cout<<response_str<<std::endl;

    me->_outbuff += response_str;

   me->_send_cb(me);
}

int main()
{
    std::shared_ptr<LoopServer> server = LoopServer::create(default_port, DefaultOnMessage);
    server->init();
    server->loop();
    return 0;
}