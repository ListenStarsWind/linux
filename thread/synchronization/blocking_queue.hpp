#pragma once

#include <pthread.h>
#include <queue>
#include <utility>
#include <cstdio>
#include <iostream>

template <class T>
class blocking_queue
{
private:
    typedef std::queue<T> queue;
    typedef size_t interval;

    pthread_mutex_t _lock;
    pthread_cond_t _not_full;
    pthread_cond_t _not_empty;
    size_t _max_size;

    queue _q;

public:
    blocking_queue(size_t max_size = 12)
    {
        pthread_mutex_init(&_lock, nullptr);
        pthread_cond_init(&_not_full, nullptr);
        pthread_cond_init(&_not_empty, nullptr);
        _max_size = max_size;
    }

    ~blocking_queue()
    {
        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_not_full);
        pthread_cond_destroy(&_not_empty);
    }

    void push(const T& val)
    {
        pthread_mutex_lock(&_lock);
        if (_q.size() == _max_size)
        {
            pthread_cond_wait(&_not_full, &_lock);
        }
        _q.push(val);
        val.producer_info();
        pthread_cond_signal(&_not_empty);
        pthread_mutex_unlock(&_lock);
    }

    template <class... Args>
    void emplace(Args &&...args)
    {
        pthread_mutex_lock(&_lock);
        while (_q.size() == _max_size)
        {
            pthread_cond_wait(&_not_full, &_lock);
        }
        _q.emplace(std::forward<Args>(args)...);
        printf("[producer]<%lu>info: Generate a task: %d %c %d = ?\n", pthread_self(), args...);
        pthread_cond_signal(&_not_empty);
        pthread_mutex_unlock(&_lock);
    }

    T front()
    {
        pthread_mutex_lock(&_lock);
        while (_q.size() == 0)
        {
            pthread_cond_wait(&_not_empty, &_lock);
        }
        T result = _q.front();
        _q.pop();
        pthread_cond_signal(&_not_full);
        pthread_mutex_unlock(&_lock);
        return result;
    }
};