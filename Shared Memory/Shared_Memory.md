# Shared Memory

## concept

共享内存是基于`System V  `标准的一套进程通信方案，它的运作机理和管道一致：让不同的进程与第三方空间建立联系。

就是系统在物理内存中找到一处空闲空间，将其映射到进程地址空间而已。具体被映射到进程地址空间的共享区。

![image-20241214160114166](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214160114236.png)

使用共享内存建立信道就两步：一，从物理内存上找处空闲区域；二，把物理地址与虚拟地址建立联系。在信道建立之后，共享内存的行为就像由`malloc`开辟的堆空间一样，不用通过系统接口直接读写。共享内存的释放也和堆空间很相似。

和管道一样，共享内存的真正所有权都由系统掌握，系统只不过把共享内存的使用权给了进程，这样能确保共享内存的相对独立性。诚然，系统中可能有很多共享内存，为了对这些共享内存进行管理，系统需要对这些共享内存进行抽象描述，然后再用某种数据结构把它们串联起来。所以对共享内存的学习无非两部分：一是学习共享内存相关系统接口的用法，二是学习系统怎么管理它们的，我们今天主要关注第一部分。

## usage

多说无益，让我们直接来用共享内存的接口吧。

先来看第一个接口，用于在物理内存上找到一处空闲区域`shmget`

![image-20241214163431072](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214163431155.png)

我们先看`shmget`的第二个参数`size`，该参数用于描述申请共享空间的大小，单位为字节。然后再来看看它的返回值，它返回一个整数，这个整数是一个唯一标识符，用来在应用层区别不同的共享内存，如果`shmget`失败，则返回-1，给人的感觉就是文件描述附表的下标，但其实不是，稍后我们会实验。第三个参数`shmflg`用于描述共享内存的其它信息，比如获取方式，创建权限之类的，它使用的传参方式是比特位传参。

`shmflg`的第一个待选项是`IPC_CREAT`，用于描述共享内存的获取方式：如果有现成的，就直接给你，没有就创建一个新的。
第二个参数`IPC_EXCL`不单独使用，都是与`IPC_CREAT`配合使用：`IPC_CREAT | IPC_EXCL`，这个组合的含义是，如果共享内存不存在，就新创建一个，如果共享内存存在，就调用失败，返回-1。

为什么要有`IPC_CREAT | IPC_EXCL`这种组合呢？假设之前已经有进程创建了一个共享内存，我们称之为a，现在又有一个进程创建共享内存，如果能创建的话，理论上，被创建的共享内存也应该是a，如果不使用`IPC_CREAT | IPC_EXCL`，新的进程就会使用原来就有的a，而不会新创建一个，这就可能会使得新进程对共享内存的写数据影响到原来a存在的通信，也就是串台了，所以使用`IPC_CREAT | IPC_EXCL`就能确保这里获取的共享内存是新创建的，不是使用原来就有的，从而避免出现上述的情况。

第一个参数`key`需要详细说说。

进程具有独立性，而共享内存也有唯一标识符，现在的问题是，如何让同一对进程获得同一个共享内存标识符，只有两个进程找到的共享内存是同一个，它们之间才可以通信。我们可以让先运行的进程创建并获取一个新的共享内存，接着后运行的进程通过某种方式也拿到一个同样的共享内存标识符，这样它们就能通信了。现在的问题是，怎么让它们获得标识符是同一个，有两种方案：两个进程运行后通过其它一些进程通信手段，比如管道，分享这个标识符，这样它们就能得到同一个标识符了，这种方案我们就不说了，我们现在想要让共享内存独立于其它通信方案而建立，这就需要第二种方案：让这个唯一标识符在进程运行前就已经相同，那它们运行后就自然是相同的。

实际上，这个`key_t`类型就是一个整型，那我们可不可以在代码上直接随手写一个整型，只要让通信进程的整型参数一致，就能在运行前让通信进程拥有相同的标识符呢？理论上可以，但实际上不行，因为随手写一个不能确保标识符唯一性，存在较大的可能与以往标识符重合，为此，系统为我们提供了一个接口，那就是`ftok`，`ftok`就是一套算法，虽说是系统提供的，但并没有对系统内核进行任何操作，因此在`man`手册中它被归类于语言接口。

