# Inter-Process Communication

## **Introduction** 

在之前的学习过程中，我们体会到了进程的独立性，但是进程之前完全独立也不行，它们之间总有一些通信需求。我们现在只讲进程的两种通信方式：管道和共享内存。消息队列太老了，所以当前我们几乎用不着了解，信号量会在线程部分说。

进程通信有什么目的呢？

- 发送数据，比如，某个变量，某个字符串。可能是某个进程专门负责收集数据，但不处理数据，所以它就要把这些数据传输给其它进程。
- 发送命令，之前我们自已对进程发送了一些命令，就是`kill -l`中的那些。
- 进行某种目的的进程协同工作。
- 通知，比如现在某些进程还排不上用场，所以我们先把它停止，等到时机到了，再唤醒休眠进程。

······

我们知道进程本身具有一定独立性，所以为了让它们进行数据交互，就必须要借助于第三方独立机构，就像某两个国家有矛盾，需要来个独立的，实力被冲突两方认可的第三个国家调停。这个机构谁来提供呢？系统里面最大的不就是系统吗？系统会为需要通信的两个进程提供一份公共区域，这个公共区域的种种操作都由系统来进行管理，这样才能保证是第三方独立的，进程需要对该空间进行的操作，比如写入，都必须通过系统来进行，系统认为操作合理，才会依照进程需求对公共空间进行操作。这公共空间从哪里找呢？很明显，是系统从一个未被使用的空闲区域内存分配出的，别管到底是用那种方式通信的，实际上都是对该公共空间换种形式。

可能有通信要求的进程有很多，所以可能会有很多公共空间，很明显，系统会对这些公共区域进行抽象化描述，随后再用某些数据结构把它们组织起来。

Linux最开始并没有进程通信能力，可能是把精力花在如何实现进程独立上了，但人们使用Linux一段时间后，感觉到没有进程通信很不方便，再加上Linux本身就是开源的，所以就有很多人设计了各种各样的进程通信方案，但由于方案太多了，所以各个方案的适配性就不太好，很显然，这不利于Linux的发展，所以最终人们从众多方案中挑选出了几个出众的方案，把它们规定为行业标准，不管这是什么版本的Linux，不管这Linux跑在什么硬件上，都必须支持这些标准，这就使得Linux的发展规范化，标准化。

进程通信系统被规定隶属于文件系统，毕竟进程很擅长使用文件，并且它们身上也多多少少有文件系统的影子。最后被公认的两套标准，分别是`System V  `和`POSIX  `。

## Pipe

管道是类Unix系统中最古老的进程通信方式，不过因为足够简单，所以现在仍被广泛使用。在以前Linux基础指令的学习过程中，我们也稍微谈过管道。例如`who`指令用于显示当前登录到系统的用户信息。而`wc`用于文本统计，`-l`选项表示需要统计文本行数。它们合起来的功能相当于统计当前登录系统用户的个数。

```shell
[wind@starry-sky projects]$ who
wind     pts/0        2024-12-04 08:21 (112.26.31.132)
[wind@starry-sky projects]$ who | wc -l
1
[wind@starry-sky projects]$
```

管道的原理相对简单：由于文件系统的独立性，可以将文件作为进程间通信的媒介，一个进程向文件写入信息，另一个进程从文件中读取信息，从而形成一个单向的信息通道，这就是管道的基础概念。
然而，考虑到进程都运行在内存中，数据的来源和去向也在内存中，为了避免外设读写带来的性能开销，管道被设计为内存级文件。这种设计使管道存储在内存中而非磁盘上，跳过了外设的读写过程，从而显著提高了通信效率。

要理解管道，首先需要回顾进程和文件是如何关联的：
每个进程都有一个对应的进程控制块（PCB），其中包含一个指向 `files_struct` 的指针。`files_struct` 是进程的文件描述符表，内部包含一个指针数组，这些指针指向具体的 `struct file` 结构体。

当进程启动时，系统会默认为其打开三个文件流：标准输入（文件描述符 `0`）、标准输出（文件描述符 `1`）和标准错误（文件描述符 `2`）。这些文件流的指针会填充到 `files_struct` 的前几个位置，从而建立文件与进程的初始联系。

