#include <unistd.h>

#include <format>
#include <iostream>
#include <random>
#include <unordered_map>

#include "task.hpp"
#include "ring_queue.hpp"

using queue_t = ring_queue<task>;
using hash_t = std::unordered_map<pthread_t, int>;

void* producer(void* p) {
    pthread_detach(pthread_self());
    std::mt19937_64 gen(std::random_device{}());
    std::uniform_real_distribution<double> value_dist(-1e3, 1e3);
    std::uniform_int_distribution<int> opid_dist(0, 3);
    static thread_local const char ops[] = {'+', '-', '*', '/'};

    auto queue = static_cast<queue_t*>(p);
    while (true) {
        // 生成两个随机数和一个运算符
        std::string left = std::to_string(value_dist(gen));
        std::string right = std::to_string(value_dist(gen));
        char op = ops[opid_dist(gen)];
        queue->push(left + op + right);
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
    hash_t producers;
    hash_t consumers;

    queue_t queue(
        5,
        [&producers](task& val) {
            std::cout << std::format("\033[31m[producer-{}]info$ push: {}\033[0m\n",
                                     producers[pthread_self()], static_cast<std::string>(val));
        },
        [&consumers](task& val) {
            val();
            std::cout << std::format("\033[32m[consumer-{}]info$ pop : {}\033[0m\n",
                                     consumers[pthread_self()], static_cast<std::string>(val));
        });

    for (int i = 1; i <= 9; ++i) {
        pthread_t p;
        pthread_create(&p, nullptr, producer, &queue);
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