![image-20241214172114632](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214172114739.png)

它借助于系统中天生就具有唯一性的东西：文件路径作为标识符唯一性的担保，也就是第一个参数`pathname`，除此之外，它还提供了一个整型参数`proj_id`，通过内部的一系列转换过程，将这两个参数转化为一个整型，这样就能大大减少标识符重复的可能。这两个参数的具体内容并不重要，重要的是，使用共享内存通信的进程要有相同的参数内容。

```cpp
// shmm.hpp
#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include<string>
#include"log.hpp"

// 先用常量
const std::string pathname = "/home/wind";
const int proj_id = 0x6194;
const int size = 4096;

wind::Log log;

enum error
{
    ftok_error = 1,
    shmget_error
};

key_t getKey()
{
    int n = ftok(pathname.c_str(), proj_id);
    if(n < 0)
    {
        log(wind::Fatal, "filed ftok:%s", strerror(errno));
        exit(ftok_error);
    }
    else
    {
        log(wind::Info, "succeeded ftok, the key is %d", n);
        return n;
    }
}

int getShmid()
{
    int n = shmget(getKey(), size, IPC_CREAT | IPC_EXCL);
    if(n == -1)
    {
        log(wind::Fatal, "create share memory error:%s", strerror(errno));
        exit(shmget_error);
    }
    else
    {
        log(wind::Info, "create share memory success, the shmid is %d", n);
        return n;
    }
}


// processa.cc
#include"shmm.hpp"

using namespace wind;
extern Log log;

int main()
{
    getShmid();
    sleep(5);
    log(Info, "a quit...");

    return 0;
}
```

```makefile
.PHONY:all
all:a b

a:processa.cc
	@g++ $^ -o $@
b:processb.cc
	@g++ $^ -o $@

.PHONY:clean
clean:
	@rm -f a b
```

```shell
[wind@starry-sky Shared Memory]$ make
In file included from processa.cc:1:
shmm.hpp:15:11: warning: built-in function ‘log’ declared as non-function [-Wbuiltin-declaration-mismatch]
   15 | wind::Log log;
      |           ^~~
In file included from processb.cc:1:
shmm.hpp:15:11: warning: built-in function ‘log’ declared as non-function [-Wbuiltin-declaration-mismatch]
   15 | wind::Log log;
      |           ^~~
[wind@starry-sky Shared Memory]$ ls
a  b  log.hpp  makefile  processa.cc  processb.cc  shmm.hpp
[wind@starry-sky Shared Memory]$ ./a
[Fatal][2024-12-14 18:24:27]::filed ftok:Success
```

我们看到执行结果是出错的，这意味着我们要改一下第二个参数，但`errno`并没有被设置，这可能是某种`bug`

```cpp
const int proj_id = 0x1246;
```

```shell
[wind@starry-sky Shared Memory]$ make clean
[wind@starry-sky Shared Memory]$ make
In file included from processa.cc:1:
shmm.hpp:15:11: warning: built-in function ‘log’ declared as non-function [-Wbuiltin-declaration-mismatch]
   15 | wind::Log log;
      |           ^~~
In file included from processb.cc:1:
shmm.hpp:15:11: warning: built-in function ‘log’ declared as non-function [-Wbuiltin-declaration-mismatch]
   15 | wind::Log log;
      |           ^~~
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-14 19:49:20]::succeeded ftok, the key is 1174470660
[Info][2024-12-14 19:49:20]::create share memory success, the shmid is 0
[Info][2024-12-14 19:49:25]::a quit...
[wind@starry-sky Shared Memory]$
```

我们看到返回值很特别，是0，这是一种很少见的情况，这意味着系统之前没有一个共享内存。`key`，也就是`1174470660`，是内核级别的共享内存标识符，对于我们用户来说，我们对该共享内存的指代，使用的都是`shmid`的值，`shmid`是应用级共享内存标识符，应用层用的都是`shmid`，不管是在代码中，还是在`bash`这种命令行解释器中，`bash`也是在应用层的。