对于普通文件，其磁盘上有一个对应的 `inode`，用于存储文件的元信息（如大小、权限等）。而 `struct file` 中则包含一个指向操作方法的指针，这些方法提供了从磁盘读写数据的接口。
为了优化性能，`struct file` 中还设计了一种机制，可以在内存中缓存磁盘上的数据块，这就是内核缓冲区。内核缓冲区的内容如果被修改，会通过 I/O 子系统同步回磁盘。

管道作为一种内存级文件，其操作方式与普通文件类似。虽然没有存储在磁盘上的 `inode`，但在内存中仍保留了对应的 `inode`，因为管道本质上是文件，需要记录其属性信息。通过调整底层的操作方法集，管道直接依赖内存缓冲区实现数据的读写。这种设计在底层优化了文件操作的实现方式，同时对上层应用保持透明，既保留了文件接口的通用性，又显著提升了进程间通信的效率。

当父进程创建管道后，其文件描述符表会被子进程继承，因此子进程也能访问这个管道。与此同时，管道对应的 `struct file` 中的引用计数字段会自动增加，以反映新的引用关系。通过这种继承机制，父子进程便可以使用管道进行双向通信。

管道本质上是单向的，只能实现一个进程写，另一个进程读，这种单向性正是其设计简单的原因，因为无需处理双向通信的复杂性。那么，如何让一个进程负责写，另一个进程负责读呢？

实现的关键在于父进程阶段：

1. 父进程创建管道后，会同时以只读和只写的方式打开管道。
2. 在进程分流（通常通过 fork）之后，每个进程根据角色关闭不需要的端。
   - 写端进程关闭管道的读端，只保留写端。
   - 读端进程关闭管道的写端，只保留读端。

通过这种方式，管道在逻辑上被分为单向的通信通道，之后即可按照设计进行正常的读写操作。

![image-20241204101441428](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241204101441557.png)

需要注意的是，只读管道的 `struct file` 和只写管道的 `struct file` 都指向同一个内核缓冲区。换句话说，同一个文件即使以不同方式打开，其对应的 `struct file` 是独立的，但底层使用的内核缓冲区却是共享的。这样才可以保证管道的读写操作都是针对同一个缓冲区进行的，从而实现进程间通信。

要实现进程间的双向通信，使用管道其实很简单。只需创建两个管道，每个管道都是单向的，只不过方向相反。一个管道用于进程A向进程B发送数据，另一个管道则用于进程B向进程A发送数据。通过这种方式，两个进程就能实现双向通信。

这种管道没有名字，通信依赖于具有某种血缘关系的进程之间的文件描述符继承，因此我们称之为 匿名管道。它主要依赖于进程间的父子关系来传递数据，不能在独立的进程之间直接共享。除了匿名管道，还有 命名管道，但由于我们现在不考虑这一类型，因此暂时不讨论。

### Operation

下面我们将进行具体操作：实现子进程向管道写入数据，父进程从管道读取数据。

管道的创建和打开有专门的接口，虽然 `open` 是用于普通文件的系统调用，但对于管道来说，使用的是 `pipe` 接口。`pipe` 是一个系统调用，用于创建匿名管道，并返回两个文件描述符，一个用于读，另一个用于写。该接口在 man 2 手册中有详细描述。

![image-20241204105232888](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241204105233099.png)

`pipe` 系统调用有两个主要功能：一是创建管道，二是分别以只读和只写的方式打开管道。通过这个接口，用户可以创建一个匿名管道，并获得对应的读写文件描述符。`pipe` 的参数是一个大小为 2 的整型数组，调用成功后，读端文件描述符会存储在数组的第一个元素中，写端文件描述符会存储在第二个元素中。如果操作成功，`pipe` 返回 0；若操作失败，则返回 -1，并设置 `errno` 来指示错误类型。

```c
#include <unistd.h>

int pipe(int pipefd[2]);
```

- `pipefd[0]`：读端文件描述符，用于读取管道中的数据。
- `pipefd[1]`：写端文件描述符，用于向管道写入数据。

