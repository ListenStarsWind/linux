#pragma once

#include<memory>
#include"log.hpp"
#include"TcpSocket.hpp"
#include"Epoller.hpp"
#include"NonCopy.hpp"
#include<cstring>
#include<string>
#include<iostream>

static const int default_port = 8080;
static const int default_max_events = 64;
static const uint32_t EPOLL_IN = EPOLLIN;
static const uint32_t EPOLL_OUT = EPOLLOUT;

class EpollServer : public NonCopy
{
    public:
    EpollServer(uint16_t epoll_server_port = default_port) : _epoll_server_port(epoll_server_port){
        _listensock_ptr = std::make_shared<TcpSocket>();
        _epoll_ptr = std::make_shared<Epoller>();
    }

    ~EpollServer(){ _listensock_ptr->socket_close_();}

    void epoll_server_init_()
    {
        _listensock_ptr->socket_create_();
        _listensock_ptr->socket_bind_(_epoll_server_port);
        _listensock_ptr->socket_reuse_port_address_();
        _listensock_ptr->socket_listen_();
    }

    void epoll_server_statrt()
    {
        _epoll_ptr->epoll_add_(_listensock_ptr->socket_fd_(), EPOLL_IN);
        for(;;)
        {
            int n = _epoll_ptr->epoll_wait_(_events, default_max_events);
            if(n > 0)   epoll_server_dispatch_event(n);
            else if(n == 0) _log(Info, "epoll 超时...");
            else _log(Warning, "epoll_wait 发生了错误:%s", strerror(errno));
        }
    }

    private:
    void epoll_server_dispatch_event(int events)
    {
        for(int i = 0; i < events; ++i)
        {
            int fd = _events[i].data.fd;
            uint32_t event = _events[i].events;
            if(event & EPOLL_IN)
            {
                if(fd == _listensock_ptr->socket_fd_()) accept_();
                else response_(fd);
            }
            else if(event & EPOLL_OUT)
            {

            }
            else
            {

            }
        }
    }

    void accept_()
    {
        std::string clientip;
        uint16_t clientport;
        int sock = _listensock_ptr->socket_accept_(&clientip, &clientport);
        if(sock < 0) _log(Warning, "不完整的三次握手:%s", strerror(errno));
        else
        {
            // 多路转接服务器只会读写确认就绪的文件
            _log(Info, "获取一个客户端连接, ip:%s, port:%d", clientip.c_str(), clientport);
            _epoll_ptr->epoll_add_(sock, EPOLL_IN);
        }
    }

    void response_(int fd)
    {
        _log(Info, "📩 收到一个新的消息，来自 fd = %d", fd);

        char buff[128] = {0};
        int n = read(fd, buff, sizeof(buff) - 1);

        if (n > 0)
        {
            buff[n] = '\0';
            std::cout << "🗣️  用户(fd=" << fd << ") 说：\n";
            std::cout << "    \"" << buff << "\"" << std::endl;
        }
        else if (n == 0)
        {
            _log(Warning, "👋 用户(fd = %d) 断开了连接", fd);
            // 关文件之前在epoll那里注销关心事件
            _epoll_ptr->epoll_del_(fd);
            close(fd);
        }
        else
        {
            _log(Warning, "❌ 读取数据时发生错误(fd = %d)", fd);
            _epoll_ptr->epoll_del_(fd);
            close(fd);
        }
    }


    private:
    std::shared_ptr<TcpSocket> _listensock_ptr;
    std::shared_ptr<Epoller> _epoll_ptr;
    struct epoll_event _events[default_max_events];
    uint16_t _epoll_server_port;
    wind::Log &_log = wind::Log::getInstance();
};