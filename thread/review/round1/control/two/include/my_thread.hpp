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
    uint64_t flags;         // 要设置的标志位
    uint64_t pidfd;         // 新进程的 PID 文件描述符
    uint64_t child_tid;     // 子进程的线程 ID
    uint64_t parent_tid;    // 父进程的线程 ID
    uint64_t exit_signal;   // 进程退出时发送的信号
    uint64_t stack;         // 栈的起始地址
    uint64_t stack_size;    // 栈的大小
    uint64_t tls;           // 新进程的线程局部存储
    uint64_t set_tid;       // 设置线程 ID 的值
    uint64_t set_tid_size;  // 设置线程 ID 的大小
    uint64_t cgroup;        // 设置拥有者
};

/// ========== 局部存储用户级引导结构 (TCB) ==========
// 局部存储布局为: 高地址 dtv(内核初始化) tcb(用户指定) .....低地址
// ....低地址
struct tls_descriptor {
    void* tcb;  // 0: 指向自身 (TCB pointer itself)
    void* dtv;  // 8: 动态线程向量 (Dynamic Thread Vector)
};

// ========== 项目级常量定义 ==========
enum {
    STACK_SIZE = 1 << 21,                                // 2MB 栈
    GUARD_SIZE = 1 << 12,                                // 4KB 守护页
    TLS_SIZE = 1 << 12,                                  // 4KB TLS
    TOTAL_STACK_SIZE = (1 << 21) + (1 << 12),            // STACK_SIZE + GUARD_SIZE
    TOTAL_MMAP_SIZE = (1 << 21) + (1 << 12) + (1 << 12)  // TOTAL_STACK_SIZE + TLS_SIZE
};

// ===================================================================
// 三、类型统一化辅助
// ===================================================================

// 线程返回值占位符，用于 void 特化，以保持内存布局一致
struct thread_void_result {};

// ===================================================================
// 四、线程退出函数 (Exit Function)
// 这是线程完成任务后，最终跳转的地址。
// ===================================================================
extern "C" inline void thread_exit_func() {
    // 终止当前轻量级进程
    syscall(SYS_exit, 0);
}

// ===================================================================
// 五、汇编垫片 (Assembly Shim) 的 C 语言入口
// 线程启动后，第一个执行的函数。它负责将栈参数移动到 RDI 寄存器。
// ===================================================================
extern "C" void __mythread_c_entry(void* arg);  // 声明 C 逻辑入口`

extern "C" void __mythread_start_shim() {
    // 新的轻量级进程第一个动作是 ret: 将 RSP 寄存器指向存储__mythread_start_shim地址的控件
    //     高地址
    // +===================================+
    // | 0ULL (dummy)                  |  RSP+24
    // +-----------------------------------+
    // | &thread_exit_func             |  RSP+16   ← addq $16, %rsp 后
    // +-----------------------------------+
    // | this                          |  RSP+8    ← movq 8(%rsp), %rdi
    // +-----------------------------------+
    // | &__mythread_start_shim        |  RSP      ← 内核 ret 弹出
    // +===================================+
    // 低地址
    __asm__ volatile(
        // 现在 RSP 即栈维护指针指向存储 __mythread_start_shim 代码段地址的低地址处
        "movq 8(%%rsp), %%rdi \n"  // 把 RSP 所指地址往上加8个字节, 这样的新地址指向的就是存储 this
                                   // 的空间, 让后把 this 手动加到 rdi寄存器中,
                                   // rdi寄存器是C/C++传参时第一个参数写入的位置,注意,
                                   // RSP本身还没有改变, 仍旧是指向
                                   // __mythread_start_shim 代码段地址的低地址处
        "addq $16, %%rsp \n"  // 让 RSP 自加 16字节, 之后它指向存储 &thread_exit_func
                              // 这个数据的那份空间
        "jmp __mythread_c_entry \n"  //  跳转的 __mythread_c_entry 代码段入口
        // 执行 __mythread_c_entry 中的代码, 在这个栈上
        // __mythread_c_entry 的末尾会有一个C/C++自动加的汇编指令 ret
        // ret 再次执行, RSP 回到指向存储 &thread_exit_func这个数据的那份空间的状态
        // 自动利用 ret 的执行特性 执行thread_exit_func, 自动退出
        :
        :
        : "%rdi", "%rsp");
    // 理论上，控制流永远不会到达这里
    __builtin_unreachable();
}