下面我们写一个`.cc`程序，`.cc`的意思就是它既使用C，也使用C++，因为系统是用C写的，所以其接口必然是C接口，所以我们要C++和C混用。

```c++
#include<iostream>
#include<cstring>   // C++里也有对应的C头文件，此处头文件原型为string.h
#include<unistd.h>
#include<cstdio>
#include<sys/types.h>
#include<sys/wait.h>


#define R 1 << 0
#define W 1 << 1

using namespace std;

#define BUFFER_SIZE 4096

// 父进程任务集
class parent_task
{
    public:
    // 父进程执行流
    void operator()(int arr[2])
    {
        close(arr[1]);
        const int& read_fd = arr[0];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 读取信息


        exit(0);
    }
};

// 子进程任务集
class child_task
{
    public:
    // 子进程执行流
    void operator()(int arr[2])
    {
        close(arr[0]);
        const int& write_fd = arr[1];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 发出信息
        // Here is the child process:

        exit(0);
    }
};

// 这是一个管道测试程序
// 其中子进程用于写，父进程用于读
int main()
{
    // 信道的建立
    int arr[2] = {0};
    int i = pipe(arr);
    if(i == -1)
    {
        perror("failed pipe");
        return 1;
    }

    pid_t id = fork();
    if(id < 0 )
    {
        perror("failed fork");
        return 2;
    }

    parent_task parent_process;
    child_task child_process;

    if(id == 0)
        child_process(arr);
    else
        parent_process(arr);

    return 0;
}
```

为了能将发送内容转换成字符串，我们需要了解一下`snprintf`，它是C语言的格式转换接口，能将其它格式的数据都转换成字符串。当然用C++的方式也是可以的。

![image-20241204142403282](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241204142403474.png)

`snprintf`的第一个参数是准备承接新字符串的缓冲区，第二个参数用于描述缓冲区的大小，以免缓冲区溢出，第三个参数就是指定格式的数据。

我们先不进行进程通信，而是把缓冲区的生成内容打印一下，检查是否有误。因为父进程现在还用不上，所以我们就让它阻塞等待子进程。

```cpp
// 父进程任务集
class parent_task
{
    public:
    // 父进程执行流
    void operator()(int arr[2], int id)
    {
        close(arr[1]);
        const int& read_fd = arr[0];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 读取信息

        pid_t i = waitpid(id, nullptr, 0);
        if(i != id)
        {
            // 等待失败(非本文重点，省略)

            exit(3);
        }
        exit(0);
    }
};

// 子进程任务集
class child_task
{
    public:
    // 子进程执行流
    void operator()(int arr[2])
    {
        close(arr[0]);
        const int& write_fd = arr[1];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 发出信息
        int count = 1;
        while(count)
        {
            buffer[0] = 0; // 在C/C++中字符串结尾为'\0'，此处暗示该缓冲区是字符串的载体
            pid_t id = getpid();
            snprintf(buffer, sizeof(buffer), "%s:: pid:%d count:%d", "Here is the child process", id, count);
            count++;
            cout<<buffer<<endl;
            sleep(1);
        }

        exit(0);
    }
};
```

```shell
[wind@starry-sky pipe]$ make
[wind@starry-sky pipe]$ ls
makefile  testPipe  testPipe.cc
[wind@starry-sky pipe]$ ./testPipe
Here is the child process:: pid:8284 count:1
Here is the child process:: pid:8284 count:2
Here is the child process:: pid:8284 count:3
Here is the child process:: pid:8284 count:4
Here is the child process:: pid:8284 count:5
Here is the child process:: pid:8284 count:6
Here is the child process:: pid:8284 count:7
Here is the child process:: pid:8284 count:8
Here is the child process:: pid:8284 count:9
Here is the child process:: pid:8284 count:10
Here is the child process:: pid:8284 count:11
Here is the child process:: pid:8284 count:12
Here is the child process:: pid:8284 count:13
Here is the child process:: pid:8284 count:14
^C
[wind@starry-sky pipe]$
```

之后就是读写了，管道就是文件，文件用什么读写，它就用什么读写。

