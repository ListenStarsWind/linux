#pragma once

#include<pthread.h>

class thread
{
    typedef void(*call_back_t)();

    static void* behavior(void* p_)
    {
        thread* td = static_cast<thread*>(p_);
        td->_cb();
        return nullptr;
    }

    public:
    thread(call_back_t cb)
        :_tid(pthread_t()), _is_running(false), _cb(cb){}

    void run()
    {
        pthread_create(&_tid, nullptr, behavior, this);
        _is_running = true;
    }

    void join()
    {
        pthread_join(_tid, nullptr);
        _is_running = false;
    }

    bool isrun()
    {
        return _is_running;
    }


    private:
    pthread_t _tid;
    bool _is_running;
    call_back_t _cb;
};