// ===================================================================
// **修复方案：引入非模板基类以确保 C 语言入口的内存安全**
// 这个结构体包含所有 myThread<T> 实例的公共成员，确保它们在内存中的偏移量固定。
// ===================================================================
struct thread_common_state {
    typedef std::shared_ptr<char> mem_ptr_t;  // 托管内存的智能指针

    mem_ptr_t stack_base;
    mem_ptr_t tls_base;
    pid_t tid{};
    pid_t child_tid{};
    std::function<void()> task_executor;  // C 入口执行的 void() 执行器

    thread_common_state() = default;
    // 必须是非虚析构函数，以确保没有虚表指针 (vtable) 影响内存布局
    ~thread_common_state() = default;

    // 访问器，供 C 入口使用
    char* get_tls_base_addr() const {
        if (!stack_base) return nullptr;
        return stack_base.get() + TOTAL_STACK_SIZE;
    }

    // 友元声明：允许 C++ 入口访问私有成员
    friend void __mythread_c_entry(void* arg);
};

// ===================================================================
// 六、主模板定义 (Result_t != void)
// ===================================================================
template <typename Result_t>
class myThread : public thread_common_state {
    // C++11 检查：确保不是 void 类型
    static_assert(!std::is_void<Result_t>::value, "myThread<void> must use the specialization.");

   public:
    // C++11 类型别名
    typedef myThread<Result_t> self_t;
    typedef std::shared_ptr<self_t> managed_self_t;  // 托管自身的智能指针
    typedef std::shared_ptr<Result_t> result_ptr_t;  // 托管结果的智能指针

   public:
    // 静态创建 + RAII
    template <typename Func, typename... Args>
    static managed_self_t create(Func&& func, Args&&... args);

    // 基于事件驱动的汇合
    result_ptr_t join();

    // FIX: 新增公共访问器，供 myThread<void> 使用
    pid_t get_tid() const {
        return tid;
    }

    // 禁止拷贝，允许移动
    myThread(const myThread&) = delete;
    myThread& operator=(const myThread&) = delete;
    myThread(myThread&&) = default;
    myThread& operator=(myThread&&) = default;

    ~myThread() = default;

   private:
    template <typename Func, typename... Args>
    myThread(Func&& func, Args&&... args);

    // FIX: 新增私有方法用于封装线程启动逻辑
    bool start_thread();

    // FIX: 存储用户任务，等待 shared_ptr 封装后执行
    std::function<Result_t()> user_task;

   private:
    result_ptr_t result;
};

// ===================================================================
// 七、myThread<void> 特化：逻辑重定向, 委托模式
// ===================================================================
template <>
class myThread<void> : public thread_common_state {
    // 逻辑委托类型：使用 thread_void_result
    typedef myThread<thread_void_result> delegate_type;
    typedef std::shared_ptr<delegate_type> delegate_ptr_t;

   public:
    // C++11 类型别名
    typedef myThread<void> self_t;
    typedef std::shared_ptr<self_t> managed_self_t;

   public:
    // 静态创建 + RAII
    template <typename Func, typename... Args>
    static managed_self_t create(Func&& func, Args&&... args);

    // 基于事件驱动的汇合
    inline void join();

    // FIX: 新增公共访问器，供 myThread<void> 使用
    pid_t get_tid() const {
        return tid;
    }

    // 禁止拷贝，允许移动
    myThread(const myThread&) = delete;
    myThread& operator=(const myThread&) = delete;
    myThread(myThread&&) = default;
    myThread& operator=(myThread&&) = default;
    ~myThread() = default;

    // 友元声明：允许 C++ 入口访问私有成员
    friend void __mythread_c_entry(void* arg);