```cpp
// 父进程任务集
class parent_task
{
    public:
    // 父进程执行流
    void operator()(int arr[2], int id)
    {
        close(arr[1]);
        const int& read_fd = arr[0];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 读取信息
        int loop_counter = 1;
        while(1)
        {
            buffer[0] = 0;
            ssize_t end = read(read_fd, buffer, sizeof(buffer) - 1);

            // 异常处理
            if(end < 0)
            {
                perror("failed read");
            }
            else
            {
                buffer[end] = 0;
                cout<<buffer<<"  "<<loop_counter<<endl;
            }

             loop_counter+=2;
        }
        pid_t i = waitpid(id, nullptr, 0);
        if(i != id)
        {
            // 等待失败

            exit(3);
        }
        exit(0);
    }
};

// 子进程任务集
class child_task
{
    public:
    // 子进程执行流
    void operator()(int arr[2])
    {
        close(arr[0]);
        const int& write_fd = arr[1];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 发出信息
        int count = 1;
        while(count)
        {
            // 生成一条信息
            buffer[0] = 0; // 在C/C++中字符串结尾为'\0'，此处暗示该缓冲区是字符串的载体
            pid_t id = getpid();
            snprintf(buffer, sizeof(buffer), "%s:: pid:%d count:%d", "Here is the child process", id, count);
            count++;

            // 将信息写到管道中
            write(write_fd, buffer, strlen(buffer));

            // cout<<buffer<<endl;  检查
            sleep(1);
        }

        exit(0);
    }
};
```

```shell
[wind@starry-sky pipe]$ make clean ; make
[wind@starry-sky pipe]$ ./testPipe
Here is the child process:: pid:10375 count:1  1
Here is the child process:: pid:10375 count:2  3
Here is the child process:: pid:10375 count:3  5
Here is the child process:: pid:10375 count:4  7
Here is the child process:: pid:10375 count:5  9
Here is the child process:: pid:10375 count:6  11
Here is the child process:: pid:10375 count:7  13
Here is the child process:: pid:10375 count:8  15
Here is the child process:: pid:10375 count:9  17
Here is the child process:: pid:10375 count:10  19
Here is the child process:: pid:10375 count:11  21
Here is the child process:: pid:10375 count:12  23
Here is the child process:: pid:10375 count:13  25
^C
[wind@starry-sky pipe]$
```

注意，这里父进程是没有`sleep`的，但循环次数与子进程同步，这体现着管道的管道的协作性：管道的读取端会阻塞，直到写入端有新的数据可以读取为止。

尽管管道是单向通信的，但在实际应用中，仍然有很多需要解决的问题。例如，如果子进程每隔一秒写入一条消息，而父进程则紧跟着读取这些消息，问题就可能出现在两者的同步上。假设父进程没有适当的延时（如`sleep`），并且在读取到一条消息后立即进行下一轮循环，它很可能会在没有数据可读的情况下重复读取管道，导致无效的读取操作。

比如，在上面的代码中，子进程写了一条信息，就休眠去了，父进程读过信息之后，不会休眠，所以就会立刻来到下一轮循环，等到其再次尝试读取时，如果真的读了，就会把管道中的其它信息读出来，在此种场景下，那取决于管道是怎么初始化的，所以不能真的读管道，于是父进程就会阻塞在`read`，直到子进程又发了一条新信息，父进程才会停止阻塞，再次运行。

-----------------

下面说说管道的四种情况。

第一种，读写端正常，正常的意思就是它的读写端没有被关闭，比如在上面的代码中，就是父进程没有关闭`read_fd`，子进程没有关闭`write_fd`，如果管道为空，为空就是能读的都读完了，读端阻塞，就是前面的那个例子。

第二种，读写端正常，如果管道满了，则写段阻塞，直到读端读完新的内容。

