#pragma once

#include <memory>
#include <thread>
#include <vector>

#include "ring_queue.hpp"

template <typename Func>
class thread_pool {
    using func_t = Func;
    using thread = std::thread;
    using threads = std::vector<thread>;
    using threads_ptr_t = std::unique_ptr<threads>;
    using funcs_t = ring_queue<func_t>;
    using funcs_ptr_t = std::unique_ptr<funcs_t>;
    using self_t = thread_pool<Func>;

   public:
    thread_pool(size_t size)
        : _is_running(true),
          _funcs(std::make_unique<funcs_t>(size)),
          _threads(std::make_unique<threads>(size)) {
        for (auto& t : *_threads) {
            t = thread(handler, this);
        }
    };

    template <typename... Args>
    void push(Args&&... args) {
        _funcs->push(std::forward<Args>(args)...);
    }

   private:
    static void handler(self_t* me) {
        while (me->_is_running) {
            auto task = me->_funcs->pop();
            (*task)();
        }
    }

   private:
    bool _is_running;
    funcs_ptr_t _funcs;
    threads_ptr_t _threads;
};