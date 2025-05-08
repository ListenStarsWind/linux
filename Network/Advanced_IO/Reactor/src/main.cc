#include"TcpServer.hpp"
#include"Calculator.hpp"

static const uint16_t default_port = 8888;

calculator cal;

void testOnMessage(std::weak_ptr<Connection> connect)
{
    auto act = connect.lock();

    std::cout<<"我是应用协议层, 我读到了数据: "<<std::endl;
    printf("来自文件描述符[%d], 地址<%s:%d>\n", act->sock_fd(), act->_ip.c_str(), act->_port);
    std::cout<<act->_inbuffer.c_str();

    std::string response_str = cal(act->_inbuffer);  // 当任务比较简单时, 可以直接这样, 但任务很繁重时, 为了快速响应, 那就要派发给其它执行流去做了.

    // 不足以构成一个完整报文那就继续积累数据
    if(response_str.empty()) return;

    std::cout<< response_str<<std::endl;

    // 将响应报文交给服务器框架
    act->_outbuffer += response_str;

    // act->_send_cb(act);
    // 上面有点粗暴, 自然来说, 应该由服务器框架和传输层间拷贝数据
    auto svr = act->_tcp_server_ptr.lock();
    svr->Sender(act);
}

int main()
{
    std::shared_ptr<TcpServer> tcp_server_ptr = TcpServer::create(default_port, testOnMessage);
    tcp_server_ptr->init();
    tcp_server_ptr->loop();
    return 0;
}