    // 利用委托实例化, 不需要构造函数

   private:
    // ------------------------------------------------------------------
    // 注意：由于使用了 aliasing constructor，这个类实例不会被实际创建
    // 这些成员现在由继承自 thread_common_state 的成员自动提供。
    // ------------------------------------------------------------------

    // std::shared_ptr<char> stack_base; // REMOVED
    // std::shared_ptr<char> tls_base;   // REMOVED
    // pid_t tid{};                      // REMOVED
    // pid_t child_tid{};                // REMOVED
    // std::shared_ptr<thread_void_result> result; // REMOVED (结果指针是 delegate_type 的私有成员)
    // std::function<void()> task_executor;        // REMOVED
};
// ===================================================================
// 八、C 语言唯一入口点实现
// 核心修复点：安全地将 void* 转换为 thread_common_state*
// ===================================================================
extern "C" void __mythread_c_entry(void* arg) {
    // --- 核心测试点：使用原始系统调用进行调试打印 ---
    // 目标：绕过 glibc 的 stdio 和内存锁，确认线程是否成功执行 C 代码
    const char* debug_msg = "Thread alive! Before task_executor.\n";
    syscall(SYS_write, 2, debug_msg, strlen(debug_msg));  // Write to stderr (fd 2)
    // ---------------------------------------------------------

    thread_common_state* self_base = static_cast<thread_common_state*>(arg);

    // // *** 注意：CLONE_SETTLS 已在 clone3 中设置 FS 寄存器。
    // // 这里再次调用 arch_prctl 可能会是冗余的，但在某些环境中可能是必要的。
    // // 为了与原意图保持一致，我们保留 arch_prctl 调用。
    // if (self_base) {
    //     // 使用成员函数获取 TCB 地址
    //     char* tcb_ptr = self_base->get_tls_base_addr();

    //     if (tcb_ptr) {
    //         // 使用 arch_prctl 系统调用设置 FS 寄存器，指向 TCB
    //         syscall(SYS_arch_prctl, ARCH_SET_FS, tcb_ptr);
    //     }
    // }

    if (self_base && self_base->task_executor) {
        // 执行包装好的任务，它会在 C++ 侧完成结果的创建和赋值
        self_base->task_executor();
    }
}

// ===================================================================
// 九、C++ 模板实现 (Result_t != void)
// ===================================================================

template <typename Result_t>
template <typename Func, typename... Args>
inline typename myThread<Result_t>::managed_self_t myThread<Result_t>::create(Func&& func,
                                                                              Args&&... args) {
    // printf("xsacxas");
    managed_self_t ptr(
        new myThread<Result_t>(std::forward<Func>(func), std::forward<Args>(args)...));
    // 检查构造函数是否失败（通过检查资源指针是否为空）
    // if (ptr->stack_base == nullptr || ptr->tls_base == nullptr) return managed_self_t();

    // 检查 new 是否失败
    if (!ptr) return managed_self_t();

    // 2. 核心 FIX: 捕获 shared_ptr 副本 (self_ref) 到 Lambda 中，以保证 myThread 实例的生命周期
    auto self_ref = ptr;

    ptr->task_executor = [self_ref]() {
        // self_ref (shared_ptr) 保证了 myThread 对象 (self_ref.get()) 在此 lambda 执行期间一直有效
        // printf("saxasx");
        auto p_this = self_ref.get();

        try {
            // 在子线程中创建结果对象，并托管给 result 成员。
            p_this->result.reset(new Result_t(p_this->user_task()));
            // 可选：清空 user_task 释放被捕获的资源
            p_this->user_task = nullptr;
        } catch (...) {
            p_this->result.reset();
        }
    };

    // 3. 启动线程
    if (!ptr->start_thread()) {
        return managed_self_t();
    }

    // 检查资源分配是否失败
    if (ptr->stack_base == nullptr || ptr->tls_base == nullptr) return managed_self_t();
    return ptr;
}

