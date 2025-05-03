#pragma once

#include"log.hpp"
#include"TcpSocket.hpp"

static const int default_port = 8080;

class EpollServer
{
    public:
    EpollServer(uint16_t port = default_port) : _port(port){}
    ~EpollServer(){ _listenSock.socket_close_();}

    void init()
    {
        _listenSock.socket_create_();
        _listenSock.socket_bind_(_port);
        _listenSock.socket_reuse_port_address();
        _listenSock.socket_listen_();
    }

    void statrt()
    {
        
    }

    private:
    TcpSocket _listenSock;
    uint16_t _port;
    Log &_log = Log::getInstance();
};