现在让我们再次运行一下

```shell
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-14 20:0:16]::succeeded ftok, the key is 1174470660
[Fatal][2024-12-14 20:0:16]::create share memory error:File exists
[wind@starry-sky Shared Memory]$
```

因为我们没有改参数，所以生成的秘钥`key`还是一样的，但当我们使用这个秘钥获取共享内存时就报错了，这是因为，共享内存的生命周期是独立于进程单独存在的，只要用户不主动释放它，或者电脑没关机，它都会一直存在，而我们的获取方式是`IPC_CREAT | IPC_EXCL`，因此自然出现了问题。

`ipcs -m`可以查看当前系统的所有共享内存

```shell
[wind@starry-sky Shared Memory]$ ipcs -m

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 0          wind       0          4096       0                       

```

现在只有一个共享内存，也就是刚刚我们创建的那个。我们看到系统的`key`是用十六进制的，所以我们可以改改代码，改成`%x`

需要说明的是，共享内存与文件系统相对独立，共享内存有自己的标识符生成机制，这使得共享内存与文件系统的兼容性很差，管道可以直接像文件那样被操作，但共享内存不行，这会给使用者增加多余的认知负担，这也是没人用共享内存的原因之一。

共享内存的指令删除使用的是`ipcrm -m`，后面跟应用级标识符，不是`key`，这要注意。

```shell
[wind@starry-sky Shared Memory]$ ipcrm -m 0x46010004
ipcrm: failed to parse argument: '0x46010004'
[wind@starry-sky Shared Memory]$ ipcrm -m 0
[wind@starry-sky Shared Memory]$ ipcs -m

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

[wind@starry-sky Shared Memory]$
```

在删除之后，我们再次运行

```shell
[wind@starry-sky Shared Memory]$ make clean ; make
In file included from processa.cc:1:
shmm.hpp:15:11: warning: built-in function ‘log’ declared as non-function [-Wbuiltin-declaration-mismatch]
   15 | wind::Log log;
      |           ^~~
In file included from processb.cc:1:
shmm.hpp:15:11: warning: built-in function ‘log’ declared as non-function [-Wbuiltin-declaration-mismatch]
   15 | wind::Log log;
      |           ^~~
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-14 20:32:19]::succeeded ftok, the key is 0x46010004
[Info][2024-12-14 20:32:19]::create share memory success, the shmid is 1
[Info][2024-12-14 20:32:24]::a quit...
[wind@starry-sky Shared Memory]$ ipcs -m

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 1          wind       0          4096       0                       

```

接下来我们解决一下权限问题，在上面的实验中，我们没有设置创建权限，所以这里的`perms`是0，另外还可以看一下`nattch`，它表示当前与共享内存建立联系的进程个数，我们上面只是单纯创建了共享内存，但并没有让这个共享内存映射到进程地址空间，所以`nattch`是0。

权限写在`shmget`的第三个参数上

```cpp
int n = shmget(getKey(), size, IPC_CREAT | IPC_EXCL | 0666);
```

重新运行

```shell
[wind@starry-sky Shared Memory]$ make clean ; make
In file included from processa.cc:1:
shmm.hpp:15:11: warning: built-in function ‘log’ declared as non-function [-Wbuiltin-declaration-mismatch]
   15 | wind::Log log;
      |           ^~~
In file included from processb.cc:1:
shmm.hpp:15:11: warning: built-in function ‘log’ declared as non-function [-Wbuiltin-declaration-mismatch]
   15 | wind::Log log;
      |           ^~~
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-14 20:42:41]::succeeded ftok, the key is 0x46010004
[Info][2024-12-14 20:42:41]::create share memory success, the shmid is 2
[Info][2024-12-14 20:42:46]::a quit...
[wind@starry-sky Shared Memory]$ ipcs -m

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 2          wind       666        4096       0                       

[wind@starry-sky Shared Memory]$
```