```cpp
// 子进程任务集
class child_task
{
    public:
    // 子进程执行流
    void operator()(int arr[2])
    {
        close(arr[0]);
        const int& write_fd = arr[1];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 发出信息
        int count = 1;
        while(count)
        {
            // 生成一条信息
            buffer[0] = 0; // 在C/C++中字符串结尾为'\0'，此处暗示该缓冲区是字符串的载体
            // pid_t id = getpid();
            // snprintf(buffer, sizeof(buffer), "%s:: pid:%d count:%d", "Here is the child process", id, count);
            snprintf(buffer, sizeof(buffer), "%s", "c");

            // 将信息写到管道中
            write(write_fd, buffer, strlen(buffer));

            cout<<count<<endl;

            // if(count == 65536)
            // {
            //     cout<<"ready"<<endl;
            // }
            // else if(count > 65536)
            // {
            //     cout<<"go"<<endl;
            // }

            count++;
            // cout<<buffer<<endl;  检查
        }

        exit(0);
    }
};
```

```shell
# 上述内容省略
65524
65525
65526
65527
65528
65529
65530
65531
65532
65533
65534
65535
65536
^C
[wind@starry-sky pipe]$
```

我们每次只输入一个字节的数据，在输入`65536`个字节之后，管道就满了，所以就卡住了，之后我用`ctrl c`强行终止了它。所以管道的大小就是65536字节，或者说，`64KB`。看来也不用运行那个`ready go`，本来去掉计数打印，想让现象更明显的，现在看来不用了。

通过`ulimit -a`可以查看系统中一些重要资源的规格，其中就包括管道大小。

```shell
[wind@starry-sky pipe]$ ulimit -a
core file size          (blocks, -c) 0
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 0
file size               (blocks, -f) unlimited
pending signals                 (-i) 6942
max locked memory       (kbytes, -l) 64
max memory size         (kbytes, -m) unlimited
open files                      (-n) 65535
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) 4096
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimited
[wind@starry-sky pipe]$
```

第十行：`pipe size            (512 bytes, -p) 8`，它的意思是每个管道的缓冲区大小是512字节，并且管道的最大缓冲区数是8，所以一个管道最大存储4KB数据。至于为什么要分出八段，现在只需要知道这是为了支持多个进程的读写需求。

不过上面实验的结果是64KB，为什么有差别呢？这要去man的7号手册中找答案

```shell
[wind@starry-sky pipe]$ man 7 pipe
```

![image-20241204174211477](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241204174211640.png)

在`2.6.11`内核版本以前，是页的大小，即4096字节，之后的版本是65536字节。

再看看下面的那段，就是`PIPE_BUF`的那项

POSIX.1-2001 规定，写入管道的字节数小于 `PIPE_BUF` 的 `write(2)` 操作必须是原子性的：输出数据会作为一个连续的序列写入管道。
对于大于 `PIPE_BUF` 字节的写操作，可能不是原子性的：内核可能会将数据与其他进程写入的数据交错。POSIX.1-2001 要求 `PIPE_BUF` 至少为 512 字节。（在 Linux 上，`PIPE_BUF` 为 4096 字节。）精确的语义取决于文件描述符是否为非阻塞模式（`O_NONBLOCK`）、是否有多个写入者以及待写入字节数 `n`：

- **`O_NONBLOCK` 禁用，`n <= PIPE_BUF`**
  所有 `n` 字节会原子性地写入；如果管道中没有足够空间立刻写入 `n` 字节，`write(2)` 会阻塞。
- **`O_NONBLOCK` 启用，`n <= PIPE_BUF`**
  如果管道中有足够空间写入 `n` 字节，`write(2)` 会立即成功并写入所有 `n` 字节；否则，`write(2)` 会失败，并设置 `errno` 为 `EAGAIN`。
- **`O_NONBLOCK` 禁用，`n > PIPE_BUF`**
  写操作不是原子性的：写入的数据可能会与其他进程的 `write(2)` 操作交错；`write(2)` 会阻塞，直到所有 `n` 字节都被写入。
- **`O_NONBLOCK` 启用，`n > PIPE_BUF`**
  如果管道已满，`write(2)` 会失败，并设置 `errno` 为 `EAGAIN`。否则，可能会写入从 1 到 `n` 字节（即可能发生“部分写入”；调用者应检查 `write(2)` 的返回值，以确定实际写入了多少字节），这些字节可能会与其他进程的写入交错。

这里涉及到一个重要的概念：原子性

--------

