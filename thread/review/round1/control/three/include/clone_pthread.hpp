#include <linux/futex.h>  //  FUTEX_WAIT 等宏
#include <sched.h>        // clone
#include <sys/mman.h>     // mmap
#include <sys/syscall.h>  // 定义 SYS_* 常量
#include <unistd.h>       // syscall

#include <cerrno>       // errno
#include <cstdio>       // fflush
#include <cstring>      // memset
#include <format>       // std::format
#include <functional>   // 函数对象
#include <iostream>     // std::cerr
#include <memory>       // 智能指针
#include <type_traits>  // 类型萃取

#if defined(__GLIBCXX__)
#include <cxxabi.h>  // __cxxabiv1::__cxa_finalize(nullptr);
#endif

enum {
    STACK_SIZE = 1 << 21,  // 2MB 栈
    GUARD_SIZE = 1 << 12,  // 4KB 守护页
    MMAP_SIZE = STACK_SIZE + GUARD_SIZE
};

class myThreadBase {
   protected:
    using mem_ptr_t = std::shared_ptr<char>;
    using lwp_t = ::pid_t;  // 去全局域里找pid
    using task_t = std::function<void()>;

   public:
    myThreadBase() = default;
    ~myThreadBase() = default;

    bool __start_thread() {
        // 栈及守护页空间申请
        void* ptr1 =
            mmap(nullptr, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        // 申请失败. 提前结束
        if (ptr1 == MAP_FAILED) return false;

        // 生命周期托管至智能指针
        _stack =
            std::shared_ptr<char>(static_cast<char*>(ptr1), [](char* p) { munmap(p, MMAP_SIZE); });

        auto stack_base = _stack.get();

        // 为整个空间进行清零初始化, 防止异常干扰
        memset(stack_base, 0, MMAP_SIZE);

        // 修改守护页属性, 取消所有属性, 实现自爆功能
        if (mprotect(stack_base, GUARD_SIZE, PROT_NONE) == -1) {
            _stack.reset();
            return false;
        }

        // 栈由高地址向低地址增长, 也就是说栈区的最高地址是栈底, 最低地址是栈顶
        // 我们要把初始地址--栈底--最高栈区地址, 交给内核
        auto stack_top = stack_base + GUARD_SIZE;
        auto stack_bottom = stack_top + STACK_SIZE;

        // 以栈底地址 为 起始地址 的那片区域实际上已经超出了栈区, 所以需要把栈底地址变小
        stack_bottom -= 16;

        // 64为系统要求基地址要对齐16字节, 也就是低四位抹零 ULL表示无符号长整型
        // 抹零也能让栈底地址变小, 但之前减是为了防止特殊情况: 栈顶地址本来就是对齐的
        stack_bottom = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(stack_bottom) & ~15ULL);

        // 为轻量级进程设置属性, 模拟线程行为
        int flags = CLONE_VM |              // 与主线程共享虚拟内存
                    CLONE_FS |              // 与主线程共享文件系统信息
                    CLONE_FILES |           // 与主线程共享文件描述符表
                    CLONE_SIGHAND |         // 与主线程共享信号处理器
                    CLONE_SYSVSEM |         // 与主线程共享 System V 信号量
                    CLONE_CHILD_CLEARTID |  // 子线程退出时清除其线程 ID
                    CLONE_THREAD |          // 把子线程放进主线程所在线程组
                    CLONE_CHILD_SETTID;     // 把子线程ID写入_tid

        int ret = ::clone(&myThreadBase::thread_entry,  // clone出的轻量级进程函数入口
                          stack_bottom,                 // 栈底地址
                          flags,                        // 行为描述符
                          this,                         // 入口函数参数
                          &_tid,                        // 新轻量级进程ID回写
                          &_tid  // 轻量级进程退出时回写重新置为0
        );

        if (ret > 0) {
            // 确保内核构造成功后应用层再返回, 自旋锁
            while (_tid == 0) sched_yield();
        } else {
            // 调用出错
            perror("clone failed: ");
            _stack.reset();
            return false;
        }

        return true;
    };

    bool __join() {
        // 原子性获取 child_tid
        int expected = __atomic_load_n(&_tid, __ATOMIC_SEQ_CST);

        // 为零, 说明子线程已经结束 直接汇合
        if (expected == 0) return true;

        while (true) {
            // 使用clone 添加 CLONE_CHILD_CLEARTID时, 为内核种下如下事件
            // 当child_tid与预期值expected不相同时, 触发事件
            // 处理逻辑为将线程从等待队列移到运行队列, 否则继续待在等待队列
            long ret = syscall(SYS_futex, &_tid, FUTEX_WAIT, expected, static_cast<void*>(nullptr),
                               nullptr, 0);

            // syscall 会返回 0 或者 -1 , 分别代表调用成功和失败
            if (ret == 0) return true;  // 事件种下成功, 等待一定时间后, 被唤醒, 获取到返回值, 为零

            // 接下来是针对失败(ret == -1)原因的细分
            if (errno == EAGAIN) {
                // 事件种下失败, 但失败原因是内核在尝试种下事件时, 发现child_tid和expected 已经不同,
                // 事件不再有种下的必要
                return true;
            } else if (errno == EINTR) {
                // 因为信号的中断导致种下失败, 继续重试种事件
                continue;
            } else {
                // 其它恶性错误, 再怎么重试, 都种不上
                return false;
            }
        }

        // 理论上也到不了这里, 到了肯定有错
        return false;
    };

