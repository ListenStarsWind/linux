#pragma once

#include"log.hpp"
#include<cstring>
#include"NonCopy.hpp"
#include<sys/epoll.h>

static const int epoll_size = 80;
static const int default_timeout = 3000;

class Epoller : public NonCopy
{
    public:
    Epoller() {
        _epoll_fd = ::epoll_create(epoll_size);
        if(_epoll_fd == -1)
        {
            _log(Fatal, "epoll实例化失败:%s", strerror(errno));
            exit(0);
        }
    }

    ~Epoller() {
        ::close(_epoll_fd);
    }

    int epoll_wait_(struct epoll_event events[], int maxevents, int timeout = default_timeout)
    {
        return ::epoll_wait(_epoll_fd, events, maxevents, timeout);
    }

    void epoll_add_(int fd, uint32_t event)
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;

        int n = ::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev);

        if(n < 0)
            _log(Error, "epoll_ctl(ADD) 出错:%s", strerror(errno));
    }

    void epoll_mod_(int fd, uint32_t event)
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;

        int n =  ::epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev);

        if(n < 0)
            _log(Error, "epoll_ctl(MOD) 出错:%s", strerror(errno));
    }

    void epoll_del_(int fd)
    {
        int n =  ::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);

        if(n < 0)
            _log(Error, "epoll_ctl(DEL) 出错:%s", strerror(errno));
    }

    private:
    int _epoll_fd;
    wind::Log &_log = wind::Log::getInstance();
};