**原子性（Atomicity）** 是计算机科学中一个重要的概念，用来描述一个操作要么完全执行，要么完全不执行，没有中间状态。这个概念确保了在多线程或多进程环境下，操作是不可分割的，即使在执行过程中发生了中断，也不会被打断或以不完整的状态被读取或修改。

在管道操作中，**原子性**指的是一次管道写操作中，数据要么完全写入，要么完全不写入。在进程间通信的上下文中，原子性保证了数据的一致性和完整性，特别是在多进程同时进行读写时。

原子性这个术语来源于古希腊哲学家对“原子”（atom）的定义：不可分割的最小粒子。尽管科学家们后来发现原子是由更小的粒子组成的，但“不可分割”的引申意义仍然被保留下来。在计算机科学中，原子性同样意味着操作不能被分割，它是一个不可拆分的单位。

在管道操作中，原子性主要体现在管道写入操作的行为上。当进程向管道写数据时，POSIX 标准规定：

1. **小于 `PIPE_BUF` 字节的写操作是原子的**：如果一次写操作的数据小于或等于 `PIPE_BUF`（例如，POSIX 定义的最小值为 512 字节，而在 Linux 上通常为 4096 字节），所有数据会作为一个整体、顺序地写入管道。这意味着管道中的数据不会被其他进程的写入操作打断或混淆，保持数据的完整性。
2. **大于 `PIPE_BUF` 字节的写操作是非原子的**：如果一次写入的数据量超过了 `PIPE_BUF`，那么这个写操作就可能变得非原子。此时，数据可能会被其他进程的写操作所“插队”，内核可能将多个进程的数据交错写入管道，导致数据被分割或打乱。

假设管道的 `PIPE_BUF` 大小为 4096 字节，两个进程同时向管道写入数据：

1. **原子写操作（小于或等于 `PIPE_BUF`）**：如果进程 A 写入 100 字节，进程 B 写入 200 字节，内核会保证每次写入的数据是完整的，顺序也不会被改变。进程 A 和进程 B 的写操作分别独立执行，不会互相干扰。
2. **非原子写操作（大于 `PIPE_BUF`）**：如果进程 A 写入 5000 字节，进程 B 写入 200 字节，那么进程 A 的 5000 字节数据可能会被拆分，且可能与进程 B 的 200 字节数据交错在一起，这样就打破了原本的数据顺序。内核可能会根据进程调度和管道状态，分次将数据写入。

**分段**的设计也与支持多个进程同时进行读写有着密切关系。

管道是进程间通信的一个重要方式，在多个进程同时操作管道时，如果没有适当的分段机制，可能会导致写入的数据相互干扰，或者读到部分无效数据。分段的设计能够确保每个写入操作都被适当处理，同时避免因多个进程的并发操作导致的错误。

当多个进程同时向管道写入数据时，如果没有分段或类似的机制，数据可能会相互覆盖或交错，这样接收方（即读取进程）就会收到不完整的数据。因此，分段的做法可以将数据切割成一定大小的块，每个块独立处理，这样就能确保即使多个进程同时写入数据，数据的顺序和完整性仍然能得到保证。

对于管道中的写操作，系统会确保每次小于 `PIPE_BUF` 字节的写入是原子的。通过分段处理，每段数据的写入和读取都能被视为一个原子操作，确保数据不被其他进程的操作干扰。这是管道分段的一个重要目的，即保证写入数据的原子性和一致性。

从细节上来说

管道将数据分段，每段数据都是独立的。这使得管道能在多个进程间进行读写而不发生冲突。例如，当一个进程在写入数据时，另一个进程也可以同时读取数据，而不会发生交叉。

每次数据的写入都有一个固定的**大小限制**（如 `PIPE_BUF` 字节）。如果写入的数据超过这个大小，它就会被分成多个部分逐步写入，而每个部分都保证按顺序完整地被写入。

上面的这些内容，实际上都可以视为一种协议，而且是最底层的。实际上，管道的原子性、分段机制以及如何处理并发读写（多个进程一块读写），都可以看作是一种底层的**通信协议**。这种协议通过硬件和操作系统的支持，保证了数据在进程间的传输不仅高效，而且可靠。通过确保数据的原子性、分段传输和并发控制，它提供了一个可靠的通信通道，支持复杂的操作和并发场景。

