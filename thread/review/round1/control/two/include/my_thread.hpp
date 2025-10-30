#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// ================================================================
// 一、系统级头文件（必须在最前）
// ================================================================
#define _GNU_SOURCE     // 必须在所有系统头之前
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
    size_t size;            // 必须第一个
    uint64_t flags;         // 必填
    uint64_t pidfd;         // 可 0
    uint64_t child_tid;     // CLONE_CHILD_CLEARTID 时使用
    uint64_t parent_tid;    // CLONE_PARENT_SETTID 时使用
    uint64_t exit_signal;   // 必填（SIGCHLD）
    uint64_t stack;         // 子进程栈底指针（高地址）
    uint64_t stack_size;    // 可 0
    uint64_t tls;           // TLS 描述符地址
    uint64_t set_tid;       // 可 0
    uint64_t set_tid_size;  // 可 0
    uint64_t cgroup;        // 可 0
};

// ================================================================
// 三、项目级常量定义（不依赖任何实现）
// ================================================================
enum {
    STACK_SIZE = 1 << 21,  // 2MB 栈
    GUARD_SIZE = 1 << 12,  // 4KB 守护页
    TLS_SIZE = 1 << 12,    // 4KB TLS
    TOTAL_SIZE = STACK_SIZE + GUARD_SIZE + TLS_SIZE
};

// ===================================================================
// 主模板
// ===================================================================
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
    std::shared_ptr<void> mem;  // 栈 + 守护页 + TLS
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
        static int routine(void* arg);
    };

   public:
    template <typename Func, typename... Args>
    static std::unique_ptr<self_t> create(Func&& func, Args&&... args);

    void join();  // 返回 void

    myThread(const myThread&) = delete;
    myThread& operator=(const myThread&) = delete;
    myThread(myThread&&) = default;
    myThread& operator=(myThread&&) = default;
    ~myThread() = default;

   private:
    template <typename Func, typename... Args>
    explicit myThread(Func&& func, Args&&... args);

   private:
    std::shared_ptr<void> mem;
    pid_t tid{};
    pid_t child_tid{};
    std::function<void()> task;  // 无返回值

    static constexpr size_t STACK_SIZE = ::STACK_SIZE;  // 2MB
    static constexpr size_t GUARD_SIZE = ::GUARD_SIZE;  // 4KB
    static constexpr size_t TLS_SIZE = ::TLS_SIZE;      // 4KB
    static constexpr size_t TOTAL_SIZE = ::TOTAL_SIZE;
};

// ===================================================================
// 实现
// ===================================================================

template <typename Result_t>
int myThread<Result_t>::fn::routine(void* arg) {
    auto* self = static_cast<self_t*>(arg);
    self->result = self->task();
    syscall(SYS_exit, 0);
    // return 0;
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
    // 分配：守护页 | 栈 | TLS
    void* ptr =
        mmap(nullptr, TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) return;

    mem = std::shared_ptr<void>(ptr, [](void* p) { munmap(p, TOTAL_SIZE); });

    char* base = static_cast<char*>(mem.get());

    // 守护页
    if (mprotect(base, GUARD_SIZE, PROT_NONE) == -1) {
        mem.reset();
        return;
    }

    auto stack = static_cast<char*>(base) + GUARD_SIZE;  // 栈底
    // 64为系统要求基地址要对齐16字节, 也就是低四位要抹零
    auto stack_top =
        reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(stack + STACK_SIZE) & ~0xF));  // 栈顶
    void* tls_base = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(stack_top + 0x10) &
                                              ~0xF));  // 局部存储, 与栈顶间隔一段距离, 防止干扰栈

    // clone
    const int flags = CLONE_VM |              // 1. 共享内存空间
                      CLONE_FS |              // 2. 共享文件系统信息（cwd, root）
                      CLONE_FILES |           // 3. 共享文件描述符表
                      CLONE_SIGHAND |         // 4. 共享信号处理程序
                      CLONE_THREAD |          // 5. 加入同一线程组（TGID 相同）
                      CLONE_SYSVSEM |         // 6. 共享 System V 信号量调整
                      CLONE_PARENT_SETTID |   // 7. 父进程写入子 tid
                      CLONE_CHILD_CLEARTID |  // 8. 子退出时清零 child_tid + futex 唤醒
                      CLONE_SETTLS |          // 9. 设置 TLS（配合tls_base）
                      SIGCHLD;                // 10. 退出时发 SIGCHLD 信号

    // int ret = clone(&fn::routine, stack_top, flags, this, &tid, tls_base, &child_tid);

    int ret = syscall(SYS_clone, flags, stack_top, &tid, &child_tid, tls_base);
    if (ret == -1) {
        mem.reset();
        return;
    }

    // 确保内核异步写入后, 即真正完成轻量级进程创建后, 应用层再完成创建
    while (child_tid == 0) sched_yield();
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
    if (!ptr->mem) return nullptr;
    return ptr;
}

