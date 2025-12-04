#pragma once

#include <sys/epoll.h>

#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <system_error>
#include <vector>

#include "Connection.hpp"
#include "EpollerItem.hpp"
#include "NonCopy.hpp"

class Epoller : public NonCopy {
   public:
    using EpollerItem = EpollerItem<Connection>;
    using vector = std::vector<EpollerItem>;

   public:
    Epoller(int size = 64) : _size(size) {
        // EPOLL_CLOEXEC 进程替换时自动关闭 epoll 模型, 避免出现继承冲突
        // epoll 模型不是一般文件, 关键在于它的就绪队列和红黑树
        // 这两个玩意开销还是很大的, 如果替换进程不用 epoll, 那就资源泄露了
        _epfd = ::epoll_create1(EPOLL_CLOEXEC);
        if (_epfd < 0) {
            throw std::system_error(errno, std::system_category(), "epoll 模型创建失败");
        }
    }

    ~Epoller() {
        ::close(_epfd);
    }

    vector wait(int timeout) {
        vector items(_size);
        int n = ::epoll_wait(_epfd, reinterpret_cast<epoll_event*>(items.data()), items.size(),
                             timeout);
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "epoll等待出错");
        }
        items.resize(n);
        return items;
    }

    void add(Connection::self_weak_ptr connect, uint32_t event) {
        auto con = connect.lock();
        EpollerItem item(event, con.get());
        int n = ::epoll_ctl(_epfd, EPOLL_CTL_ADD, item.connect()->file(),
                            reinterpret_cast<epoll_event*>(&item));
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "epoll添加事件出错");
        }
        con->need_write_interest() = event & EpollerItem::outwritable;
    }

    void mod(Connection::self_weak_ptr connect, uint32_t event) {
        auto con = connect.lock();
        EpollerItem item(event, con.get());
        int n = ::epoll_ctl(_epfd, EPOLL_CTL_MOD, item.connect()->file(),
                            reinterpret_cast<epoll_event*>(&item));
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "epoll事件修改出错");
        }
        con->need_write_interest() = event & EpollerItem::outwritable;
    }

    void del(Connection::self_weak_ptr connect) {
        auto con = connect.lock();
        int n = ::epoll_ctl(_epfd, EPOLL_CTL_DEL, con->file(), nullptr);
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "epoll事件删除出错");
        }
    }

   private:
    int _size;
    int _epfd;
};
