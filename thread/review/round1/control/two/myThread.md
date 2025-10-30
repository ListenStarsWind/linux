# myThread

本文档的作用是对同项目下使用系统接口模拟实现 linux 原生线程库 pthread 部分功能的这个项目中涉及到的要点进行文档说明.

事件的起因是我在本存储库中次根目录中的 thread.md 文档进行复习时发现, 其中说了线程库底层调用的接口 `clone`, 于是我想通过该项目来进一步加深我对线程的理解.

![image-20251029194911811](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251029194911811.png)

在最开始, 其实我只是打算随便用 C式风格来模拟实现一下, 但在实现的过程中, 为了防止申请的空间能在调用出错时更好的自动释放, 我引入了智能指针, 但智能指针本身的生命周期也需要维护, 总不能把它扔给用户吧, 所以最后干脆以 C++ 为主要风格实现了.

首先对于头文件中的宏 `_GNU_SOURCE`,看起来很突兀, 实际上它与之后我们用到的局部存储描述符所用到的头文件有关, 不带他, 局部存储描述符类型就用不了, 至于为什么还要检测一下存在不存在, 是因为项目是用 CMake 构建的, 构建脚本里含有这个宏, 但等到我查出来的时候, 源代码上已经定义了这个宏, 所以我就在外面加了层判断, 防止重复包含, 导致 clangd 报警让我心烦. 值得一题的是, 这个宏必须要在与之相对应头文件的的上面, 可能是头文件有了这个宏会条件编译吧.

首先我们看这个构造函数

```cpp
    template <typename Func, typename... Args>
    explicit myThread(Func&& func, Args&&... args);
```

最开始我们并不是像现在这样, 把构造函数设置为私有, 然后用一个静态函数建造出来, (Grok 说现在我们这种构造方法也是一种工厂模式), 而是直接用构造函数, 但后来我自己否定了这个方案, 因为构造函数的核心职责是从系统中申请一段空间, 并 clone 出一个轻量级进程供我们在外部包装, 作为子线程的栈, 局部存储来使用, 尽管在函数的实现上我们可以通过 RAII 技术来实现差错情况下更方便的空间返还. 但用户并不好清楚, 这到底有没有构造成功, 本来我的想法是再写一个检测函数, 专门判断有没有构造成功, 但 Grok 给我提了一个更好的方案, 那就是现在我们使用的这个方案, 正如 Grok 所说那样, 检测函数的这种方法用户可能忘记调用或者根本不知道要调用, 另外我也不太喜欢抛异常的另一种处理方法, 所以就用了这种工厂模式方案. 其中的 explicit 作用是禁止隐式转换, 放在构造函数这里就是禁止使用基于隐式类型转换的构造方式: 这种构造方案容易造成歧义.

```cpp
auto t = myThread([]{ return 42; });  // 隐式构造！

// 危险！允许以下写法：
myThread t = []{ return 42; };  // 编译通过！
// 等价于：myThread t = myThread( lambda );
```

 就是看起来就像是赋值一样, 加了 explicit 后就只能允许 `new Type()` 和 `Type{}`的形式. 为了适配各种各样的函数对象和参数, 此处使用了模版并顺便用了万能引用, 它的定义内部也用了完美转发.

```cpp
    // 基于事件驱动的汇合
    Result_t join();
```

对于 `join`, 它的任务和 `pthread_join`类型, 就是阻塞等待副线程退出并会和, 并且会把任务的返回值也一并返回. 为了知道返回值是什么类型, 整个类就成了模版类. 其内部实现的思路正如注释所言: 他会在内核中种下一个事件, 事件触发后, 主线程会被系统重新移回运行队列, 从而继续运行, 实现唤醒的效果. "事件"这个概念在 linux 阶段并没有说, 我这里实际上是用 Qt 中的事件概念反哺系统中的事件概念.

```cpp
    std::shared_ptr<void> mem;  // 栈 + 守护页 + TLS
    pid_t tid{};
    pid_t child_tid{};
    Result_t result{};
    std::function<Result_t()> task;

    static constexpr size_t STACK_SIZE = 1 << 21;  // 2MB
    static constexpr size_t GUARD_SIZE = 1 << 12;  // 4KB
    static constexpr size_t TLS_SIZE = 1 << 12;    // 4KB
    static constexpr size_t TOTAL_SIZE = STACK_SIZE + GUARD_SIZE + TLS_SIZE;
```