接下来我们看看`bytes`，也就是共享内存的大小，需要注意的是，共享内存是以页框为单位进行分配的，在上面的代码中，我们使用的就是页框的实际大小，4096字节，如果设置为4097字节，虽然用户实际能支配的共享内存确实是4097字节，但系统实际上为这个共享内存分配了两个页框，只不过第二个页框只用一字节，所以我们建立，共享内存的大小应该是页框大小的整数倍。

接下来我们看看共享内存怎么映射到进程地址空间上。为此，我们需要使用`shmat`接口

![image-20241214205621021](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214205621113.png)

虽说`shmat`有三个参数，但实际上我们只要关心第一个，`shmid`自然不必多说，就是应用标识符，第二个参数是用于规定共享内存在共享区映射的具体起始地址，但进程地址空间的具体区域划分范围用户是不知道的，所以一般我们都设置为空，表示让系统自己决定具体映射到哪，第三个是进程级别权限，比如可以把一个共享内存以只读方式映射，这样，这个进程就只能读共享内存，而不能写。不过一般上，我们都直接设置为0，表示共享内存自己是什么权限。就以什么权限映射，它的返回值是映射区域的起始地址。这个接口用起来有点像`malloc`。

接下来我们重构代码，用C++的方式写

```cpp
#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string>
#include <unordered_map>
#include "log.hpp"

namespace wind
{
    // 与共享内存有关的相关常量
    enum ShmGetMode
    {
        PreferExisting = IPC_CREAT,             // 表示优先复用已有资源
        ForceNew = IPC_CREAT | IPC_EXCL | 0666, // 强制创建新的共享内存
        size = 4096,
        proj_id = 0x1246,
        pathname
    };

    // 为无法在枚举中映射的成员建立映射关系
    const std::unordered_map<ShmGetMode, const char *> ShmGetModeToCString =
        {
            {pathname, "/home/wind"}};

    class channel
    {
        enum error
        {
            ftok_error = 1,
            shmget_error = 2,
            attach_error
        };

        const char *toCStrung(ShmGetMode mode)
        {
            Log log;
            auto it = ShmGetModeToCString.find(mode);
            if(it == ShmGetModeToCString.end())
            {
                log(Fatal, "invalid mapping!");
                return "Unknown";
            }
            else
            {
                return it->second;
            }
        }

    public:
        channel(ShmGetMode Mode, ShmGetMode Size = size, ShmGetMode Path = pathname, ShmGetMode KeyHint = proj_id)
            : _ptr(nullptr), _size(Size)
        {
            key_t key = getKey(Path, KeyHint);
            sleep(2);
            int id = getShmid(key, Size, Mode);
            sleep(2);
            _ptr = getShmAttach(id);
            sleep(5);
        }

    private:
        key_t getKey(ShmGetMode Path, ShmGetMode KeyHint)
        {
            int n = ftok(toCStrung(Path), KeyHint);
            if (n < 0)
            {
                log(Fatal, "filed ftok:%s", strerror(errno));
                exit(ftok_error);
            }
            else
            {
                log(Info, "succeeded ftok, the key is 0x%x", n);
                return n;
            }
        }

        int getShmid(key_t key, ShmGetMode Size, ShmGetMode Mode)
        {
            int n = shmget(key, Size, Mode);
            if (n == -1)
            {
                log(Fatal, "create share memory error:%s", strerror(errno));
                exit(shmget_error);
            }
            else
            {
                log(Info, "create share memory success, the shmid is %d", n);
                return n;
            }
        }

        char* getShmAttach(int id)
        {
            char* ptr = (char*)shmat(id, nullptr, 0);
            if(ptr == (void*)-1)
            {
                log(Fatal, "shared memory attach failed:%s", strerror(errno));
                exit(attach_error);
            }
            else
            {
                log(Info, "shared memory attach success, the mapped address is %p", ptr);
                return ptr;
            }
        }

        Log log;
        char *_ptr;   // 共享内存的起始地址
        size_t _size; // 共享内存的大小
    };
}

// processa.cc
#include "shmm.hpp"

using namespace wind;

int main()
{
    Log log;
    channel shmm(ForceNew);
    log(Info, "a quit...");

    return 0;
}
```

