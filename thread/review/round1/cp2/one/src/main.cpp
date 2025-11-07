#include <unistd.h>

#include <format>
#include <iostream>
#include <random>
#include <unordered_map>

#include "task.hpp"
#include "ring_queue.hpp"

using queue_t = ring_queue<double>;
using hash_t = std::unordered_map<pthread_t, int>;

using dist_t = std::uniform_real_distribution<double>;
using random_t = std::function<double(void)>;

struct producer_args {
    random_t* _random;
    queue_t* _queue;
};

void* producer(void* p) {
    pthread_detach(pthread_self());
    auto args = static_cast<producer_args*>(p);
    while (true) {
        auto val = args->_random->operator()();
        args->_queue->push(val);
        sleep(1);
    }

    return nullptr;
}

void* consumer(void* p) {
    pthread_detach(pthread_self());
    auto queue = static_cast<queue_t*>(p);
    while (true) {
        auto val = queue->pop();
        sleep(1);
    }
    return nullptr;
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-1e3, 1e3);

    hash_t producers;
    hash_t consumers;

    random_t random = [&gen, &dist]() { return dist(gen); };
    queue_t queue(
        5,
        [&producers](const double& val) {
            std::cout << std::format("\033[31m[producer-{}]info$ push: {}\033[0m\n",
                                     producers[pthread_self()], val);
        },
        [&consumers](const double& val) {
            std::cout << std::format("\033[32m[consumer-{}]info$ pop : {}\033[0m\n",
                                     consumers[pthread_self()], val);
        });

    producer_args pargs{&random, &queue};
    for (int i = 1; i <= 9; ++i) {
        pthread_t p;
        pthread_create(&p, nullptr, producer, &pargs);
        producers.emplace(p, i);
    }
    for (int i = 1; i <= 7; ++i) {
        pthread_t c;
        pthread_create(&c, nullptr, consumer, &queue);
        consumers.emplace(c, i);
    }

    while (true);

    return 0;
}
