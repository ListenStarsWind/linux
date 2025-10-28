#include "my_thread.h"

#include <signal.h>

#include <memory>

int __start_routine(void* arg) {
    auto t = static_cast<my_thread_t*>(arg);
    t->result = t->start(t->arg);
    return 0;
}

int my_thread_create(my_thread_t* newthread, void* (*start_routine)(void*), void* arg) {
    // 参数合法性检查
    if (!newthread || !start_routine) {
        return -1;
    }

    const size_t STACK_SIZE = 1 << 21;  // 线程子栈大小是 2 * 2^20 字节, 即 2MB
    const size_t GUARD_SIZE = 1 << 12;  // 守护页, 4KB大小
    const size_t TLS_SIZE = 1 << 12;    // 局部存储段大小, 4KB

    // 将要创建的轻量级进程相关设置
    const int flags = CLONE_VM |              // 共享内存映射
                      CLONE_FS |              // 共享文件系统信息（cwd, root）
                      CLONE_FILES |           // 共享文件描述符表
                      CLONE_SIGHAND |         // 共享信号处理程序
                      CLONE_THREAD |          // 加入同一线程组（TGID 相同）
                      CLONE_SYSVSEM |         // 共享 System V 信号量调整
                      CLONE_PARENT_SETTID |   // 父进程写入子 tid
                      CLONE_CHILD_CLEARTID |  // 子退出时清零 child_tid + futex 唤醒
                      CLONE_SETTLS |          // 设置 TLS（配合 user_desc）
                      SIGCHLD;                // 退出时发 SIGCHLD

    // 向系统请求一份空间用来存放线程子栈, 局部存储空间
    auto mem =
        std::shared_ptr<void>(mmap(nullptr, STACK_SIZE + GUARD_SIZE + TLS_SIZE,
                                   PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0),
                              [](void* _ptr) { munmap(_ptr, STACK_SIZE + GUARD_SIZE + TLS_SIZE); });

    // 失败返回 宏 MAP_FAILED
    if (mem.get() == MAP_FAILED) return -1;

    auto addr = static_cast<char*>(mem.get());  // 守护页起始地址
    auto stack_top = addr + STACK_SIZE +
                     GUARD_SIZE;  // 栈顶地址, 栈从高地址往低地址增长, 亦是局部存储空间的起始地址

    // 调整守护页权限属性 不授予 任何权限
    if (mprotect(addr, GUARD_SIZE, PROT_NONE) == -1) return -1;

    // 为内核描述局部存储段
    struct user_desc tls_desc;
    memset(&tls_desc, 0, sizeof(tls_desc));  // 描述符内部初始化
    tls_desc.entry_number = static_cast<unsigned int>(-1);
    tls_desc.base_addr = reinterpret_cast<unsigned long>(stack_top);
    tls_desc.limit = static_cast<unsigned int>((TLS_SIZE / 4096) - 1);
    tls_desc.seg_32bit = 1;
    tls_desc.contents = 0;
    tls_desc.read_exec_only = 0;
    tls_desc.limit_in_pages = 1;
    tls_desc.seg_not_present = 0;
    tls_desc.useable = 1;

    // clone 轻量级子进程作为线程执行流
    int ret = clone(__start_routine, stack_top, flags, newthread, &newthread->tid, &tls_desc,
                    &newthread->child_tid);

    if(ret == -1) return -1;

    // 等待内核异步写入
    while(newthread->child_tid == 0)
    {
        sched_yield();  // 放弃时间片节约资源
    }

    return 0;
}