```shell
[wind@starry-sky Shared Memory]$ make clean; make
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-15 9:45:11]::succeeded ftok, the key is 0x46010004
[Info][2024-12-15 9:45:13]::create share memory success, the shmid is 3
[Info][2024-12-15 9:45:15]::shared memory attach success, the mapped address is 0x7f890a5a4000
[Info][2024-12-15 9:45:20]::a quit...
[wind@starry-sky Shared Memory]$
```

```shell
[wind@starry-sky Shared Memory]$ while :; do ipcs -m | head -3 ; ipcs -m | grep wind; sleep 1; done

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 3          wind       666        4096       0                       
^C
[wind@starry-sky Shared Memory]$
```

最开始共享内存没有被创建，所以搜索不到，之后进程创建了共享内存，所以就有了，但由于进程没有把共享内存映射到进程地址空间中，所以`number-attch`的数字为0，然后`attch`（附加）上去了，所以`nattch`数字加一，最后进程退出，进程地址空间消失，因此`nattch`又减一。

除了进程退出能取消映射关系之外，也可以采用`shmdt`接口，`shmat`中的`at`是`attch`，意为附加，而`shmdt`中的`dt`是`detach`，意为分离。

![image-20241214205621021](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214205621113.png)

`shmdt`的使用方法就像是`free`，里面的参数`shmaddr`就是`shmat`返回的地址

```cpp
~channel()
{
    int n = shmdt(_ptr);
    if(n == -1)
    {
        log(Error, "failed to detach shared memory:%s", strerror(errno));
    }
    else
    {
        log(Info, "shared memory detached successfully");
    }
}
```

```cpp
void f()
{
    Log log;
    channel shmm(ForceNew);
    log(Info, "f done...");
}

int main()
{
    Log log;
    f();
    sleep(4);
    log(Info, "a quit...");
    return 0;
}
```

```shell
[wind@starry-sky Shared Memory]$ ipcrm -m 3
[wind@starry-sky Shared Memory]$ make clean ; make
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-15 10:25:11]::succeeded ftok, the key is 0x46010004
[Info][2024-12-15 10:25:13]::create share memory success, the shmid is 4
[Info][2024-12-15 10:25:15]::shared memory attach success, the mapped address is 0x7f3abf3f4000
[Info][2024-12-15 10:25:20]::f done...
[Info][2024-12-15 10:25:20]::shared memory detached successfully
[Info][2024-12-15 10:25:24]::a quit...
[wind@starry-sky Shared Memory]$
```

```shell
[wind@starry-sky Shared Memory]$ while :; do ipcs -m | head -3 ; ipcs -m | grep wind; sleep 1; done

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 4          wind       666        4096       0                       
^C
[wind@starry-sky Shared Memory]$
```

最后再说一个接口`shmctl`，它含有多种功能：获取共享内存段的状态，设置共享内存段的控制参数，删除共享内存段

![image-20241215103128729](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241215103128871.png)

它的第二个参数`cmd`用于描述进行何种操作，比如`IPC_STAT`，用于获取共享内存的状态，通过第三个参数`buf`进行输出，`IPC_SET`用于对共享内存进行设置，很少用，`IPC_RMID`用于删除物理层面的共享内存，此时第三个参数没用，可以直接设为空，第一个参数就不多说了，它是共享内存标识符，不过我们上面的类中似乎记录标识符的成员，所以要再加一个`int`成员。

```cpp
enum ShmGetMode
{
    PreferExisting = IPC_CREAT,             // 表示优先复用已有资源
    ForceNew = IPC_CREAT | IPC_EXCL | 0666, // 强制创建新的共享内存
    size = 4096,
    proj_id = 0x1246,
    remove = IPC_RMID,
    statu = IPC_STAT,
    pathname
};

~channel()
{
    if (_mode == ForceNew)
    {
        int n = shmdt(_ptr);
        if (n == -1)
        {
            log(Error, "failed to detach shared memory:%s", strerror(errno));
        }
        else
        {
            log(Info, "shared memory detached successfully");
        }
        n = shmctl(_id, remove, nullptr);
        if (n == -1)
        {
            log(Error, "failed to delete shared memory:%s", strerror(errno));
        }
        else
        {
            log(Info, "delete shared memory successfully");
        }
    }
}

Log log;
ShmGetMode _mode;
int _id;      // 共享内存标识符
char *_ptr;   // 共享内存的起始地址
size_t _size; // 共享内存的大小
```

