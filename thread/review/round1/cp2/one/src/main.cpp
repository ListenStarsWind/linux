#include <unistd.h>

#include <format>
#include <iostream>
#include <random>
#include <unordered_map>

#include "ring_queue.hpp"

using queue_t = ring_queue<double>;
using hash_t = std::unordered_map<pthread_t, int>;

using dist_t = std::uniform_real_distribution<double>;

struct producer_args {
    move_only_function<double(void)> _random;
    queue_t _queue;
};

void* producer(void* p) {
    pthread_detach(pthread_self());
    auto args = static_cast<producer_args*>(p);
    while (true) {
        auto val = args->_random();
        args->_queue.push(val);
        sleep(1);
    }

    return nullptr;
}

void* consumer(void* p)
{
    pthread_detach(pthread_self());
    auto queue = static_cast<queue_t*>(p);
    while(true)
    {
        auto val = queue->pop();
        sleep(1);
    }
    return nullptr;
}

int main() {
    


    return 0;
}