tid 就是 clone 出的新轻量级进程 ID, 对于 child_tid, 其实我对它这个名字不太理解, 它的指针将会被设置进 clone 中, 等到线程执行完任务退出, 就会把它置成0, join 就是把它作为事件触发的参考量的. 下面的大小, 我们都是通过移位来实现的, 因为计算机对移位更擅长, 而且这种移位比较好对其页框, 稍后我们会在 mmap 那里说, 它的大小不是随便定的, 最好是要对齐页框, 标准库也喜欢用这种移位的风格.

```cpp
    class fn {
       public:
        static int routine(void* arg);
    };
```

因为用了模版函数, 模版函数的函数指针无法设置进 C式的系统接口, 所以特别写了一下, 作为轻量级进程实际的入口函数.

下面是定义中的要点

```cpp
        mmap(nullptr, TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

#include <sys/mman.h>

void* mmap(
    void*   addr,       // 建议映射地址（通常 nullptr）
    size_t  length,     // 映射字节数
    int     prot,       // 内存保护（读/写/执行）
    int     flags,      // 映射标志（共享/私有/匿名等）
    int     fd,         // 文件描述符（-1 表示匿名）
    off_t   offset      // 文件偏移（匿名时 0）
);
```

首先是系统调用 mmap, mmap 的主要作用是从内核那里申请一段空间, 然后再把文件, 主要是设备文件映射到这出空间里, 从而实现对设备的控制.

但它其实还可以用来进行匿名空间的申请, 所谓匿名空间就是光申请一份空间, 不进行文件映射, 我们函数实现中使用的就是这种. `TOTAL_SIZE`是我们申请的控件大小, 分为两个部分, 正好对应上图中的那个 线程子栈, 局部存储, 另外, 线程栈底外还专门设置了一个保护页. `PROT_READ | PROT_WRITE`自然就是可读可写, `MAP_PRIVATE`就是空间私有, 子线程在其中的修改不会对主线程产生影响, `MAP_ANONYMOUS`表示匿名映射. 剩下两个不用看, 一个是文件描述符, 一个是文件偏移量, 那文件偏移量其实描述的就是设备寄存器的基地地址, 他把寄存器加载进来, 通过控制寄存器控制设备.

至于那个保护页, 其实就是什么权限都不给, 如果内存溢出到这里, 因为什么权限都没有, 什么就相当于进程因为读写不该读写的部分而自爆, 起到保护内存中其它数据的作用, 稍后我们也会看到, 有相应的接口专门改变那块的权限.

```cpp
    // 守护页
    if (mprotect(base, GUARD_SIZE, PROT_NONE) == -1) {
        mem.reset();
        return;
    }
```

这里就是调整它的权限了, 因为栈是从高地址往低地址那里增长的, 所以即使溢出也是往栈底更低的地址溢出的, 所以守护页的区域就是基底地址再加上它自身大小的这个范围. 如果设置失败, 那我们就提前释放, 构造失败.

```cpp
    // 栈顶 = 守护页后 + 栈
    void* stack_top = base + GUARD_SIZE + STACK_SIZE;

struct user_desc tls_desc {};
tls_desc.entry_number = -1;           // 让内核自动分配 GDT 条目
tls_desc.base_addr = (unsigned long)stack_top;  // TLS 起始地址
tls_desc.limit = (TLS_SIZE / 4096) - 1; // 页数 - 1（4KB 对齐）
tls_desc.seg_32bit = 1;               // 32 位段（兼容 x86）
tls_desc.contents = 0;                // 数据段
tls_desc.read_exec_only = 0;          // 可读写
tls_desc.limit_in_pages = 1;          // limit 单位为页
tls_desc.seg_not_present = 0;         // 段存在
tls_desc.useable = 1;                 // 可用
```

这里的主体就是局部存储描述符, 该类型是内核中的一个用处还是比较广的类型, 所以为了适配更多功能, 里面成员比较多, 这也就造成了, 有时候对于某些功能来说, 比如充当局部存储描述符的这个功能, 其中的一些成员都是固定的, 甚至必须是固定的.