```shell
[wind@starry-sky Shared Memory]$ ipcrm -m 4
[wind@starry-sky Shared Memory]$ make clean ; make
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-15 10:52:14]::succeeded ftok, the key is 0x46010004
[Info][2024-12-15 10:52:16]::create share memory success, the shmid is 5
[Info][2024-12-15 10:52:18]::shared memory attach success, the mapped address is 0x7f3d9420f000
[Info][2024-12-15 10:52:23]::f done...
[Info][2024-12-15 10:52:23]::shared memory detached successfully
[Info][2024-12-15 10:52:23]::delete shared memory successfully
[Info][2024-12-15 10:52:27]::a quit...
[wind@starry-sky Shared Memory]$
```

```shell
[wind@starry-sky Shared Memory]$ while :; do ipcs -m | head -3 ; ipcs -m | grep wind; sleep 1; done

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 5          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 5          wind       666        4096       0                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 5          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 5          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 5          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 5          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 5          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
^C
[wind@starry-sky Shared Memory]$
```

现在我们把信道建立，看看效果

```cpp
#include "shmm.hpp"

using namespace wind;

int main()
{
    Log log;
    channel shmm(ForceNew);
    sleep(6);
    log(Info, "a quit...");
    return 0;
}
```

```cpp
#include "shmm.hpp"

using namespace wind;

int main()
{
    Log log;
    channel shmm(PreferExisting);
    sleep(3);
    log(Info, "b quit...");
    return 0;
}
```

此处去除了其它地方的`sleep`

```shell
[wind@starry-sky Shared Memory]$ make clean ; make
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-15 11:12:47]::succeeded ftok, the key is 0x46010004
[Info][2024-12-15 11:12:47]::create share memory success, the shmid is 6
[Info][2024-12-15 11:12:47]::shared memory attach success, the mapped address is 0x7f8937a83000
[Info][2024-12-15 11:12:53]::a quit...
[Info][2024-12-15 11:12:53]::shared memory detached successfully
[Info][2024-12-15 11:12:53]::delete shared memory successfully
[wind@starry-sky Shared Memory]$
```

```shell
[wind@starry-sky Shared Memory]$ ./b
[Info][2024-12-15 11:12:49]::succeeded ftok, the key is 0x46010004
[Info][2024-12-15 11:12:49]::create share memory success, the shmid is 6
[Info][2024-12-15 11:12:49]::shared memory attach success, the mapped address is 0x7fb5ae62d000
[Info][2024-12-15 11:12:52]::b quit...
[wind@starry-sky Shared Memory]$
```

```shell
[wind@starry-sky Shared Memory]$ while :; do ipcs -m | head -3 ; ipcs -m | grep wind; sleep 1; done

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 6          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 6          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 6          wind       666        4096       2                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 6          wind       666        4096       2                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 6          wind       666        4096       2                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x46010004 6          wind       666        4096       1                       

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
^C
[wind@starry-sky Shared Memory]$
```

接下来就是要正式通信了，为了简化通信过程，这里我们约定双方只进行字符串交流。并规定，a作为读端，b作为写端。

通过共享内存进行读写不需要任何接口，直接在内存块里读写即可。

```cpp
const char* read()
{
    return _ptr;
}

ssize_t write(const char* str)
{
    size_t len = strlen(str) + 1;
    if(len > _size)
    {
        log(Error, "too much data");
        memcpy(_ptr, str, _size);
        return _size;
    }
    else
    {
        memcpy(_ptr, str, len);
        return len;
    }
}

// processa.cc
#include "shmm.hpp"
#include<iostream>

using namespace wind;
using namespace std;

int main()
{
    Log log;
    channel shmm(ForceNew);
    sleep(3);
    while(1)
    {
        cout<<"Message received#"<<shmm.read()<<endl;
        sleep(1);
    }
    log(Info, "a quit...");
    return 0;
}

// processb.cc
#include "shmm.hpp"
#include<iostream>

using namespace wind;
using namespace std;

int main()
{
    Log log;
    channel shmm(PreferExisting);
    while(1)
    {
        string buf;
        cout<<"Please enter#";
        cin >> buf;
        shmm.write(buf.c_str());
        sleep(1);
    }
    log(Info, "b quit...");
    return 0;
}
```

