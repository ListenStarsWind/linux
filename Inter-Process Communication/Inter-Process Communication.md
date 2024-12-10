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

接下来讨论第四种情况：管道的读端被关闭，而写端仍保持正常。从逻辑上看，系统应该会阻止写端继续写入数据，因为读端已经不存在，写入操作也就失去了意义。关键在于，系统究竟会通过何种方式来阻止写入？

实际上，当管道的读端被关闭后，如果写端尝试写入数据，系统会直接终止对应的进程。这种处理方式表明写入操作被视为一种不可恢复的错误。以下是测试代码的具体表现，让我们深入探讨它的行为。

至于系统如何终止写端进程，实际上也是通过进程间通信实现的。系统作为特殊的进程，会向尝试写入的进程发送一个特定的信号（如 `SIGPIPE`），导致该进程异常终止。这意味着写入操作不仅被禁止，而且会触发严重的错误处理机制。

```shell
[wind@starry-sky pipe]$ kill -l
1) SIGHUP	 2) SIGINT	 3) SIGQUIT	 4) SIGILL	 5) SIGTRAP
6) SIGABRT	 7) SIGBUS	 8) SIGFPE	 9) SIGKILL	10) SIGUSR1
11) SIGSEGV	12) SIGUSR2	13) SIGPIPE	14) SIGALRM	15) SIGTERM
16) SIGSTKFLT	17) SIGCHLD	18) SIGCONT	19) SIGSTOP	20) SIGTSTP
21) SIGTTIN	22) SIGTTOU	23) SIGURG	24) SIGXCPU	25) SIGXFSZ
26) SIGVTALRM	27) SIGPROF	28) SIGWINCH	29) SIGIO	30) SIGPWR
31) SIGSYS	34) SIGRTMIN	35) SIGRTMIN+1	36) SIGRTMIN+2	37) SIGRTMIN+3
38) SIGRTMIN+4	39) SIGRTMIN+5	40) SIGRTMIN+6	41) SIGRTMIN+7	42) SIGRTMIN+8
43) SIGRTMIN+9	44) SIGRTMIN+10	45) SIGRTMIN+11	46) SIGRTMIN+12	47) SIGRTMIN+13
48) SIGRTMIN+14	49) SIGRTMIN+15	50) SIGRTMAX-14	51) SIGRTMAX-13	52) SIGRTMAX-12
53) SIGRTMAX-11	54) SIGRTMAX-10	55) SIGRTMAX-9	56) SIGRTMAX-8	57) SIGRTMAX-7
58) SIGRTMAX-6	59) SIGRTMAX-5	60) SIGRTMAX-4	61) SIGRTMAX-3	62) SIGRTMAX-2
63) SIGRTMAX-1	64) SIGRTMAX	
[wind@starry-sky pipe]$
```