inline int myThread<void>::fn::routine(void* arg) {
    auto* self = static_cast<self_t*>(arg);
    self->task();
    // syscall(SYS_exit, 0);
    return 0;
}

template <typename Func, typename... Args>
myThread<void>::myThread(Func&& func, Args&&... args)
    : task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...)) {
    void* ptr =
        mmap(nullptr, TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) return;

    mem = std::shared_ptr<void>(ptr, [](void* p) { munmap(p, TOTAL_SIZE); });
    char* base = static_cast<char*>(mem.get());

    // 为整个空间进行清零初始化, 防止异常干扰, 特别是局部存储
    memset(base, 0, TOTAL_SIZE);

    if (mprotect(base, GUARD_SIZE, PROT_NONE) == -1) {
        mem.reset();
        return;
    }

    auto stack = static_cast<char*>(base) + GUARD_SIZE;  // 栈底
    // 64为系统要求基地址要对齐16字节, 也就是低四位要抹零
    auto stack_top =
        reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(stack + STACK_SIZE) & ~0xF));  //栈顶
    void* tls_base = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(stack_top + 0x10) &
                                              ~0xF));  // 局部存储, 与栈顶间隔一段距离,
                                              //防止干扰栈

    // // clone
    // const int flags = CLONE_VM |              // 1. 共享内存空间
    //                   CLONE_FS |              // 2. 共享文件系统信息（cwd, root）
    //                   CLONE_FILES |           // 3. 共享文件描述符表
    //                   CLONE_SIGHAND |         // 4. 共享信号处理程序
    //                   CLONE_THREAD |          // 5. 加入同一线程组（TGID 相同）
    //                   CLONE_SYSVSEM |         // 6. 共享 System V 信号量调整
    //                   CLONE_PARENT_SETTID |   // 7. 父进程写入子 tid
    //                   CLONE_CHILD_CLEARTID |  // 8. 子退出时清零 child_tid + futex 唤醒
    //                   CLONE_SETTLS |          // 9. 设置 TLS（配合 user_desc）
    //                   SIGCHLD;                // 10. 退出时发 SIGCHLD 信号

    struct clone_args params;
    memset(&params, 0, sizeof(params));
    params.flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                   CLONE_SYSVSEM
                   // 注意：是否加入 CLONE_THREAD 取决于你是否需要线程组行为。
                   // 先不加 CLONE_THREAD 做测试更稳妥；若要 pthread 等价语义再加上它。
                   | CLONE_THREAD
                   | CLONE_PARENT_SETTID  // 内核会在父进程写入 child tid（parent copy）
                   | CLONE_CHILD_CLEARTID  // 内核在 child 退出时把 *child_tid = 0 并 futex_wake
                   | CLONE_SETTLS;         // 使用 .tls 字段设置 FS（x86_64）
    // args.exit_signal = SIGCHLD; // exit_signal used when no CLONE_THREAD and for process-like
    // behavior
    params.stack = (uintptr_t)stack_top;  // 子线程的初始栈指针
    params.stack_size = 0;                // 可选（0 表示不另行指定）
    params.parent_tid = (uintptr_t)&tid;  // parent copy (内核把 child's TID写到这里作为父的备份) 
    params.child_tid = (uintptr_t)&child_tid;  // child清零地址（必须为用户可写地址） 
    params.tls = (uintptr_t)tls_base;          // FS base for child (x86_64)

    long ret = syscall(SYS_clone3, &params, sizeof(params));

    if(ret == 0)
    {
        // 创建成功, 将执行流引导至执行函数中
        fn::routine(this);
        // 向编译器提示, 这里不会被执行
        __builtin_unreachable();
    }

    // int ret = clone(&fn::routine, stack_top, flags, this, &tid, tls_base, &child_tid);

    // fprintf(stderr,
    //         "[debug] clone returned=%d, tid(parent copy)=%d, child_tid_addr=%p, "
    //         "child_tid_value(before)=%d\n",
    //         ret, tid, (void*)&child_tid, child_tid);

    if (ret == -1) {
        mem.reset();
        return;
    }

    while (child_tid == 0) sched_yield();
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

