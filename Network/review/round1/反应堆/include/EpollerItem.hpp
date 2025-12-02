#pragma once

#include <sys/epoll.h>

#include <string>

template <typename Connect>
class EpollerItem {
    using self_t = EpollerItem<Connect>;

   public:
    EpollerItem() {
        static_assert(sizeof(self_t) == sizeof(epoll_event), "ABI mismatch");
    }
    EpollerItem(uint32_t events, const Connect* connect) : _events(events), _connect(connect) {}

    uint32_t events() {
        return _events;
    }

    Connect* connect() {
        return _connect;
    }

    static std::string event_name(uint32_t events) {
        std::string res;
        if (events & self_t::inreadable) res += "可读事件|";
        if (events & self_t::outwritable) res += "可写事件|";
        if (events & self_t::urgent) res += "紧急数据可读|";
        if (events & self_t::error) res += "文件错误|";
        if (events & self_t::hangup) res += "对端关闭|";
        if (events & self_t::edge_trigger) res += "事件驱动|";
        if (events & self_t::one_shot) res += "仅监听一次|";

        if (!res.empty())
            res.pop_back();  // 去掉最后的 '|'
        else
            res = "未知事件";
        return res;
    }

   private:
    uint32_t _events;    // epoll events
    uint32_t _reserved;  // ABI 保留字段，用于与 epoll_event 对齐
    Connect* _connect;   // 用户指针，与 epoll_event.data.ptr 对齐

   public:
    inline static const uint32_t inreadable = EPOLLIN;
    inline static const uint32_t outwritable = EPOLLOUT;
    inline static const uint32_t urgent = EPOLLPRI;
    inline static const uint32_t error = EPOLLERR;
    inline static const uint32_t hangup = EPOLLHUP;
    inline static const uint32_t edge_trigger = EPOLLET;
    inline static const uint32_t one_shot = EPOLLONESHOT;
};
