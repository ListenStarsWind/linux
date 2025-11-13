#pragma once

#include <pthread.h>

struct lock_guard {
    pthread_mutex_t* _mutex;
    lock_guard(pthread_mutex_t* mutex) : _mutex(mutex) {
        pthread_mutex_lock(_mutex);
    };
    ~lock_guard() {
        pthread_mutex_unlock(_mutex);
    };
};
