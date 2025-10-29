#include "my_thread.h"

#include <errno.h>
#include <linux/futex.h>
#include <signal.h>
#include <sys/syscall.h>

#include <cstring>
#include <utility>

#include "my_thread.h"

template <typename Result_t>
int myThread<Result_t>::fn::routine(void* arg) {
    auto* self = static_cast<self_t*>(arg);
    self->result = self->task();
    return 0;
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

    // 栈顶 = 守护页后 + 栈
    void* stack_top = base + GUARD_SIZE + STACK_SIZE;

    // TLS 描述符
    struct user_desc tls_desc {};
    tls_desc.entry_number = -1;
    tls_desc.base_addr = reinterpret_cast<unsigned long>(stack_top);
    tls_desc.limit = (TLS_SIZE / 4096) - 1;
    tls_desc.seg_32bit = 1;
    tls_desc.contents = 0;
    tls_desc.read_exec_only = 0;
    tls_desc.limit_in_pages = 1;
    tls_desc.seg_not_present = 0;
    tls_desc.useable = 1;

    // clone
    const int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |
                      CLONE_SYSVSEM | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | CLONE_SETTLS |
                      SIGCHLD;

    int ret = clone(&fn::routine, stack_top, flags, this, &tid, &tls_desc, &child_tid);

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

    // 为零, 说明已经结束退出
    if (expected == 0) return result;

    while (true) {
        // 使用系统调用 SYS_futex, 为内核种下如下事件
        // 当child_tid与预期值expected不相同时, 触发事件
        // 处理逻辑为将线程从等待队列移到运行队列, 否则继续待在等待队列
        long ret = syscall(SYS_futex, &child_tid, FUTEX_WAIT, expected, nullptr, nullptr, 0);

        if (ret == 0) {
            // 事件种下成功, 等待一定时间后, 被唤醒, 获取到返回值, 为零
            break;
        } else {
            if (errno == EAGAIN) {
                // 事件种下失败, 但失败原因是内核在尝试种下事件时, 发现child_tid和expected 已经不同,
                // 事件不在有种下的必要
                return result;
            } else if (errno == EINTR) {
                // 因为信号的中断导致种下失败, 继续重试种事件
                continue;
            }
            // 其它恶性错误, 再怎么重试, 都种不上, 直接返回
            return result;
        }
    }

    return result;
}