在下面的代码中，我们先让读写端通信一次，然后读端关闭，然后查看父进程的等待结果。

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

            sleep(6);

            close(read_fd);

            cout<<"The read end has been closed."<<endl; // 读端已经关闭
            break;

            // // loop_counter+=2;
            // // loop_counter++;
        }

        sleep(8);  // 创建一个窗口期观察子进程的僵死状态
        int status = 0;
        pid_t i = waitpid(id, &status, 0);

        // 在ProcessWaiting.md中对waitpid的使用和返回有详细介绍
        if(WIFEXITED(status))
        {
            printf("Normal exit, code : %d\n", WEXITSTATUS(status));
        }
        else
        {
            printf("Exception Interrupt, code : %d\n", status & 0x7f);
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

            sleep(10);
            cout<<"The write end is preparing for the next write operation."<<endl; // 写段准备下一次写入

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

执行之后

```shell
[wind@starry-sky pipe]$ make clean ; make
[wind@starry-sky pipe]$ ./testPipe
c
The read end has been closed.
The write end is preparing for the next write operation.
Exception Interrupt, code : 13
[wind@starry-sky pipe]$
```

也就是这个信号是`SIGPIPE`。

接下来我们创建一个监控脚本来查看父子进程的状态。

```shell
[wind@starry-sky pipe]$ while : ; do ps ajx | head -1 ; ps ajx | grep testPipe | grep -v grep; sleep 1;  done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 Z+    1002   0:00 [testPipe] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 Z+    1002   0:00 [testPipe] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701   929   929 29701 pts/0      929 S+    1002   0:00 ./testPipe
  929   930   929 29701 pts/0      929 Z+    1002   0:00 [testPipe] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
^C
[wind@starry-sky pipe]$
```

### Application

下面我们来说说管道的应用场景。一是`shell`中的管道操作，下面我们找个无关紧要的指令，把它们用管道串联起来。

我们在一个窗口上执行`sleep 6194 | sleep 1024 | sleep 4096`，然后在另一个窗口上监控一下。

```shell
[wind@starry-sky pipe]$ ps ajx | head -1 ; ps ajx | grep sleep | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
29701  2193  2193 29701 pts/0     2193 S+    1002   0:00 sleep 6194
29701  2194  2193 29701 pts/0     2193 S+    1002   0:00 sleep 1024
29701  2195  2193 29701 pts/0     2193 S+    1002   0:00 sleep 4096
29538  2262 29538 29538 ?           -1 S     1002   0:00 sleep 180
[wind@starry-sky pipe]$
```

我们看到`sleep 6194`，`sleep 1024`，`sleep 4096`，有着相同的父进程，也就是`bash`，它们具有一定的血缘关系，简单的说，`bash`先创建两个管道，然后再创建`sleep 6194`和`sleep 1024`这两个子进程，在这两个子进程被对应的指令替换前，`sleep 6194`的标准输出被重定向到了第一个管道的写端，而`sleep 1024`的标准输入被重定向到了第一个管道的读端，然后又创建了进程`sleep 4096`，并把`sleep 1024`的标准输出重定向到第二个管道的写端，`sleep 4096`的标准输入重定向到第二个管道的读端，这样它们就被联系起来了。

具体我们就不写了，对我们来说，用`shell`作应用场景已经没有太大价值，所以我们换个场景。

管道可以用于构建进程池。创建新进程对于系统来说是一个相对昂贵且复杂的操作。因此，与其在需要时再动态创建新进程，不如在程序初始化时预先一次性创建多个子进程，并通过管道与这些子进程通信，让它们根据接收到的指令执行特定任务。

例如，可以一次性创建三个子进程，分别记为 A、B 和 C。每创建一个子进程之前，先创建一个管道作为父进程与该子进程之间的通信信道。其中，父进程充当管道的写端，子进程则充当管道的读端。这样，在创建完三个子进程后，父进程就持有三个独立的管道，分别对应子进程 A、B 和 C。父进程向管道 a 写入指令，子进程 A 便会从中读取并执行相应任务。同理，B 和 C 的操作方式类似。

需要注意的是，管道的通信方式相对简单、底层，其数据传输是面向字节流的。这种特性决定了通信双方需要事先约定数据的格式和含义，以便正确解析。一个常见的约定是使用任务码，每个任务码对应一个具体的功能实现。父进程通过管道发送任务码，子进程解析任务码后调用相应的功能，从而实现灵活的任务分配。如果子进程在管道中读不到信息，则会进入阻塞状态。

在进程池的设计中，管道通信可以作为一个底层机制，而面向对象的设计思想可以被用来封装这一过程。虽然进程池本质上是面向过程的，因为进程具有独立性，无法直接共享内存或状态，它依赖于通过管道在父进程和子进程之间进行任务传递，但我们可以通过面向对象的方法来更高层次地抽象和管理这个过程。具体来说，我们可以将每个子进程封装为一个对象，进程池和任务调度机制也可以封装成一个类，用户只需与这些类的接口进行交互，而不需要关心底层的进程创建、管道操作或任务分配的细节。

子进程本身可以作为一个独立的类来管理，负责启动、执行任务以及与父进程通过管道进行通信。管道的操作也可以封装在一个专门的类中，提供简洁的接口来实现数据的传输。进程池则负责管理多个子进程，调度空闲进程来处理任务，保证任务能够在进程池中均衡分配。

我们先定义一个结构体用于描述每个子进程的进程ID和对应管道的文件描述符

```cpp
#include <unistd.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>

namespace wind
{
    struct channel
    {
        int _cmdfd;               // 负责发送指令的文件描述符
        pid_t _taskid;            // 子进程的pid
        std::string _processname; // 子进程的代称

        channel(int cmdfd, int taskid, const std::string &name = std::string())
            : _cmdfd(cmdfd), _taskid(taskid), _processname(name)
        {
        }
    };
}
```

接着我们去定义进程池这个类

```cpp
class taskpool
    {
        typedef void (*func)();

    public:
        taskpool(std::vector<func> tasks, int processnum = 5) // 缺省参数默认创建五个子进程
            : _tasks(tasks)  // 定义和初始化对象中的任务集
        {
            // 函数体部分负责信道的建立
            // 包含子进程及其对应管道的创建
                
            
        }

    private:
        std::vector<channel> _channels;   // 描述子进程及对应的信道
        std::vector<func> _tasks;         // 任务集，为了降低理解门槛，这里就用无参无返回函数指针，任务码就是其对应的下标
    };
```

```cpp
// 函数体部分负责信道的建立
// 包含子进程及其对应管道的创建

while (processnum--)
{
    int pipeid[2];
    int i = pipe(pipeid);

    // 管道异常处理
    if (i != 0)
    {
        fprintf(stderr, "%s\n", "Fatal error, pipe cannot be created.");
        exit(1);
    }

    pid_t id = fork();
    if (id > 0)
    {
        // 父进程流
        close(pipeid[0]); // 关闭读端

        // 输出提示
        fprintf(stderr, "%s:%d %s%d\n", "子进程创建成功 pid",id, "信道为", pipeid[1]);

        _channels.push_back(channel(pipeid[1], id));
    }
    else if (id == 0)
    {
        // 子进程流
        
        // 子进程文件状态初始化
        for (const auto &e : _channels) // 关闭从父进程继承下来的其余兄弟进程管道
        {
            close(e._cmdfd);
        }

        close(pipeid[1]);   // 关闭写端
        dup2(pipeid[0], 0); // 将管道重定向为标准输入
        close(pipeid[0]);   // 顺手关掉原文件
        
        // 子进程的轮询执行内容
        while (1)
        {
            int i; // 数据缓冲区
            ssize_t n = read(0, &i, sizeof(int));
            printf("子进程%d所读到的字节数:%d\n", getpid(), n);
            if (n == sizeof(int))
            {
                _tasks[i]();
            }
            else if (n == 0)
            {
                // 写端已经关闭
                break;
            }
            else
            {
                // read出错了
                fprintf(stderr, "%s\n", "Fatal error, Read failed.");
            }
        }
        // 提示性信息，用于丰富运行时效果
        printf("%s:%d\n", "这个子进程即将退出", getpid());
        exit(0);
    }
    else
    {
        // fork出错了
        fprintf(stderr, "%s\n", "Fatal error, process cannot be created.");
    }
}
```

这里有一个很重要的点要注意，就是子进程文件状态初始化中一定要把`_channels`里的文件文件描述符给关掉。下面我拿创建两个子进程的情况简要说明一下。

我们记第一个被创建的进程是A，在A即将被创建前，父进程的文件描述表是这样的：0号下标是标准输入，1号下标是标准输出，2号下标是标准错误，在管道被创建后，我们记这个管道为a，a的读端位于3号下标，a的写端位于4号下标。

进程分流之后，父进程的管道读端被关闭，这样父进程文件描述符表的3号下标现在就指向空了；而A继承了父进程的文件描述符状态，3号是读端，4号是写端，随后我们关闭了子进程的写端，并在重定向读端之后关闭了原读端，所以A文件描述符表的状态是，0号下标是读端，2号是标准输出，3号是标准错误，到目前还没有问题。

在第二次循环，我们创建了新的管道b，在B被创建之前，父进程的文件描述符表状态是，0号标准输入，1号标准输出，2号标准错误，3号是b的读端，4号是a的写端，5号是b的写端。

在B创建之后，父进程关闭了b的读端，于是现在父进程的状态是0号标准输入，1号标准输出，2号标准错误，3号是空，4号是a的写端，5号是b的写端；而B继承了父进程分流前的文件状态，即0号标准输入，1号标准输出，2号标准错误，3号是b的读端，4号是a的写端，5号是b的写端，随后我们关闭了子进程的写端，并在重定向读端之后关闭了原读端，所以B文件描述符表的状态是0号b的读端，1号标准输出，2号标准错误，3号是空，4号是a的写端，5号是空。

这样，B就有了a的写端，这就会引发进程等待的问题，我们的本意是，每个管道都只有一个写端和一个读端，写端在父进程，读端在子进程，可现在，有些管道有多个写端，比如对于上面的a来说，它有两个写端，一个在父进程，一个在B中，那等到父进程关闭a的写端时，由于B中也有一个a的写端，所以a的写端没有完全关闭，所以A就会一直阻塞在`read`。此时如果父进程采用的是阻塞等待就会因为A迟迟不退出而也发生阻塞，这样父子进程都始终无法退出。

所以一定子进程进入之后一定要把不相关的文件关了。

接下来是调用，采用的是轮询调度机制

```cpp
bool call(int taskcode)
{
    static int scheduling = 0;
    if (taskcode >= 0 && taskcode < _tasks.size())
    {
        printf("向%d发送消息\n", _channels[scheduling]._taskid);
        write(_channels[scheduling]._cmdfd, &taskcode, sizeof(int));
        scheduling++;
        if (scheduling == _channels.size())
            scheduling = 0;
        return true;
    }
    else
    {
        // 非法的任务码
        return false;
    }
}
```

析构负责回收子进程资源

```cpp
~taskpool()
{
    for (const auto &e : _channels)
    {
        int ret = close(e._cmdfd);

        fprintf(stderr, "%s:%d\n", "信道已关闭", e._cmdfd);

        int status = 0;
        pid_t i = waitpid(e._taskid, &status, 0);

        // 在ProcessWaiting.md中对waitpid的使用和返回有详细介绍
        if (i > 0)
        {
            // 回收成功
            printf("回收进程资源%d\n", e._taskid);

            // 查看子进程退出状态
            if (WIFEXITED(status))
            {
                printf("正常退出, 退出码 : %d\n", WEXITSTATUS(status));
            }
            else
            {
                printf("异常退出, 退出码 : %d\n", WTERMSIG(status));
            }
        }
        else if (i < 0)
        {
            // waitpid自己出错了
            fprintf(stderr, "%s\n", "Fatal error, Waitpid Failed.");
        }
        else
        {
            // 实际上不会用到该分支，顺手写的，若是采用轮询式等待，具有返回0的可能性
            fprintf(stderr, "%s\n", "No child process state changed.");
        }
    }
}
```

上述代码皆位于`taskpool.hpp`中，接下来我们写一下配套文件

```cpp
// tasks.hpp
#include <iostream>

void f1()
{
    std::cout << 1 << std::endl;
}

void f2()
{
    std::cout << 2 << std::endl;
}

void f3()
{
    std::cout << 3 << std::endl;
}

void f4()
{
    std::cout << 4 << std::endl;
}

void f()
{
}

// main.cc
#include<vector>
#include<string>
#include"taskpool.hpp"
#include"tasks.hpp"

using namespace std;

typedef void(*func)();

int main()
{
    // vector<func> tasks = {f1, f2, f3, f4};
    vector<func> tasks = {f, f, f, f};
    wind::taskpool t(tasks);
    int i = 5;
    int j = 0;
    while(i--)
    {
        t.call(j);
        j++;
        if(j == tasks.size())
        {
            j = 0;
        }
    }
    return 0;
}
```

```makefile
processPool: main.cc taskpool.hpp tasks.hpp
	@g++ $^ -g -o $@
.PHONY:clean
clean:
	@rm -f processPool
run:processPool
	@./$^
```

我们来运行一下

```shell
[wind@starry-sky pipe_using]$ make clean;make
[wind@starry-sky pipe_using]$ ./processPool 
子进程创建成功 pid:22331 信道为4
子进程创建成功 pid:22332 信道为5
子进程创建成功 pid:22333 信道为6
子进程创建成功 pid:22334 信道为7
子进程创建成功 pid:22335 信道为8
向22331发送消息
向22332发送消息
向22333发送消息
向22334发送消息
向22335发送消息
信道已关闭:4
子进程22331所读到的字节数:4
子进程22331所读到的字节数:0
这个子进程即将退出:22331
子进程22332所读到的字节数:4
回收进程资源22331
正常退出, 退出码 : 0
信道已关闭:5
子进程22332所读到的字节数:0
这个子进程即将退出:22332
子进程22333所读到的字节数:4
子进程22334所读到的字节数:4
子进程22335所读到的字节数:4
回收进程资源22332
正常退出, 退出码 : 0
信道已关闭:6
子进程22333所读到的字节数:0
这个子进程即将退出:22333
回收进程资源22333
正常退出, 退出码 : 0
信道已关闭:7
子进程22334所读到的字节数:0
这个子进程即将退出:22334
回收进程资源22334
正常退出, 退出码 : 0
信道已关闭:8
子进程22335所读到的字节数:0
这个子进程即将退出:22335
回收进程资源22335
正常退出, 退出码 : 0
[wind@starry-sky pipe_using]$
```

下面我们看看如果子进程没有关闭从父进程继承下来的文件会发生什么。

```cpp
else if (id == 0)
{

    // for (const auto &e : _channels) // 关闭从父进程继承下来的其余兄弟进程管道
    // {
    //     close(e._cmdfd);
    // }

    // 子进程流
    close(pipeid[1]);   // 关闭写端
    dup2(pipeid[0], 0); // 将管道重定向为标准输入
    close(pipeid[0]);   // 顺手关掉原文件
    while (1)
    {
        // printf("%s:%d\n", "这是子进程", getpid());
        int i;
        ssize_t n = read(0, &i, sizeof(int));
        printf("子进程%d所读到的字节数:%d\n", getpid(), n);
        if (n == sizeof(int))
        {
            _tasks[i]();
        }
        else if (n == 0)
        {
            // 写端已经关闭
            break;
        }
        else
        {
            fprintf(stderr, "%s\n", "Fatal error, Read failed.");
        }
    }
    printf("%s:%d\n", "这个子进程即将退出", getpid());
    exit(0);
}
```

如果我们这样等待，就会直接卡死

```cpp
~taskpool()
{
    for (const auto &e : _channels)
    {
        int ret = close(e._cmdfd);

        fprintf(stderr, "%s:%d\n", "信道已关闭", e._cmdfd);

        int status = 0;
        pid_t i = waitpid(e._taskid, &status, 0);

        // 在ProcessWaiting.md中对waitpid的使用和返回有详细介绍
        if (i > 0)
        {
            // 回收成功
            printf("回收进程资源%d\n", e._taskid);

            // 查看子进程退出状态
            if (WIFEXITED(status))
            {
                printf("正常退出, 退出码 : %d\n", WEXITSTATUS(status));
            }
            else
            {
                printf("异常退出, 退出码 : %d\n", WTERMSIG(status));
            }
        }
        else if (i < 0)
        {
            // waitpid自己出错了
            fprintf(stderr, "%s\n", "Fatal error, Waitpid Failed.");
        }
        else
        {
            // 实际上不会用到该分支，顺手写的，若是采用轮询式等待，具有返回0的可能性
            fprintf(stderr, "%s\n", "No child process state changed.");
        }
    }
}

```

```shell
[wind@starry-sky pipe_using]$ make clean ; make
[wind@starry-sky pipe_using]$ ./processPool
子进程创建成功 pid:12782 信道为4
子进程创建成功 pid:12783 信道为5
子进程创建成功 pid:12784 信道为6
子进程创建成功 pid:12785 信道为7
子进程创建成功 pid:12786 信道为8
向12782发送消息
向12783发送消息
向12784发送消息
向12785发送消息
向12786发送消息
信道已关闭:4
子进程12786所读到的字节数:4
子进程12783所读到的字节数:4
子进程12782所读到的字节数:4
子进程12785所读到的字节数:4
子进程12784所读到的字节数:4

```

我们再从另一个窗口中看看进程状态

```shell
[wind@starry-sky pipe_using]$ ps ajx | head -1 ; ps ajx | grep processPool | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
12314 12781 12781 12314 pts/8    12781 S+    1002   0:00 ./processPool
12781 12782 12781 12314 pts/8    12781 S+    1002   0:00 ./processPool
12781 12783 12781 12314 pts/8    12781 S+    1002   0:00 ./processPool
12781 12784 12781 12314 pts/8    12781 S+    1002   0:00 ./processPool
12781 12785 12781 12314 pts/8    12781 S+    1002   0:00 ./processPool
12781 12786 12781 12314 pts/8    12781 S+    1002   0:00 ./processPool
[wind@starry-sky pipe_using]$
```

我们看到包括父进程在内的所有进程都卡死了，都进入了阻塞状态，对于父进程，是在`waitpid`阻塞，对于子进程，是在`read`阻塞

然后看看外部阻塞等待会怎么样

```cpp
~taskpool()
{
    for (const auto &e : _channels)
    {
        int ret = close(e._cmdfd);

        fprintf(stderr, "%s:%d\n", "信道已关闭", e._cmdfd);

    }

    int i = 0;
    while(i <_channels.size())
    {
        int status = 0;
        pid_t ret = waitpid(_channels[i]._taskid, &status, 0); // 外部阻塞等待
        //pid_t ret = waitpid(_channels[i]._taskid, &status, WNOHANG);

        if (ret > 0)
        {
            i++;
            printf("回收进程资源%d\n", ret);
            if (WIFEXITED(status))
            {
                printf("正常退出, 退出码 : %d\n", WEXITSTATUS(status));
            }
            else
            {
                printf("异常退出, 退出码 : %d\n", status & 0x7f);
            }
        }
        else if (ret < 0)
        {
            perror("failed wait");
        }
        else
        {
            printf("未获得等待状态\n");
        }
        sleep(1);
    }
}
```

```shell
[wind@starry-sky pipe_using]$ make clean ; make
[wind@starry-sky pipe_using]$ ./processPool
子进程创建成功 pid:13176 信道为4
子进程创建成功 pid:13177 信道为5
子进程创建成功 pid:13178 信道为6
子进程创建成功 pid:13179 信道为7
子进程创建成功 pid:13180 信道为8
向13176发送消息
向13177发送消息
向13178发送消息
向13179发送消息
向13180发送消息
信道已关闭:4
信道已关闭:5
信道已关闭:6
信道已关闭:7
信道已关闭:8
子进程13176所读到的字节数:4
子进程13177所读到的字节数:4
子进程13178所读到的字节数:4
子进程13179所读到的字节数:4
子进程13180所读到的字节数:4
子进程13180所读到的字节数:0
这个子进程即将退出:13180
子进程13179所读到的字节数:0
这个子进程即将退出:13179
子进程13178所读到的字节数:0
这个子进程即将退出:13178
子进程13177所读到的字节数:0
这个子进程即将退出:13177
子进程13176所读到的字节数:0
这个子进程即将退出:13176
回收进程资源13176
正常退出, 退出码 : 0
回收进程资源13177
正常退出, 退出码 : 0
回收进程资源13178
正常退出, 退出码 : 0
回收进程资源13179
正常退出, 退出码 : 0
回收进程资源13180
正常退出, 退出码 : 0
[wind@starry-sky pipe_using]$
```

我们看到一个很奇怪的现象，首先没有卡死，确实全部进程都成功退出了，另一个现象是，它这里进程的退出（也就是“这个子进程即将退出”提示信息）和进程的资源回收（“回收进程资源”提示信息）给人一种递归的感觉，为什么呢？

第一个关闭的进程是`13180`，为什么呢？因为它后面没有进程了，所以与之对应的管道写端就一个，就在父进程里面，而父进程又关闭了这个写端，所以`13180`就先退出了，`13180`的退出也会导致其从父进程继承下来的其它管道写端而关闭，于是`13179`的所有写端就关闭了，所以它就自己退出了，接着是是`13178`，`13177`，`13176`，`13176`结束之后，父进程就等到了，于是就回收了进程资源，接着是`13177`，`13178`，`13179`，`13180`，所以给人一种递归的感觉。

如果你采用轮询等待，可能会先打印`未获得等待状态`，因为"递"的过程需要一定时间，所以第一次等待可能并没有等到，之后我们可能因为休眠一秒，所以“递”的过程就结束了，于是就能等到了。

```cpp
~taskpool()
{
    for (const auto &e : _channels)
    {
        int ret = close(e._cmdfd);

        fprintf(stderr, "%s:%d\n", "信道已关闭", e._cmdfd);

    }

    int i = 0;
    while(i <_channels.size())
    {
        int status = 0;
        //pid_t ret = waitpid(_channels[i]._taskid, &status, 0); // 外部阻塞等待
        pid_t ret = waitpid(_channels[i]._taskid, &status, WNOHANG);

        if (ret > 0)
        {
            i++;
            printf("回收进程资源%d\n", ret);
            if (WIFEXITED(status))
            {
                printf("正常退出, 退出码 : %d\n", WEXITSTATUS(status));
            }
            else
            {
                printf("异常退出, 退出码 : %d\n", status & 0x7f);
            }
        }
        else if (ret < 0)
        {
            perror("failed wait");
        }
        else
        {
            printf("未获得等待状态\n");
        }
        sleep(1);
    }
}
```

```shell
[wind@starry-sky pipe_using]$ make clean ; make
[wind@starry-sky pipe_using]$ ./processPool
子进程创建成功 pid:14353 信道为4
子进程创建成功 pid:14354 信道为5
子进程创建成功 pid:14355 信道为6
子进程创建成功 pid:14356 信道为7
子进程创建成功 pid:14357 信道为8
向14353发送消息
向14354发送消息
向14355发送消息
向14356发送消息
向14357发送消息
信道已关闭:4
信道已关闭:5
信道已关闭:6
信道已关闭:7
信道已关闭:8
未获得等待状态
子进程14353所读到的字节数:4
子进程14354所读到的字节数:4
子进程14355所读到的字节数:4
子进程14357所读到的字节数:4
子进程14357所读到的字节数:0
这个子进程即将退出:14357
子进程14356所读到的字节数:4
子进程14356所读到的字节数:0
这个子进程即将退出:14356
子进程14355所读到的字节数:0
这个子进程即将退出:14355
子进程14354所读到的字节数:0
这个子进程即将退出:14354
子进程14353所读到的字节数:0
这个子进程即将退出:14353
回收进程资源14353
正常退出, 退出码 : 0
回收进程资源14354
正常退出, 退出码 : 0
回收进程资源14355
正常退出, 退出码 : 0
回收进程资源14356
正常退出, 退出码 : 0
回收进程资源14357
正常退出, 退出码 : 0
[wind@starry-sky pipe_using]$
```

不过我们最好还是直接在子进程关闭继承下来的写端，从而避免出现一些其它的未知问题。

 ## Named_Pipe

之前我们说的是匿名管道，接下来我们说说命名管道。匿名管道因为没有名字，所以无法指代，因此只能通过父子进程分流时的文件状态继承来建立联系，而对于命名管道来说，因为具有名字所以可以指代，即使是对于没有任何关系的两个进程来说，命名管道也可以作为它们通信的媒介。

我们先用指令创建一个命名管道。

![image-20241209194447260](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241209194447437.png)

为什么叫`FIFO`先进先出呢？因为管道就像是队列一样，其中的字节流具有先进先出的性质。

```shell
[wind@starry-sky named_pipes]$ ll
total 0
[wind@starry-sky named_pipes]$ mkfifo pipe
[wind@starry-sky named_pipes]$ ls
pipe
[wind@starry-sky named_pipes]$ ll
total 0
prw-rw-r-- 1 wind wind 0 Dec  9 20:16 pipe
[wind@starry-sky named_pipes]$
```

文件类型以`p`开头，虽然`ll`可以检索出它，但它并不是硬盘文件，而是和匿名管道一样，都是内存文件。

如果没有建立信道，即同时打开命名管道的写端和读端，那对它的读写都会进入阻塞。比如我们现在建立两个窗口，准备一个对管道写，一个对管道读。

如果光写，就会发现进程进入了阻塞状态

```shell
[wind@starry-sky named_pipes]$ echo "You cannot step into the same river twice." > pipe
[wind@starry-sky named_pipes]$ while : ; do echo "You cannot step into the same river twice."; sleep 1; done >> pipe
```

```shell
[wind@starry-sky named_pipes]$ cat pipe
You cannot step into the same river twice.
[wind@starry-sky named_pipes]$ cat pipe
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.
You cannot step into the same river twice.

```

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241209203803707.mp4"></video>

命名管道的原理和匿名管道一致。虽说管道是一种内存级文件，但是，它仍然具有内存级别文件结构，有内存级的缓冲区，只不过存储媒介不同，也有对应的`inode`编号。虽说它是内存文件，但其文件名和inode编号的映射关系仍被记录在所在目录的data block中，这是为了保持文件系统的一致性。反正有管道路径就能找到这个管道，所以可被指代，因此可供毫无关系的进程间进行通信。

现在我们用代码写写看

就把这两个独立进程叫做`server`(服务端)和`client`(用户端)，它们有一个共用的头文件`comm`(communication 通信)。

```makefile
.PHONY:all
all:client server
client:client.cc
	@g++ $^ -o $@
server:server.cc
	@g++ $^ -o $@

.PHONY:clean
clean:
	@rm -rf client server
```

通信的形式是，用户对客户端输入信息，服务端打印信息

```cpp
// comm.h
#pragma once

#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<string>


#define FIFO_NAME "myfifo" // The name of the pipe
#define MODE 0664          // The creation mode of the pipe

enum exit_code
{
    FIFO_CREATE_ERROR = 1,
    FIFO_REMOVE_ERROR = 2,
    FIFO_OPEN_ERROR = 3,
    FIFO_READ_ERROR = 4,
    FIFO_WRITE_ERROR = 5
};


// server.cc
#include"comm.h"

using namespace std;

int main()
{
    // 创建命名管道
    if(mkfifo(FIFO_NAME, MODE) != 0)
    {
        perror("failed mkfifo");
        exit(FIFO_CREATE_ERROR);
    }

    // 建立信道
    int fd = open(FIFO_NAME, O_RDONLY);
    if(fd < 0)
    {
        perror("failed open");
        exit(FIFO_OPEN_ERROR);
    }

    cout<<"The server is now able to receive information."<<endl;

    // 数据处理
    char buffer[1024];
    while(1)
    {
        buffer[0] = 0;
        ssize_t  n = read(fd, buffer, sizeof(buffer) - 1);
        if(n > 0)
        {
            buffer[n] = 0;
            cout<<"client say# "<<buffer<<endl;
        }
        else if(n == 0)
        {
            cout<<"client quit, me too."<<endl;
            break;
        }
        else
        {
            perror("failed read");
            exit(FIFO_READ_ERROR);
        }
    }


    // 善后操作
    close(fd);
    if(unlink(FIFO_NAME) != 0)
    {
        perror("failed unlink");
        exit(FIFO_REMOVE_ERROR);
    }

    return 0;
}

// client.cc
#include"comm.h"


using namespace std;
int main()
{
    int fd = open(FIFO_NAME, O_WRONLY);
    if(fd < 0)
    {
        perror("failed open");
        exit(FIFO_OPEN_ERROR);
    }

    cout<<"The client is now able to send messages."<<endl;

    string buffer;
    while(1)
    {
        cout<<"Please input# ";
        getline(cin, buffer); // 不把空格作为分隔符
        if(write(fd, buffer.c_str(), buffer.size()) < 0)
        {
            perror("failed wwirte");
            exit(FIFO_WRITE_ERROR);
        }
    }

    close(fd);
    return 0;
}
```

代码具体内容就不说了，我们直接看看现象

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241210144419021.mp4"></video>

当服务端刚刚打开后，由于客户端还没运行，所以管道写端还未打开，因此服务端一直阻塞在`open`。也就是说，单纯考虑`open`对管道的打开是否阻塞取决于是否建立了完整的信道，如果只有读端，没有写端，则读端会一直阻塞在`open`，如果没有读端，只有写端，写端也会一致阻塞在`open`，当写端关闭后，读端`read`返回0，意味着文件已经读到末尾。

# end