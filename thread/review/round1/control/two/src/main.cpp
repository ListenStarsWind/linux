#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cerrno>
#include <memory>
#include <functional>
#include "my_thread.hpp"

int g = 0;

// 自定义线程工厂：使用 myThread<void>
template<typename ThreadFactory>
void test(ThreadFactory createThread, const char* tag) {
    std::cout << "\n=== 测试线程类型: " << tag << " ===\n";

    // 明确 fn 是个 void() 类型的可调用对象
    auto fn = [] {
        extern int g;
        g++;
        int local = 123;
        std::cout << "child: getpid=" << getpid()
                  << ", pthread_self=" << pthread_self()
                  << ", &errno=" << &errno
                  << ", &local=" << &local
                  << ", g=" << g << std::endl;
        usleep(100000);  // 模拟工作
    };

    auto t = createThread(fn);  

    std::cout << "parent: getpid=" << getpid()
              << ", pthread_self=" << pthread_self()
              << ", &errno=" << &errno
              << ", &g=" << &g << std::endl;

    if (t) t->join();
    std::cout << "parent after join: g=" << g << std::endl;
}

// Pthread 线程工厂
void* pthread_worker(void*) {
    g++;
    int local = 456;
    std::cout << "child(pthread): getpid=" << getpid()
              << ", pthread_self=" << pthread_self()
              << ", &errno=" << &errno
              << ", &local=" << &local
              << ", g=" << g << std::endl;
    usleep(100000);  // 模拟工作
    return nullptr;
}

int main() {
    // 测试 myThread<void> 行为
    test([](std::function<void()> f){
        return myThread<void>::create(f);
    }, "myThread<void>");

    // 测试原生 Pthread 行为
    std::cout << "\n=== 测试线程类型: Pthread ===\n";
    pthread_t tid;
    pthread_create(&tid, nullptr, pthread_worker, nullptr);
    std::cout << "parent: getpid=" << getpid()
              << ", pthread_self=" << pthread_self()
              << ", &errno=" << &errno
              << ", &g=" << &g << std::endl;
    pthread_join(tid, nullptr);  // 等待线程结束
    std::cout << "parent after join: g=" << g << std::endl;

    return 0;
}