```shell
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-15 11:54:30]::succeeded ftok, the key is 0x46010004
[Info][2024-12-15 11:54:30]::create share memory success, the shmid is 7
[Info][2024-12-15 11:54:30]::shared memory attach success, the mapped address is 0x7f60282e4000
Message received#
Message received#
Message received#
Message received#
Message received#
Message received#aaaaaaaaaaaaaaaa
Message received#aaaaaaaaaaaaaaaa
Message received#aaaaaaaaaaaaaaaa
Message received#aaaaaaaaaaaaaaaa
Message received#bbbbbbbbbbbbbbbbbbbb
Message received#bbbbbbbbbbbbbbbbbbbb
Message received#bbbbbbbbbbbbbbbbbbbb
Message received#ccccccccccccccccccccccccc
Message received#ccccccccccccccccccccccccc
Message received#ccccccccccccccccccccccccc
Message received#dddddddddddddddddddddddddddddd
Message received#dddddddddddddddddddddddddddddd
Message received#dddddddddddddddddddddddddddddd
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#eeeeeeeeeeeee
Message received#aaaaaaaaaaa
Message received#aaaaaaaaaaa
Message received#aaaaaaaaaaa
^C
[wind@starry-sky Shared Memory]$
```

```shell
[wind@starry-sky Shared Memory]$ ./b
[Info][2024-12-15 11:54:32]::succeeded ftok, the key is 0x46010004
[Info][2024-12-15 11:54:32]::create share memory success, the shmid is 7
[Info][2024-12-15 11:54:32]::shared memory attach success, the mapped address is 0x7f4a04948000
Please enter#aaaaaaaaaaaaaaaa
Please enter#bbbbbbbbbbbbbbbbbbbb
Please enter#ccccccccccccccccccccccccc
Please enter#dddddddddddddddddddddddddddddd
Please enter#eeeeeeeeeeeee
Please enter#aaaaaaaaaaa^H^H^H
Please enter#^C
[wind@starry-sky Shared Memory]$ 
```

不太好看，应该`#`后面加个空格的。这不重要，关键我们可以看到共享内存完全没有管道的那种同步互斥的保护机制，所以读端一直在读共享内存的内容，不会出现阻塞的状态，这意味着，共享内存直接使用很危险，数据容易被破坏。

不过，共享内存的读写效率很高，这是因为它是直接映射，所以拷贝的次数较少，所以读写效率很高，给人的感觉就像是堆空间。如果对效率有很高要求，可以直接把`_ptr`公开，直接写在上面，不经过`str`这一中间层，这样就又能减少一次拷贝了。

接下来我们获取一下共享内存的状态

```cpp
struct shmid_ds getStatu()
{
    struct shmid_ds buf;
    int n = shmctl(_id, statu, &buf);
    return buf;
}
```

```cpp
int main()
{
    Log log;
    channel shmm(ForceNew);

    struct shmid_ds shmds = shmm.getStatu();
    cout<<"shm size:"<<shmds.shm_segsz<<endl;
    cout<<"shm nattch:"<<shmds.shm_nattch<<endl;
    printf("shm key:%x\n", shmds.shm_perm.__key);
    cout<<"shm mode:"<<shmds.shm_perm.mode<<endl;
   
    log(Info, "a quit...");
    return 0;
}
```

```shell
[wind@starry-sky Shared Memory]$ make clean ; make
[wind@starry-sky Shared Memory]$ ./a
[Info][2024-12-15 12:41:43]::succeeded ftok, the key is 0x46010004
[Info][2024-12-15 12:41:43]::create share memory success, the shmid is 8
[Info][2024-12-15 12:41:43]::shared memory attach success, the mapped address is 0x7f18f97a4000
shm size:4096
shm nattch:1
shm key:46010004
shm mode:438
[Info][2024-12-15 12:41:43]::a quit...
[Info][2024-12-15 12:41:43]::shared memory detached successfully
[Info][2024-12-15 12:41:43]::delete shared memory successfully
[wind@starry-sky Shared Memory]$
```