template <typename Result_t>
template <typename Func, typename... Args>
myThread<Result_t>::myThread(Func&& func, Args&&... args) {
    // 任务包装并保存到 user_task 成员变量
    this->user_task = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
}

// FIX: 封装原始的 clone3 线程启动逻辑
template <typename Result_t>
bool myThread<Result_t>::start_thread() {
    // --- 1. 栈及守护页空间申请 ---
    void* ptr1 = mmap(nullptr, TOTAL_STACK_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // 申请失败. 提前结束
    if (ptr1 == MAP_FAILED) return false;

    // 生命周期托管至智能指针
    stack_base = std::shared_ptr<char>(static_cast<char*>(ptr1),
                                       [](char* p) { munmap(p, TOTAL_STACK_SIZE); });

    auto stack_base__ = stack_base.get();

    // 为整个空间进行清零初始化, 防止异常干扰
    memset(stack_base__, 0, TOTAL_STACK_SIZE);

    // 修改守护页属性, 取消所有属性, 实现自爆功能
    if (mprotect(stack_base__, GUARD_SIZE, PROT_NONE) == -1) {
        stack_base.reset();
        return false;
    }

    auto stack_bottom = stack_base__ + GUARD_SIZE;  // 栈底
    char* stack_top = stack_bottom + STACK_SIZE;
    // 64为系统要求基地址要对齐16字节, 也就是低四位抹零 ULL表示无符号长整型
    stack_top = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(stack_top) & ~0xFULL);

    // 栈初始化
    // 先减去16字节, 防止离边界太近, 从上面溢出
    stack_top -= 16;

    // 1. 压入 DUMMY (RSP+24), 确保 16 字节对齐
    stack_top -= 8;
    *reinterpret_cast<uint64_t*>(stack_top) = 0ULL;

    // 2. 压入线程退出函数地址 (RSP+16)
    stack_top -= 8;
    *reinterpret_cast<void**>(stack_top) = reinterpret_cast<void*>(&thread_exit_func);

    // 3. 压入 fn::routine 的参数 (this 指针) (RSP+8)
    stack_top -= 8;
    *reinterpret_cast<void**>(stack_top) = this;

    // 4. 压入汇编垫片入口地址 (__mythread_start_shim) (RSP)
    stack_top -= 8;
    *reinterpret_cast<void**>(stack_top) = reinterpret_cast<void*>(&__mythread_start_shim);

    // 单独开辟, 和栈一块开辟容易相互干扰
    // --- 2. TLS/TCB 空间申请及初始化 ---
    void* ptr2 = mmap(nullptr, TLS_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // 申请失败, 提前终止
    if (ptr2 == MAP_FAILED) {
        stack_base.reset();  // 回滚栈分配
        return false;
    }

    // 生命周期托管至智能指针
    tls_base =
        std::shared_ptr<char>(static_cast<char*>(ptr2), [](char* p) { munmap(p, TLS_SIZE); });

    auto tls_block_base = tls_base.get();

    // 清零防止原先的旧数据干扰
    memset(tls_block_base, 0, TLS_SIZE);

    auto tcb_start_ptr =
        tls_block_base + TLS_SIZE - sizeof(struct tls_descriptor);  // 结构体放在高地址尾部

    // 地址对齐 64字节, 抹去 tcb_start_ptr 的64以下的低位
    tcb_start_ptr = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(tcb_start_ptr) & ~63ULL);

    // 准备对基地址引导结构进行初始化
    auto header = reinterpret_cast<struct tls_descriptor*>(tcb_start_ptr);
    header->tcb = tcb_start_ptr;
    header->dtv = nullptr;  // dtv 内核自己初始化

    // --- 3. clone3 系统调用 ---
    struct clone_args params;
    memset(&params, 0, sizeof(struct clone_args));

    params.flags = CLONE_VM |  // 与父进程共享虚拟内存（创建线程而非独立进程）
                   CLONE_FS |  // 与父进程共享文件系统信息（cwd、umask 等）
                   CLONE_FILES |           // 与父进程共享文件描述符表
                   CLONE_SIGHAND |         // 与父进程共享信号处理器
                   CLONE_SYSVSEM |         // 与父进程共享 System V 信号量
                   CLONE_CHILD_CLEARTID |  // 子线程退出时清除其线程 ID
                   CLONE_THREAD |          // 告诉内核这是一个线程
                   CLONE_SETTLS |          // 设置子线程的线程局部存储（TLS）
                   //    CLONE_THREAD |  // 自动分配局部存储, 避免手动设置与内核
                   CLONE_CHILD_SETTID;  // 轻量级进程启动时将其ID写入child_tid

    params.stack = reinterpret_cast<uint64_t>(stack_top);
    params.stack_size = STACK_SIZE;
    params.parent_tid = reinterpret_cast<uint64_t>(&tid);
    params.child_tid = reinterpret_cast<uint64_t>(&child_tid);
    params.tls = reinterpret_cast<uint64_t>(tcb_start_ptr);  // 逆向偏移寻址
    params.exit_signal = 0;

    auto ret = syscall(SYS_clone3, &params, sizeof(struct clone_args));

    if (ret > 0) {
        // 返回轻量级进程 ID
        tid = ret;
        // 确保内核构造成功后应用层再返回
        // 注意: 在实际 glibc 实现中, child_tid 被置 0 后会有一个 FUTEX_WAKE, 这里的
        // sched_yield() 是一个简化的 busy-wait
        while (child_tid == 0) sched_yield();
    }

    if (ret < 0) {
        // 调用失败, 致命错误
        stack_base.reset();
        tls_base.reset();
        // 打印错误信息
        fprintf(stderr, "Error: clone3 failed with errno %d\n", errno);
        return false;
    }

    return true;
}

