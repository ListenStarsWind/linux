#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// ================================================================
// 一、系统级头文件（必须在最前）
// ================================================================
#include <asm/prctl.h>  // ARCH_SET_FS / ARCH_GET_FS
#include <errno.h>
#include <linux/futex.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>  // SYS_clone3
#include <sys/types.h>
#include <unistd.h>

// ================================================================
// 二、C++ 标准库（放在系统头之后）
// ================================================================
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <utility>
// ---------------------------------------------------------------
// 手动定义 clone_args（覆盖可能不完整的 <linux/sched.h>）
// ---------------------------------------------------------------
struct clone_args {
    uint64_t flags;
    uint64_t pidfd;
    uint64_t child_tid;
    uint64_t parent_tid;
    uint64_t exit_signal;
    uint64_t stack;
    uint64_t stack_size;
    uint64_t tls;
    uint64_t set_tid;
    uint64_t set_tid_size;
    uint64_t cgroup;
    void* (*fn)(void*);
    void* fn_arg;
};

// ========== 局部存储基地址引导结构 ==========
struct tls_descriptor {
    void* tcb;           // 指向自身
    void* dtv;           // 动态线程向量
    void* self;          // 指向 pthread 结构体
    void* _padding[13];  // 填充到 128 字节（glibc 实际更大）
    void* stack_guard;   // 栈保护
    // 更多字段... 我们只初始化关键的
};

// ================================================================
// 三、项目级常量定义（不依赖任何实现）
// ================================================================
enum {
    STACK_SIZE = 1 << 21,  // 2MB 栈
    GUARD_SIZE = 1 << 12,  // 4KB 守护页
    TLS_SIZE = 1 << 12,    // 4KB TLS
    TOTAL_SIZE = STACK_SIZE + GUARD_SIZE
};

// ===================================================================
// 主模板
// ===================================================================
template <typename Result_t>
class myThread {
    using self_t = myThread<Result_t>;

    class fn {
       public:
        static void* routine(void* arg);
    };

   public:
    // 静态创建 + RAII
    template <typename Func, typename... Args>
    static std::unique_ptr<self_t> create(Func&& func, Args&&... args);

    // 基于事件驱动的汇合
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
    std::shared_ptr<char> stack_base;
    std::shared_ptr<char> tls_base;
    pid_t tid{};
    pid_t child_tid{};
    Result_t result{};
    std::function<Result_t()> task;

    static constexpr size_t STACK_SIZE = ::STACK_SIZE;  // 2MB
    static constexpr size_t GUARD_SIZE = ::GUARD_SIZE;  // 4KB
    static constexpr size_t TLS_SIZE = ::TLS_SIZE;      // 4KB
    static constexpr size_t TOTAL_SIZE = ::TOTAL_SIZE;
};

// ===================================================================
// void 特化
// ===================================================================
template <>
class myThread<void> {
    using self_t = myThread<void>;

    class fn {
       public:
        static void* routine(void* arg);
    };

   public:
    template <typename Func, typename... Args>
    static std::unique_ptr<self_t> create(Func&& func, Args&&... args);

    void join();

    myThread(const myThread&) = delete;
    myThread& operator=(const myThread&) = delete;
    myThread(myThread&&) = default;
    myThread& operator=(myThread&&) = default;
    ~myThread() = default;

   private:
    template <typename Func, typename... Args>
    explicit myThread(Func&& func, Args&&... args);

   private:
    std::shared_ptr<char> stack_base;
    std::shared_ptr<char> tls_base;
    pid_t tid = 0;
    pid_t child_tid = 0;
    std::function<void()> task;

    static constexpr size_t STACK_SIZE = ::STACK_SIZE;  // 2MB
    static constexpr size_t GUARD_SIZE = ::GUARD_SIZE;  // 4KB
    static constexpr size_t TLS_SIZE = ::TLS_SIZE;      // 4KB
    static constexpr size_t TOTAL_SIZE = ::TOTAL_SIZE;
};

// ===================================================================
// 模版实现
// ===================================================================

template <typename Result_t>
void* myThread<Result_t>::fn::routine(void* arg) {
    auto* self = static_cast<self_t*>(arg);
    self->result = self->task();
    return nullptr;
}

template <typename Result_t>
template <typename Func, typename... Args>
inline std::unique_ptr<myThread<Result_t>> myThread<Result_t>::create(Func&& func, Args&&... args) {
    auto* raw_ptr = new myThread<Result_t>(std::forward<Func>(func), std::forward<Args>(args)...);
    auto ptr = std::unique_ptr<myThread<Result_t>>(raw_ptr);
    if (!ptr->mem) return nullptr;
    return ptr;
}