这个`mode`需要转换。

## expand

下面我们要对共享内存实现同步互斥的保护功能，为了尽可能完善共享内存的读写机制，我们将再一次对代码进行大的修改。

共享内存的同步互斥要靠其它通信方式来实现，比如管道，我们利用管道的保护机制，在使用共享内存发送消息前先用管道通信一下，这样读端就能从管道中读到信息，从而进行一次共享内存读取，而在其它时候，由于读端从管道读不到信息，就会一直阻塞在管道读端。

```cpp
// shmm.hpp
#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include"comm.h"

#define PATHNAME "/home/wind"
#define KEYHINT 0x1246
#define SHMM_SIZE 4096
#define SHMM_Mode 0666

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

// a.cc
#include"shmm.hpp"

int main()
{
    // 创建并映射共享地址
    key_t key = ftok(PATHNAME, KEYHINT);
    int md = shmget(key, SHMM_SIZE, IPC_CREAT | IPC_EXCL | SHMM_Mode);
    char* ptr = (char*)shmat(md, nullptr, 0);

    mkfifo(FIFO_NAME, MODE);            // 创建命名管道
    int fd = open(FIFO_NAME, O_RDONLY); // 打开管道读端

    // 通信
    while(1)
    {
        char c;
        ssize_t n = read(fd, &c, sizeof(char));
        if(n == 0 || n < 0) break;
        printf("Message received: %s", ptr);
    }

    close(fd);  // 关闭读端
    shmdt(ptr); // 解除映射关系
    unlink(FIFO_NAME);             // 删除管道文件
    shmctl(md, IPC_RMID, nullptr); // 删除共享内存

    return 0;
}

// b.cc
#include"shmm.hpp"

int main()
{
    // 创建并映射共享地址
    key_t key = ftok(PATHNAME, KEYHINT);
    int md = shmget(key, SHMM_SIZE, IPC_CREAT);
    char* ptr = (char*)shmat(md, nullptr, 0);

    int fd = open(FIFO_NAME, O_WRONLY); // 打开管道写端

    // 通信
    while(1)
    {
        printf("Please enter: ");
        fgets(ptr, SHMM_SIZE, stdin);
        write(fd, "c", sizeof(char));
    }

    close(fd);  // 关闭写端
    shmdt(ptr); // 解除映射关系

    return 0;
}
```

```shell
[wind@starry-sky sync_mutex]$ ls
a.cc  b.cc  comm.h  shmm.hpp
[wind@starry-sky sync_mutex]$ g++ a.cc -o a
[wind@starry-sky sync_mutex]$ g++ b.cc -o b
```

```shell
[wind@starry-sky sync_mutex]$ ./a
Message received: heaop
Message received: xjahnxax
Message received: xjAXJU
Message received: jxnjsanx
Message received: 花销小农举行阿萨辛
Message received: kjoncsjncj
Message received: aaaaaaaaaaaaaaa
Message received: kxsax
Message received: ckijdsiocjw
Message received: xkjsajxio  kioddioa
Message received: xkosaiaociasiooqxko j ij ioijaixaxcokaco 
[wind@starry-sky sync_mutex]$
```

```shell
[wind@starry-sky sync_mutex]$ ./b
Please enter: heaop
Please enter: xjahnxax
Please enter: xjAXJU
Please enter: jxnjsanx
Please enter: 花销小农举行阿萨辛
Please enter: kjoncsjncj
Please enter: aaaaaaaaaaaaaaa
Please enter: kxsax
Please enter: ckijdsiocjw
Please enter: xkjsajxio  kioddioa
Please enter: xkosaiaociasiooqxko j ij ioijaixaxcokaco 
Please enter: ^C
[wind@starry-sky sync_mutex]$
```

这样就可以让共享内存有同步互斥的效果。

# end