--------

接下来我们看看读写端不正常的时候，会发生什么。

先是写段关闭。

```cpp
// 父进程任务集
class parent_task
{
    public:
    // 父进程执行流
    void operator()(int arr[2], int id)
    {
        close(arr[1]);
        const int& read_fd = arr[0];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 读取信息
        int loop_counter = 1;
        while(1)
        {
            buffer[0] = 0;
            ssize_t end = read(read_fd, buffer, sizeof(buffer) - 1);

            // 查看read的返回情况
            cout<<end<<endl;

            // // 异常处理
            // if(end < 0)
            // {
            //     perror("failed read");
            // }
            // else
            // {
            //     buffer[end] = 0;
            //     // cout<<buffer<<"  "<<loop_counter<<endl;
            //     cout<<buffer<<endl;
            // }

            // // loop_counter+=2;
            // // loop_counter++;
        }
        pid_t i = waitpid(id, nullptr, 0);
        if(i != id)
        {
            // 等待失败

            exit(3);
        }
        exit(0);
    }
};

// 子进程任务集
class child_task
{
    public:
    // 子进程执行流
    void operator()(int arr[2])
    {
        close(arr[0]);
        const int& write_fd = arr[1];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 发出信息
        int count = 1;
        while(count)
        {
            // 生成一条信息
            // buffer[0] = 0; // 在C/C++中字符串结尾为'\0'，此处暗示该缓冲区是字符串的载体
            // pid_t id = getpid();
            // snprintf(buffer, sizeof(buffer), "%s:: pid:%d count:%d", "Here is the child process", id, count);
            snprintf(buffer, sizeof(buffer), "%s", "c");

            // 将信息写到管道中
            write(write_fd, buffer, strlen(buffer));

            // cout<<count<<endl;

            // if(count == 65536)
            // {
            //     cout<<"ready"<<endl;
            // }
            // else if(count > 65536)
            // {
            //     cout<<"go"<<endl;
            // }

            if(count == 5)
                break;

            count++;
            sleep(1);
            // cout<<buffer<<endl;  检查
        }

        exit(0);
    }
};
```

子进程在写入五次字符串后，就会跳出循环，最终退出，子进程都结束了，其打开的写段自然也就关闭了，此时只剩下读端。另外我们还知道，`read`返回的是读到的字符个数。

执行程序后我们发现，在最开始，读写端都处于正常状态，由于写段是间隔一秒写的，所以会引发读端的阻塞，而读到的字符个数都是一，所以每隔一秒父进程就打印一个一，在子进程退出后，父进程明显就不阻塞了，打印的都是0。

写段关闭后，当读端把管道中残留的信息都读出之后，管道就没信息可读了，所以`read`返回了0，意思是读到了文件末尾，另外，因为已经没有写段了，所以管道数据的单向流动性已经消失，所以其行为类似于普通文件，不发生阻塞行为，而是一直返回0。

为此我们就可以对读端稍作修改，当`read`的返回值为0时，说明写端已经关闭，因此可以结束通信，退出循环，这样就可以自然而言的回收子进程资源，而不用我们强行停止。

```cpp
// 父进程任务集
class parent_task
{
    public:
    // 父进程执行流
    void operator()(int arr[2], int id)
    {
        close(arr[1]);
        const int& read_fd = arr[0];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 读取信息
        int loop_counter = 1;
        while(1)
        {
            buffer[0] = 0;
            ssize_t end = read(read_fd, buffer, sizeof(buffer) - 1);

            // // 查看read的返回情况
            // cout<<end<<endl;

            // 异常处理
            if(end > 0)
            {
                buffer[end] = 0;
                // cout<<buffer<<"  "<<loop_counter<<endl;
                cout<<buffer<<endl;
            }
            else if(end == 0)
            {
                // 写端已经关闭，通信结束
                break;
            }
            else
            {
                // 未知错误，也跳出循环
                break;
            }

            // // loop_counter+=2;
            // // loop_counter++;
        }
        pid_t i = waitpid(id, nullptr, 0);
        if(i != id)
        {
            // 等待失败

            exit(3);
        }
        exit(0);
    }
};
```

# end