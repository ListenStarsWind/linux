#pragma once

#include"DataGuard.hpp"
#include<pthread.h>
#include <semaphore.h>
#include <utility>
#include<vector>

template<class T>
class RingQueue
{
    private:
    typedef DataGuard<T> Data_guard;
    typedef typename Data_guard::function function;
    typedef std::vector<Data_guard> vector;
    typedef sem_t sem;
    typedef pthread_mutex_t mutex;

    public:
    RingQueue(size_t capa, function in_log = [](const T& val){}, function out_log = [](const T& val){})
        :_capa(capa)
        ,_queue(_capa, Data_guard(in_log, out_log))
        ,_push_idx(0)
        ,_pop_idx(0)
        {
            sem_init(&_space_sum, 0, _capa);
            sem_init(&_data_sum, 0, 0);
            pthread_mutex_init(&_push_mutex, nullptr);
            pthread_mutex_init(&_pop_mutex, nullptr);
        }

    ~RingQueue()
    {
        sem_destroy(&_space_sum);
        sem_destroy(&_data_sum);
        pthread_mutex_destroy(&_push_mutex);
        pthread_mutex_destroy(&_pop_mutex);
    }

    template<class... Args>
    void push(Args&&... args)
    {
        P(&_space_sum);
        lock(&_push_mutex);
        _queue[_push_idx].push(std::forward<Args>(args)...);
        ++_push_idx;
        _push_idx %= _capa;
        unlock(&_push_mutex);
        V(&_data_sum);
    }

    T pop()
    {
        P(&_data_sum);
        lock(&_pop_mutex);
        T temp = _queue[_pop_idx].pop();
        ++_pop_idx;
        _pop_idx %= _capa;
        unlock(&_pop_mutex);
        V(&_space_sum);
        return std::move(temp);
    }

    private:
    inline void P(sem* sum)
    {
        sem_wait(sum);
    }

    inline void V(sem* sum)
    {
        sem_post(sum);
    }

    inline void lock(mutex* m)
    {
        pthread_mutex_lock(m);
    }

    inline void unlock(mutex* m)
    {
        pthread_mutex_unlock(m);
    }

    private:
    size_t _capa;
    vector _queue;
    int _push_idx;
    int _pop_idx;
    sem _space_sum;
    sem _data_sum;
    mutex _push_mutex;
    mutex _pop_mutex;
};