那什么 GDT 条目咱们也不懂, 那就给内核自己判断吧, 注意这里是栈顶地址, 因为栈是从高往低的, 所以起始地址当然是栈顶, 这一点, 不能弄错. limit 描述的事最大地址偏移量, 你可以这么认为, 你可以把栈顶的地址看做零, 那既然是从零开始的, 自然是要减一个一的, 这跟数组十个元素最大下标是九是相同的道理, 它有两种单位, 一个是字节, 另一种是页框., seg_32bit 表示 段的地址宽度, 尽管这里实际上是 64位, 局部存储器也是64位, 但实际上, 对于 x86_64 来说, 内核和内存实际上使用的还是32位.  contents 表示段中的数据类型, 0 表示数据, 1 表示代码, 局部存储当然存的是运行过程中的热数据, read_exec_only 表示读写权限, 0 表示可读可写, 1 表示只读或只写, limit_in_pages 表示单位, 1 是页框(4KB), 0 是字节, seg_not_present 表示段是否在内存中, 0 表示该空间必须在一开始就与实际的物理内存建立联系, 1 表示可以允许仅仅在逻辑上有这个虚拟内存, 等到真的要用的时候, 在建立映射关系, 对于局部存储来说, 那就必须要用 0, 否则保证不了性能, 本来就是热数据, 我急着要的时候, 你说先别急, 让我映射一下, 这样当然是不可行的, useable 表示段是否可用, 这可能是给其他功能配置的成员, 1 表示可用, 0 表示不可用, 对于局部存储来说, 当然是可用

```cpp
    const int flags = CLONE_VM |              // 1. 共享内存空间
                      CLONE_FS |              // 2. 共享文件系统信息（cwd, root）
                      CLONE_FILES |           // 3. 共享文件描述符表
                      CLONE_SIGHAND |         // 4. 共享信号处理程序
                      CLONE_THREAD |          // 5. 加入同一线程组（TGID 相同）
                      CLONE_SYSVSEM |         // 6. 共享 System V 信号量调整
                      CLONE_PARENT_SETTID |   // 7. 父进程写入子 tid
                      CLONE_CHILD_CLEARTID |  // 8. 子退出时清零 child_tid + futex 唤醒
                      CLONE_SETTLS |          // 9. 设置 TLS（配合 user_desc）
                      SIGCHLD;                // 10. 退出时发 SIGCHLD 信号

    int ret = clone(&fn::routine, stack_top, flags, this, &tid, &tls_desc, &child_tid);

    if (ret == -1) {
        mem.reset();
        return;
    }
```

此处flags, 就是给轻量级进程设置各种各样的属性, 让它和线程表现出相同的行为. 对于 clone 来说, routine 是入口函数, stack_top 是栈顶, flags 是属性, this 是函数参数, tid 是轻量级进程 ID, tls_desc 是局部存储描述符, child_tid 标记轻量级线程的始与终, 轻量级进程创建成功, 会把它变成 tid, 轻量级进程退出, 会把他置为0. 同样是出错, 提前释放.

```cpp
    // 确保内核异步写入后, 即真正完成轻量级进程创建后, 应用层再完成创建
    while (child_tid == 0) sched_yield();
```

轻量级进程的创建是交给系统的, 对于我们应用层来说, 这是一个异步的过程, 为了确保轻量级进程创建成功再返回, 我们会检测 child_tid , 是零的话, 就主动放弃时间片, 然后下一次轮询到自己后再查询, 这里的循环主要是确保一种极端情况, 就是你放弃时间片后, 下一个轮询还是你, 这种场景下不会出错.

```cpp
template <typename Result_t>
int myThread<Result_t>::fn::routine(void* arg) {
    auto* self = static_cast<self_t*>(arg);
    self->result = self->task();
    return 0;
}
```

这没什么好说的, 我们之前已经在构造函数里把任务 bind 成一个无参的函数对象了, 直接调用即可, 然后记录返回值.

```cpp
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
```

这里的主要思路其实很简单, 那就是 首先原子性判断 child_tid, 看看还是不是零, 是零, 直接返回, 不是零, 那就为内核设置对应的事件, 当这是建立在一切顺利的情况下的, 实际上, 还存在事件创建失败的场景, 所以外面的循环就是确保在重试创建事件还有意义的情况下, 不断的进行重试.  我曾打算在其中在增加 mem 自动释放的逻辑的, 但经过查询, 我发现有一些风险, 所以还是给析构函数释放吧.

后面我想到模版不能分离编译, 所以后面又把他们合起来了.

