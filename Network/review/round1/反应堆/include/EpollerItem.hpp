#pragma once

#include <sys/epoll.h>

#include <cstddef>
#include <cstring>
#include <string>

template <typename Connect>
class EpollerItem final {
   public:
    EpollerItem() {
        ::memset(&item, 0, sizeof(item));
    }

    EpollerItem(uint32_t events, Connect* connect) {
        item.events = events;
        item.data.ptr = connect;
    }

    uint32_t& events() {
        return item.events;
    }

    Connect* connect() {
         return static_cast<Connect*>(item.data.ptr);
    }

    static std::string event_name(uint32_t events) {
        std::string res;
        if (events & inreadable) res += "可读事件|";
        if (events & outwritable) res += "可写事件|";
        if (events & urgent) res += "紧急数据可读|";
        if (events & error) res += "文件错误|";
        if (events & hangup) res += "对端关闭|";
        if (events & edge_trigger) res += "事件驱动|";
        if (events & one_shot) res += "仅监听一次|";
        if (!res.empty())
            res.pop_back();  // 去掉最后的 '|'
        else
            res = "未知事件";
        return res;
    }

    static constexpr uint32_t inreadable = EPOLLIN;
    static constexpr uint32_t outwritable = EPOLLOUT;
    static constexpr uint32_t urgent = EPOLLPRI;
    static constexpr uint32_t error = EPOLLERR;
    static constexpr uint32_t hangup = EPOLLHUP;
    static constexpr uint32_t edge_trigger = EPOLLET;
    static constexpr uint32_t one_shot = EPOLLONESHOT;

   private:
    ::epoll_event item;
};

// 外部 layout 检查
inline void check_epolleritem_layout() {
    static_assert(std::is_standard_layout_v<EpollerItem<int>>, "EpollerItem 必须是标准布局");
    static_assert(sizeof(EpollerItem<int>) == sizeof(::epoll_event), "大小必须一致");
    static_assert(alignof(EpollerItem<int>) == alignof(::epoll_event), "对齐必须一致");
}