template <typename Result_t>
template <typename Func, typename... Args>
myThread<Result_t>::myThread(Func&& func, Args&&... args)
    : task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...)) {
    // 栈及守护页空间申请
    void* ptr1 =
        mmap(nullptr, TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // 申请失败. 提前结束
    if (ptr1 == MAP_FAILED) return;

    // 生命周期托管至智能指针
    stack_base =
        std::shared_ptr<char>(static_cast<char*>(ptr1), [](char* p) { munmap(p, TOTAL_SIZE); });

    auto stack_base__ = stack_base.get();

    // 为整个空间进行清零初始化, 防止异常干扰
    memset(stack_base__, 0, TOTAL_SIZE);

    // 修改守护页属性, 取消所有属性, 实现自爆功能
    if (mprotect(stack_base__, GUARD_SIZE, PROT_NONE) == -1) {
        stack_base.reset();
        return;
    }

    auto stack_bottom = stack_base__ + GUARD_SIZE;  // 栈底
    char* stack_top = stack_bottom + STACK_SIZE;
    // 64为系统要求基地址要对齐16字节, 也就是低四位抹零 ULL表示无符号长整型
    stack_top = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(stack_top) & ~0xFULL);

    // 单独开辟 局部存储空间, 一块开辟容易相互干扰
    void* ptr2 =
        mmap(nullptr, TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // 申请失败, 提前终止
    if (ptr2 == MAP_FAILED) return;

    // 生命周期托管至智能指针
    tls_base =
        std::shared_ptr<char>(static_cast<char*>(ptr2), [](char* p) { munmap(p, TLS_SIZE); });

    auto tls_base__ = tls_base.get();

    // 清零初始化防止原先数据干扰
    memset(tls_base__, 0, TLS_SIZE);

    // 局部存储要求 64 字节对齐, 即把 0~63以下的数位抹去
    tls_base__ = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(tls_base.get()) & ~63ULL);

    // 准备对基地址引导结构进行初始化
    auto boot = reinterpret_cast<struct tls_descriptor*>(tls_base__);
    boot->tcb = tls_base__;
    boot->self = tls_base__;
    boot->dtv = nullptr;

    // 其余部分不使用, 填充垃圾
    boot->stack_guard = reinterpret_cast<void*>(0x123456789ABCDEF0ULL);

    struct clone_args params;
    memset(&params, 0, sizeof(struct clone_args));

    params.flags = CLONE_VM |  // 与父进程共享虚拟内存（创建线程而非独立进程）
                   CLONE_FS |  // 与父进程共享文件系统信息（cwd、umask 等）
                   CLONE_FILES |           // 与父进程共享文件描述符表
                   CLONE_SIGHAND |         // 与父进程共享信号处理器
                   CLONE_SYSVSEM |         // 与父进程共享 System V 信号量
                   CLONE_PARENT_SETTID |   // 将子线程 ID 存入父线程提供的地址
                   CLONE_CHILD_CLEARTID |  // 子线程退出时清除其线程 ID
                   CLONE_SETTLS;           // 设置子线程的线程局部存储（TLS）

    params.stack = reinterpret_cast<uint64_t>(stack_top);
    params.stack_size = STACK_SIZE;
    params.parent_tid = reinterpret_cast<uint64_t>(&tid);
    params.child_tid = reinterpret_cast<uint64_t>(&child_tid);
    params.tls = reinterpret_cast<uint64_t>(tls_base__);
    params.exit_signal = SIGCHLD;
    params.fn = &fn::routine;
    params.fn_arg = this;

    auto ret = syscall(SYS_clone3, &params, sizeof(struct clone_args));

    if (ret > 0) {
        // 返回轻量级进程 ID
        tid = ret;
        // 确保内核构造成功后应用层再返回
        while (child_tid == 0) sched_yield();
    }

    if (ret < 0) {
        // 调用失败, 致命错误
        stack_base.reset();
        tls_base.reset();
        return;
    }

}

template <typename Result_t>
Result_t myThread<Result_t>::join() {
    // 原子性获取 child_tid
    int expected = __atomic_load_n(&child_tid, __ATOMIC_SEQ_CST);

    // 为零, 说明子线程已经结束 直接汇合
    if (expected == 0) return result;

    while (true) {
        // 使用系统调用 SYS_futex, 为内核种下如下事件
        // 当child_tid与预期值expected不相同时, 触发事件
        // 处理逻辑为将线程从等待队列移到运行队列, 否则继续待在等待队列
        long ret = syscall(SYS_futex, &child_tid, FUTEX_WAIT, expected, nullptr, nullptr, 0);

        // syscall 会返回 0 或者 -1 , 分别代表调用成功和失败
        if (ret == 0) break;  // 事件种下成功, 等待一定时间后, 被唤醒, 获取到返回值, 为零

        // 接下来是针对失败(ret == -1)原因的细分
        if (errno == EAGAIN) {
            // 事件种下失败, 但失败原因是内核在尝试种下事件时, 发现child_tid和expected 已经不同,
            // 事件不在有种下的必要
            break;
        } else if (errno == EINTR) {
            // 因为信号的中断导致种下失败, 继续重试种事件
            continue;
        } else {
            // 其它恶性错误, 再怎么重试, 都种不上
            break;
        }
    }

    return result;
}

