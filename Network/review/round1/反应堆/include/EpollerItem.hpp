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

        struct Item {
            uint32_t bit;
            const char* name;
        };
        static constexpr Item table[] = {
            {error, "文件错误"},        {hangup, "挂断"},         {rdhangup, "对端关闭"},
            {urgent, "紧急数据可读"},   {inreadable, "可读事件"}, {outwritable, "可写事件"},
            {edge_trigger, "边沿触发"}, {one_shot, "仅监听一次"},
        };

        for (auto& t : table)
            if (events & t.bit) res += t.name, res += '|';

        if (!res.empty())
            res.pop_back();
        else
            res = "未知事件";

        return res;
    }

    // | 事件               | 是否自动返回       | 说明                |
    // | ---------------- | ------------ | ----------------- |
    // | **EPOLLHUP**     | ✔ 无论是否关心都会返回 | 强制关心（不可屏蔽）        |
    // | **EPOLLERR**     | ✔ 无论是否关心都会返回 | 强制关心（不可屏蔽）        |
    // | **EPOLLRDHUP**   | 需要手动关心       | 不关注就不会收到可读事件      |
    // | **EPOLLIN**      | 需要手动关心       | 不关注就不会收到可读事件      |
    // | **EPOLLOUT**     | 需要手动关心       | 不关注就不会收到可写事件      |
    // | **EPOLLPRI**     | 需要手动关心       | 带外数据（几乎不用）        |
    // | **EPOLLET**      | 配置修饰符，不是事件   |                   |
    // | **EPOLLONESHOT** | 配置修饰符，不是事件   |                   |

    static constexpr uint32_t inreadable = EPOLLIN;
    static constexpr uint32_t outwritable = EPOLLOUT;
    static constexpr uint32_t urgent = EPOLLPRI;
    static constexpr uint32_t error = EPOLLERR;
    static constexpr uint32_t hangup = EPOLLHUP;
    static constexpr uint32_t rdhangup = EPOLLRDHUP;
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