template <typename Result_t>
typename myThread<Result_t>::result_ptr_t myThread<Result_t>::join() {
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
            // 事件不再有种下的必要
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

// ===================================================================
// 十、myThread<void> 特化实现
// (由于委托模式，这里的生命周期问题通过 myThread<thread_void_result> 解决)
// ===================================================================
template <typename Func, typename... Args>
typename myThread<void>::managed_self_t myThread<void>::create(Func&& func, Args&&... args) {
    // 1. 包装用户任务：将 void() 任务转换为 thread_void_result() 任务
    std::function<void()> bound_task =
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...);

    // 核心：包装后的任务返回 thread_void_result 占位符
    std::function<thread_void_result()> wrapped_func = [bound_task]() -> thread_void_result {
        bound_task();
        return thread_void_result{};
    };

    // 2. 委托给 delegate_type (myThread<thread_void_result>) 的 create 函数，它会启动线程
    typedef myThread<thread_void_result> delegate_type;
    typedef std::shared_ptr<delegate_type> delegate_ptr_t;

    delegate_ptr_t delegate_shared_ptr = delegate_type::create(std::move(wrapped_func));

    if (!delegate_shared_ptr) {
        return managed_self_t();
    }

    // 3. 使用 C++11 aliasing constructor 强制转换 shared_ptr
    // 返回一个指向 myThread<void> 的 shared_ptr，但其控制块（引用计数）
    // 属于 delegate_shared_ptr，且其指针指向 delegate_shared_ptr.get() 的地址。
    // 由于 myThread<void> 和 myThread<thread_void_result> 都继承自 thread_common_state，
    // 这种转换对于访问基类成员是安全的。
    return managed_self_t(delegate_shared_ptr,
                          reinterpret_cast<myThread<void>*>(delegate_shared_ptr.get()));
}

void myThread<void>::join() {
    // 直接在共享内存地址上执行 FUTEX 等待。
    // child_tid 成员通过继承 thread_common_state 保证了正确性。
    int expected = __atomic_load_n(&child_tid, __ATOMIC_SEQ_CST);
    if (expected == 0) return;
    while (true) {
        // child_tid 访问来自基类
        long ret = syscall(SYS_futex, &child_tid, FUTEX_WAIT, expected, nullptr, nullptr, 0);
        if (ret == 0 || errno == EAGAIN) return;
        if (errno == EINTR) continue;
        return;
    }
}
