#include <pthread.h>
#include <unistd.h>

#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

static int tickets = 1000;

struct Args {
    std::string _id;
    pthread_mutex_t* _mutex;
    Args(int id, pthread_mutex_t* mutex) : _id(std::format("thread-{}", id)), _mutex(mutex) {}
};

struct mutex {
    pthread_mutex_t* _mutex;
    mutex(pthread_mutex_t* mutex) : _mutex(mutex) {
        pthread_mutex_lock(_mutex);
    };
    ~mutex() {
        pthread_mutex_unlock(_mutex);
    };
};

void* handler(void* p) {
    auto args = static_cast<Args*>(p);
    while (true) {
        mutex lock(args->_mutex);
        if (tickets > 0) {
            usleep(1000);
            std::cout << std::format("{}已经抢到一票, 剩余票数{}\n", args->_id, tickets);
            --tickets;
        } else {
            return nullptr;
        }
        usleep(10);
    }
}

int main() {
    using args_ptr_t = std::unique_ptr<Args>;
    using pair_t = std::pair<pthread_t, args_ptr_t>;
    using vector_t = std::vector<pair_t>;

    vector_t v;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);

    for (int i = 1; i <= 4; ++i) {
        pair_t pair;
        pair.second = std::make_unique<Args>(i, &mutex);
        pthread_create(&pair.first, nullptr, handler, pair.second.get());
        v.emplace_back(std::move(pair));
    }

    for (auto& e : v) {
        pthread_join(e.first, nullptr);
        e.second.reset();
    }

    pthread_mutex_destroy(&mutex);

    return 0;
}