// template <typename Func, typename... Args>
// inline std::unique_ptr<myThread<void>> myThread<void>::create(Func&& func, Args&&... args) {
//     auto* raw_ptr = new myThread<void>(std::forward<Func>(func), std::forward<Args>(args)...);
//     auto ptr = std::unique_ptr<myThread<void>>(raw_ptr);
//     if (!ptr->mem) return nullptr;
//     return ptr;
// }

// inline int myThread<void>::fn::routine(void* arg) {
//     auto* self = static_cast<self_t*>(arg);
//     self->task();
//     return 0;
// }

// template <typename Func, typename... Args>
// myThread<void>::myThread(Func&& func, Args&&... args)
//     : task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...)) {
//     void* ptr =
//         mmap(nullptr, TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
//     if (ptr == MAP_FAILED) return;
//     mem = std::shared_ptr<void>(ptr, [](void* p) { munmap(p, TOTAL_SIZE); });

//     char* base = static_cast<char*>(mem.get());
//     memset(base, 0, TOTAL_SIZE);

//     if (mprotect(base, GUARD_SIZE, PROT_NONE) == -1) {
//         mem.reset();
//         return;
//     }

//     char* stack = base + GUARD_SIZE;
//     char* stack_top =
//         reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(stack + STACK_SIZE) & ~0xF));

//     void* tls_base =
//         reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(stack_top + 0x10) & ~0xF));

//     auto params = new struct clone_args();

//     params->flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM |
//                     CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | CLONE_SETTLS;
//     params->stack = (uintptr_t)stack_top;
//     params->parent_tid = (uintptr_t)&tid;
//     params->child_tid = (uintptr_t)&child_tid;
//     params->tls = (uintptr_t)tls_base;
//     params->exit_signal = SIGCHLD;  // 必须设置
//     params->size = sizeof(clone_args);

//     long ret = syscall(SYS_clone3, params, sizeof(clone_args));

//     if (ret == 0) {
//         // 子线程执行
//         task();
//         delete params;
//         // 清理 child_tid 并唤醒父线程
//         // __atomic_store_n(&child_tid, 0, __ATOMIC_SEQ_CST);
//         syscall(SYS_exit, 0);
//     }

//     if (ret == -1) {
//         mem.reset();
//         return;
//     }

//     // 父线程等待子线程写入 tid
//     while (child_tid == 0) sched_yield();
// }

// inline void myThread<void>::join() {
//     int expected = __atomic_load_n(&child_tid, __ATOMIC_SEQ_CST);
//     if (expected == 0) return;

//     // 重设期待值
//     expected = 0;
//     while (true) {
//         long ret = syscall(SYS_futex, &child_tid, FUTEX_WAIT, expected, nullptr, nullptr, 0);
//         if (ret == 0) return;
//         if (errno == EAGAIN) return;
//         if (errno == EINTR) continue;
//         return;
//     }
// }