// === void 特化 ===
template <typename Func, typename... Args>
inline std::unique_ptr<myThread<void>> myThread<void>::create(Func&& func, Args&&... args) {
    auto* raw_ptr = new myThread<void>(std::forward<Func>(func), std::forward<Args>(args)...);
    auto ptr = std::unique_ptr<myThread<void>>(raw_ptr);
    if (ptr->tls_base == nullptr || ptr->stack_base == nullptr) return nullptr;
    return ptr;
}

inline void* myThread<void>::fn::routine(void* arg) {
    auto* self = static_cast<self_t*>(arg);
    if (self->task) self->task();
    return nullptr;
}

template <typename Func, typename... Args>
myThread<void>::myThread(Func&& func, Args&&... args)
    : task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...)) {
    // 栈及守护页空间申请
    void* ptr1 =
        mmap(nullptr, TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // 申请失败. 提前结束
    if (ptr1 == MAP_FAILED) return;

    // 生命周期托管至智能指针
    stack_base =
        std::shared_ptr<char>(static_cast<char*>(ptr1), [](char* p) { munmap(p, TOTAL_SIZE); });

    auto stack_base__ = stack_base.get();

    // 为整个空间进行清零初始化, 防止异常干扰
    memset(stack_base__, 0, TOTAL_SIZE);

    // 修改守护页属性, 取消所有属性, 实现自爆功能
    if (mprotect(stack_base__, GUARD_SIZE, PROT_NONE) == -1) {
        stack_base.reset();
        return;
    }

    auto stack_bottom = stack_base__ + GUARD_SIZE;  // 栈底
    char* stack_top = stack_bottom + STACK_SIZE;
    // 64为系统要求基地址要对齐16字节, 也就是低四位抹零 ULL表示无符号长整型
    stack_top = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(stack_top) & ~0xFULL);

    // 单独开辟 局部存储空间, 一块开辟容易相互干扰
    void* ptr2 =
        mmap(nullptr, TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // 申请失败, 提前终止
    if (ptr2 == MAP_FAILED) return;

    // 生命周期托管至智能指针
    tls_base =
        std::shared_ptr<char>(static_cast<char*>(ptr2), [](char* p) { munmap(p, TLS_SIZE); });

    auto tls_base__ = tls_base.get();

    // 清零初始化防止原先数据干扰
    memset(tls_base__, 0, TLS_SIZE);

    // 局部存储要求 64 字节对齐, 即把 0~63以下的数位抹去
    tls_base__ = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(tls_base.get()) & ~63ULL);

    // 准备对基地址引导结构进行初始化
    auto boot = reinterpret_cast<struct tls_descriptor*>(tls_base__);
    boot->tcb = tls_base__;
    boot->self = tls_base__;
    boot->dtv = nullptr;

    // 其余部分不使用, 填充垃圾
    boot->stack_guard = reinterpret_cast<void*>(0x123456789ABCDEF0ULL);

    struct clone_args params;
    memset(&params, 0, sizeof(struct clone_args));

    params.flags = CLONE_VM |  // 与父进程共享虚拟内存（创建线程而非独立进程）
                   CLONE_FS |  // 与父进程共享文件系统信息（cwd、umask 等）
                   CLONE_FILES |           // 与父进程共享文件描述符表
                   CLONE_SIGHAND |         // 与父进程共享信号处理器
                   CLONE_SYSVSEM |         // 与父进程共享 System V 信号量
                   CLONE_PARENT_SETTID |   // 将子线程 ID 存入父线程提供的地址
                   CLONE_CHILD_CLEARTID |  // 子线程退出时清除其线程 ID
                   CLONE_SETTLS;           // 设置子线程的线程局部存储（TLS）

    params.stack = reinterpret_cast<uint64_t>(stack_top);
    params.stack_size = STACK_SIZE;
    params.parent_tid = reinterpret_cast<uint64_t>(&tid);
    params.child_tid = reinterpret_cast<uint64_t>(&child_tid);
    params.tls = reinterpret_cast<uint64_t>(tls_base__);
    params.exit_signal = SIGCHLD;
    params.fn = &fn::routine;
    params.fn_arg = this;

    auto ret = syscall(SYS_clone3, &params, sizeof(struct clone_args));

    if (ret > 0) {
        // 返回轻量级进程 ID
        tid = ret;
        // 确保内核构造成功后应用层再返回
        while (child_tid == 0) sched_yield();
    }

    if (ret < 0) {
        // 调用失败, 致命错误
        stack_base.reset();
        tls_base.reset();
        return;
    }
}

inline void myThread<void>::join() {
    int expected = __atomic_load_n(&child_tid, __ATOMIC_SEQ_CST);
    if (expected == 0) return;

    while (true) {
        long ret = syscall(SYS_futex, &child_tid, FUTEX_WAIT, expected, nullptr, nullptr, 0);
        if (ret == 0) return;
        if (errno == EAGAIN) return;
        if (errno == EINTR) continue;
        return;
    }
}