    void __setTask(task_t&& task) {
        _task = task;
    }

   private:
    void thread_task() {
        if (_task != nullptr) _task();
    }

    static int thread_entry(void* arg) {
        auto me = static_cast<myThreadBase*>(arg);

        // 执行任务
        try {
            me->thread_task();
        } catch (...) {
            std::cerr << std::format("[Worker thread {} caught an unknown exception]",
                                     syscall(SYS_gettid))
                      << std::endl;
        }

        // 在语言层上进行清理
#if defined(__GLIBCXX__)
        __cxxabiv1::__cxa_finalize(nullptr);
#else
        // Clang + libc++ 下没有通用接口
        // 用操作系统硬性解散内存映射
        // 为保险起见, 任务中涉及持久化的对象应该主动刷新
        // 任务中也不要有全局对象, 依靠生命周期主动析构
#endif

        // 在IO流上清理
        fflush(nullptr);
        std::cout.flush();
        std::cerr.flush();

        _exit(0);

        // 实际上不会来到这里
        // 这里写是为了让语法分析器不报警
        return 0;
    }

   private:
    mem_ptr_t _stack{};
    lwp_t _tid{};
    task_t _task{};
};

template <typename result_t>
class myThread : public myThreadBase {
    // 编译时模版类型检查
    static_assert(!std::is_void<result_t>::value, "这个模版不能走void类型特化");

   public:
    using mem_ptr_t = myThreadBase::mem_ptr_t;
    using lwp_t = myThreadBase::lwp_t;
    using task_t = myThreadBase::task_t;

    using self_t = myThread<result_t>;
    using self_ptr_t = std::shared_ptr<self_t>;      // 托管自身的智能指针
    using result_ptr_t = std::shared_ptr<result_t>;  // 托管结果的智能指针

   public:
    // 工厂模式实例化对象
    template <typename Func, typename... Args>
    static self_ptr_t create(Func&& func, Args&&... args) {
        // 构造函数负责构造任务
        auto ptr = self_ptr_t(new self_t(func, args...));
        // myThreadBase::start_thread 负责启动
        if (ptr->myThreadBase::__start_thread())
            return ptr;
        else
            return {};
    };

    // 基于事件驱动的汇合
    result_ptr_t join() {
        if (this->myThreadBase::__join())
            return _result;
        else
            return result_ptr_t{};
    }

    myThread(const myThread&) = delete;
    myThread& operator=(const myThread&) = delete;
    myThread(myThread&&) = default;
    myThread& operator=(myThread&&) = default;
    ~myThread() = default;

   private:
    template <typename Func, typename... Args>
    explicit myThread(Func&& func, Args&&... args) {
        this->myThreadBase::__setTask(
            [this, func = std::forward<Func>(func), ... args = std::forward<Args>(args)]() mutable {
                _result = result_ptr_t(new result_t(func(args...)));
            });
    };

   private:
    result_ptr_t _result;
};

// 线程返回值占位符，用于 void 特化，以保持内存布局一致
struct thread_void_result {};

template <>
class myThread<void> : public myThreadBase {
    public:
    using mem_ptr_t = myThreadBase::mem_ptr_t;
    using lwp_t = myThreadBase::lwp_t;
    using task_t = myThreadBase::task_t;

    using self_t = myThread<void>;
    using self_ptr_t = std::shared_ptr<self_t>;  // 托管自身的智能指针

    using result_t = struct thread_void_result;
    using result_ptr_t = std::shared_ptr<result_t>;  // 托管结果的智能指针(占位)
    using delegate_t = myThread<result_t>;
    using delegate_ptr_t = std::shared_ptr<delegate_t>;


public:    
    myThread(const myThread&) = delete;
    myThread& operator=(const myThread&) = delete;
    myThread(myThread&&) = default;
    myThread& operator=(myThread&&) = default;
    ~myThread() = default;

    template <typename Func, typename... Args>
    static self_ptr_t create(Func&& func, Args&&... args) {
        // 包装任务函数, 让它看上去就像平常的, 会返回值的一样
        auto user_task = [func = std::forward<Func>(func),
                          ... args = std::forward<Args>(args)]() mutable { return func(args...); };

        auto delegate_task = [user_task = std::move(user_task)]() mutable {
            user_task();
            return result_t{};
        };

        // 委托至delegate_ptr_t对象进行实际构造
        auto delegate = delegate_t::create(std::move(delegate_task));

        if (!delegate) return {};

        return delegateToSelf(delegate);
    };

    bool join() {
        return this->myThreadBase::__join();
    };

   private:
    static self_ptr_t delegateToSelf(const delegate_ptr_t& delegate) {
        return self_ptr_t{delegate, reinterpret_cast<self_t*>(delegate.get())};
    }

   private:
    // 尽管实际上不会用到它, 但可以提供一个用作临时构造
    myThread() = default;

   private:
    // 实际不会被使用, 它仅用来占位, 保持所有实例化后类内存布局的绝对一致
    result_ptr_t _result;
};