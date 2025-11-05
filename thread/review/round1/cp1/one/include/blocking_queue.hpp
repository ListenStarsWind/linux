#pragma once
// #define USE_CLASSIC_COND_SCHEME

#include <pthread.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <queue>
#include <boost/format.hpp>

struct lock_guard {
    pthread_mutex_t* _mutex;
    lock_guard(pthread_mutex_t* mutex) : _mutex(mutex) {
        pthread_mutex_lock(_mutex);
    };
    ~lock_guard() {
        pthread_mutex_unlock(_mutex);
    };
};

template <typename T>
class blocking_queue {
    // unique_ptr 明确生命周期, 防止因拷贝造成的生命周期异常
    using queue_t = std::queue<T>;
    using queue_ptr_t = std::unique_ptr<queue_t>;

    using mutex_t = pthread_mutex_t;
    using cond_t = pthread_cond_t;

    using mutex_ptr_t = std::unique_ptr<mutex_t, void (*)(mutex_t*)>;
    using cond_ptr_t = std::unique_ptr<cond_t, void (*)(cond_t*)>;

    using self_t = blocking_queue<T>;
    using self_ptr_t = std::unique_ptr<self_t>;

   public:
    blocking_queue(size_t max_size = 12)
#ifdef USE_CLASSIC_COND_SCHEME
        : _max_size(max_size),
#else
        : _left_size(max_size / 3),
          _right_size(max_size * 2 / 3),
          _wake_batch_size(_right_size - _left_size),
#endif
          _queue(std::make_unique<queue_t>()),
          _mutex(new mutex_t, delete_mutex_func),
          _not_full(new cond_t, delete_cond_func),
          _not_empty(new cond_t, delete_cond_func) {
        pthread_mutex_init(_mutex.get(), nullptr);
        pthread_cond_init(_not_full.get(), nullptr);
        pthread_cond_init(_not_empty.get(), nullptr);
    }

    template <typename... Args>
    void emplace(Args... args) {
        lock_guard lock(_mutex.get());

        // 理论上用 if 就可以, 但这里的while是为了防止某些异常唤醒场景
        // 在异常唤醒情况下, 尽管线程获得锁, 但可能条件仍不满足, 此时应该让它再次进入等待队列,
        // 而不是直接出去
#ifdef USE_CLASSIC_COND_SCHEME
        while (_queue->size() >= _max_size)
#else
        while (_queue->size() >= _right_size)
#endif
        {
            pthread_cond_wait(_not_full.get(), _mutex.get());
        }
        _queue->emplace(std::forward<Args>(args)...);
        std::cout << (boost::format("生产了一份新的数据: %1%\n") % static_cast<std::string>(_queue->back())).str();

        // 唤醒一个或一批线程

#ifdef USE_CLASSIC_COND_SCHEME
        pthread_cond_signal(_not_empty.get());
#else
        if (_queue->size() == _right_size) {
            for (size_t i = 0; i < _wake_batch_size; ++i) {
                pthread_cond_signal(_not_empty.get());
            }
        }
#endif
    };

    // 很明显, 为了保护其中的资源, 我们不能返回引用
    // 另外, 为了确保取出和移除队首元素的原子性
    // 我们需要把front和pop合并
    T front() {
        T result;
        {
            lock_guard lock(_mutex.get());
#ifdef USE_CLASSIC_COND_SCHEME
            while (_queue->size() == 0)
#else
            // _left_size 可能过小, 导致其为零
            while (_queue->size() < _left_size)
#endif
            {
                pthread_cond_wait(_not_empty.get(), _mutex.get());
            }
            result = std::move(_queue->front());
            _queue->pop();
            std::cout << (boost::format("获取了一份新的数据: %1%\n") % static_cast<std::string>(result)).str();

#ifdef USE_CLASSIC_COND_SCHEME
            pthread_cond_signal(_not_full.get());
#else
            if (_queue->size() == _left_size) {
                for (size_t i = 0; i < _wake_batch_size; ++i) {
                    pthread_cond_signal(_not_full.get());
                }
            }
#endif
        }

        return result;
    }

   private:
    static void delete_mutex_func(mutex_t* ptr) {
        pthread_mutex_destroy(ptr);
        delete ptr;
    }

    static void delete_cond_func(cond_t* ptr) {
        pthread_cond_destroy(ptr);
        delete ptr;
    }

   private:
#ifdef USE_CLASSIC_COND_SCHEME
    size_t _max_size;
#else
    size_t _left_size;
    size_t _right_size;
    const size_t _wake_batch_size;
#endif
    queue_ptr_t _queue;
    mutex_ptr_t _mutex;
    cond_ptr_t _not_full;
    cond_ptr_t _not_empty;
};
