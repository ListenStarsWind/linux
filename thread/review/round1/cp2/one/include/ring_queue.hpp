#pragma once

#include <pthread.h>
#include <semaphore.h>

#include <vector>

#include "data_guard.hpp"
#include "lock_guard.hpp"

template <typename T>
class ring_queue {
    using data_base_t = T;
    using data_t = data_guard<data_base_t>;
    using data_base_ptr_t = typename data_t::data_ptr_t;
    using function_t = typename data_t::function_t;
    using vector = std::vector<data_t>;

    using sem_t = ::sem_t;
    using sem_ptr_t = std::unique_ptr<sem_t, int (*)(sem_t*)>;

    using mutex_t = pthread_mutex_t;
    using mutex_ptr_t = std::unique_ptr<mutex_t, int (*)(mutex_t*)>;

   public:
    ring_queue(size_t capa, function_t in_log = empty_log, function_t out_log = empty_log)
        : _capa(capa),
          _push_idx(0),
          _pop_idx(0),
          _space_sum(new sem_t, ::sem_destroy),
          _data_sum(new sem_t, ::sem_destroy),
          _push_mutex(new mutex_t, ::pthread_mutex_destroy),
          _pop_mutex(new mutex_t, ::pthread_mutex_destroy) {
        sem_init(_space_sum.get(), 0, _capa);
        sem_init(_data_sum.get(), 0, 0);
        pthread_mutex_init(_push_mutex.get(), nullptr);
        pthread_mutex_init(_pop_mutex.get(), nullptr);
        _queue.reserve(_capa);
        for (size_t i = 0; i <= _capa; ++i) {
            _queue.emplace_back(in_log, out_log);
        }
    }

    ~ring_queue() = default;

    template <typename... Args>
    void push(Args&&... args) {
        P(_space_sum);
        {
            lock_guard lock(_push_mutex.get());
            _queue[_push_idx].input(std::forward<Args>(args)...);
            _push_idx++;
            _push_idx %= _capa;
        }
        V(_data_sum);
    }

    data_base_ptr_t pop() {
        data_base_ptr_t data;
        P(_data_sum);
        {
            lock_guard lock(_pop_mutex.get());
            data = _queue[_pop_idx].output();
            _pop_idx++;
            _pop_idx %= _capa;
        }
        V(_space_sum);
        return std::move(data);
    }

   private:
    void P(sem_ptr_t& sum) {
        sem_wait(sum.get());
    }
    void V(sem_ptr_t& sum) {
        sem_post(sum.get());
    }

    static void empty_log(const data_base_t&) {};

   private:
    size_t _capa;
    vector _queue;
    size_t _push_idx;
    size_t _pop_idx;
    sem_ptr_t _space_sum;
    sem_ptr_t _data_sum;
    mutex_ptr_t _push_mutex;
    mutex_ptr_t _pop_mutex;
};