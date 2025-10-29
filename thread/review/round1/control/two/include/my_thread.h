#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>

#include <functional>
#include <memory>

template <typename Result_t>
class myThread {
    using self_t = myThread<Result_t>;

    class fn {
       public:
        static int routine(void* arg);
    };

   public:
    // 静态创建 + RAII
    template <typename Func, typename... Args>
    static std::unique_ptr<self_t> create(Func&& func, Args&&... args);

    // 等待线程结束并获取结果
    Result_t join();

    // 禁止拷贝，允许移动
    myThread(const myThread&) = delete;
    myThread& operator=(const myThread&) = delete;
    myThread(myThread&&) = default;
    myThread& operator=(myThread&&) = default;

    ~myThread() = default;

   private:
    template <typename Func, typename... Args>
    explicit myThread(Func&& func, Args&&... args);

   private:
    std::shared_ptr<void> mem;  // 栈 + 守护页 + TLS
    pid_t tid{};
    pid_t child_tid{};
    Result_t result{};
    std::function<Result_t()> task;

    static constexpr size_t STACK_SIZE = 1 << 21;  // 2MB
    static constexpr size_t GUARD_SIZE = 1 << 12;  // 4KB
    static constexpr size_t TLS_SIZE = 1 << 12;    // 4KB
    static constexpr size_t TOTAL_SIZE = STACK_SIZE + GUARD_SIZE + TLS_SIZE;
};