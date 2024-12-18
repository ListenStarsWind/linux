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

# others

接下来我们谈谈消息队列和信号量。消息队列我们只略微提一下，信号量在这里也是略微提一下，在线程阶段我们再深入探讨。为什么要把共享内存，消息队列，信号量放在同一个文章里说呢？因为它们都是基于`System V`标准的进程通信方案，所以在使用接口，内核形式都具有很多相似点。

让我们先回顾一下共享内存的各种接口

用于获取内核标识符的`ftok`(file to key)，将一个文件转化为唯一的关键字，文件是Linux中的稳定实体，它本身就具有唯一性，以它为担保生成的键值，是一个稳定且唯一的键值。用该键值就可以区分各种共享资源。
![image-20241214172114632](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214172114739.png)

用于创建共享内存的`shmget`，它返回一个用户级标识符
![image-20241214163431072](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214163431155.png)

用于把共享内存附加到进程地址空间中`shmat`
![image-20241214205621021](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214205621113.png)

用于对共享内存进行控制的`shmctl`
![image-20241215103128729](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241215103128871.png)

共享内存面向用户的内核数据结构是
```cpp
struct shmid_ds {
    struct ipc_perm shm_perm;    /* Ownership and permissions */
    size_t          shm_segsz;   /* Size of segment (bytes) */
    time_t          shm_atime;   /* Last attach time */
    time_t          shm_dtime;   /* Last detach time */
    time_t          shm_ctime;   /* Last change time */
    pid_t           shm_cpid;    /* PID of creator */
    pid_t           shm_lpid;    /* PID of last shmat(2)/shmdt(2) */
    shmatt_t        shm_nattch;  /* No. of current attaches */
    ...
};


struct ipc_perm {
    key_t          __key;    /* Key supplied to shmget(2) */
    uid_t          uid;      /* Effective UID of owner */
    gid_t          gid;      /* Effective GID of owner */
    uid_t          cuid;     /* Effective UID of creator */
    gid_t          cgid;     /* Effective GID of creator */
    unsigned short mode;     /* Permissions + SHM_DEST and
                                           SHM_LOCKED flags */
    unsigned short __seq;    /* Sequence number */
};

```

接下来我们说说消息队列

消息队列，其实就是系统创建的队列，这个队列独立于进程存在，所以可以让进程间进行通信，需要注意的是，消息队列是双向的，每个进程既有写端，也有读端。进程之间通过某些约定就可以直接以数据块为单位进行通信。

为了便于数据块类型的识别，消息队列的通信需要遵循一定的规则，它要求每个数据块以

```cpp
struct msgbuf {
    long mtype;       /* message type, must be > 0 */
    char mtext[1];    /* message data */
};
```

的形式进行通信。其中第一个成员`mtype`是一个`long`类型，用于区别传输的数据类型，在通信前，可以通过诸如宏，枚举的方式，将参与通信的数据类型进行一一映射，这样就可以根据`mtype`区分实际传送数据的类别；第二个成员`mtext[1]`是一个变长数组，用于存储真正的传送数据，这个数组的一个元素是用来占位的，更标准的写法应该是`char mtext[]`。

比如现在我们有一个`mytype`的类型，并用这个类型创建了一个变量，那么就可以这样生成数据块`msgbuf`

```cpp
// 伪代码 仅作思路展示

// 枚举其实并不好
// 因为枚举值是int类型
//enum types
//{
//    mytype,
//    ...
//};

#define MYTYPE 1

// 需要用户自己定义
struct msgbuf {
    long mtype;       
    char mtext[];     
};

mytype a;

struct msgbuf* msg = (struct msgbuf*)malloc(sizeof(long) + sizeof(mytype));
// 写成这样也行
// void* msg = malloc(sizeof(long) + sizeof(mytype));

msg->mtype = MYTYPE;

// mtext是数组名,也就是首元素地址  不能赋值,赋值是非法的
memcpy(msg->mtext, &a, sizeof(mytype));  
```

消息队列同样要有内核级标识符，同样是用`ftok`获得
![image-20241214172114632](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214172114739.png)

消息队列的创建方式与共享内存相似，如果想要确保获得新的消息队列，需要使用`IPC_CREAT | IPC_EXCL`，如果想用现成的，就用`IPC_CREAT`
![image-20241217150230872](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241217150231086.png)

消息队列不是内存块，有专门的读写接口，发送消息是`msgsnd`，接收消息是`msgrcv`
![image-20241217150501728](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241217150501965.png)

消息队列也有对应的控制接口
![image-20241217150651663](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241217150651849.png)

其面向用户的内核数据结构为

```cpp
struct msqid_ds {
    struct ipc_perm msg_perm;     /* Ownership and permissions */
    time_t          msg_stime;    /* Time of last msgsnd(2) */
    time_t          msg_rtime;    /* Time of last msgrcv(2) */
    time_t          msg_ctime;    /* Time of last change */
    unsigned long   __msg_cbytes; /* Current number of bytes in
                                                queue (nonstandard) */
    msgqnum_t       msg_qnum;     /* Current number of messages
                                                in queue */
    msglen_t        msg_qbytes;   /* Maximum number of bytes
                                                allowed in queue */
    pid_t           msg_lspid;    /* PID of last msgsnd(2) */
    pid_t           msg_lrpid;    /* PID of last msgrcv(2) */
};

struct ipc_perm {
    key_t          __key;       /* Key supplied to msgget(2) */
    uid_t          uid;         /* Effective UID of owner */
    gid_t          gid;         /* Effective GID of owner */
    uid_t          cuid;        /* Effective UID of creator */
    gid_t          cgid;        /* Effective GID of creator */
    unsigned short mode;        /* Permissions */
    unsigned short __seq;       /* Sequence number */
};

```

它也有对应的命令行操作(我的系统里没有消息队列)

```shell
[wind@starry-sky ~]$ ipcs -q

------ Message Queues --------
key        msqid      owner      perms      used-bytes   messages    

```

`ipcrm -q`同样可以删除对应`id`的消息队列

我们再瞅一眼信号量

![image-20241217151452157](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241217151452278.png)

![image-20241217151529536](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241217151529701.png)



我们发现，不管是共享内存，消息队列，信号量，其内核描述对象`struct nameid_ds`组织形式都是类似的，并且，`struct nameid_ds`的第一个成员，都是`struct ipc_perm`。实际上，系统中有一个专门用于进程间通信的`IPC`子系统，专门负责对于上述进程通信资源进行管理。下面我们就`IPC`子系统的工作逻辑进行基本的描述。

在上面的学习过程中，我提到了一个很奇怪的词“面向用户的内核结构”，为什么我要用这个词呢？因为一方面，这些结构体里的数据确实是共享资源的内核数据拷贝，但另一方面，系统自己不是真的使用这些结构进行资源管理的，它有自己的面向系统的内核结构，这两者在形式上并无太大差别，但某些细节的改变使得“面向系统的内核结构”更容易被系统管理。面向用户的内核结构是给人看的，面向系统的内核结构是系统真正进行管理的。

让我们把视角转回到`struct ipc_perm`

```cpp
struct ipc_perm {
    key_t          __key;       
    uid_t          uid;         
    gid_t          gid;         
    uid_t          cuid;        
    gid_t          cgid;        
    unsigned short mode;        
    unsigned short __seq;       
};
```

`IPC`子系统中有一个指针数组，数组中的每个元素都指向一个`struct ipc_perm`的对象，每当我们使用创建一个共享资源时，其`struct nameid_ds`中都有一个`struct ipc_perm`对象，指针数组指向的就是它们，而`nameget`系列接口返回的其实就是这个数组的下标，再加上我的系统之前没创建过共享资源，所以返回的`id`是从0开始，线性递增的。注意在系统中，真正使用的是面向系统结构体，面向系统和面向对象结体具有一定的区别，比如面向系统的`struct ipc_perm`中含有一个`types`字段，该字段表示`struct ipc_perm`到底处于何种`struct nameid_ds`中，比如如果某个`struct ipc_perm`中的`type`对象映射的是`struct msqid_ds`，就说明这是一个位于`struct msqid_ds`中的`struct ipc_perm`，所以就能通过`(struct msqid_ds*)ptr`的方式拿到`struct msqid_ds`的地址，有了`struct msqid_ds`的地址，自然可以访问`struct msqid_ds`的其它成员。

用C++的视角来说，可以认为`struct ipc_perm`是基类，而`struct nameid_ds`是派生类，这是一种C语言的多态，在数组里的，是基类指针，不是派生类指针。所以还是那句话，C++等面向对象语言的创造者，不是某天一拍脑袋，觉得多态好，才让自家语言有了多态特性，而是在大量的工程实践中，体会到多态特性的必要性，所以为自家语言加上了多态特性。

-----------

现在让我们把视角转回到信号量。本文的信号量全是概念，具体应用要等到线程阶段。

为了信号量不至于太抽象，先让我们设想一个场景：现在有两个进程`A`和`B`，它们之间为了进行通信，建立了共享内存，现在`A`往共享内存中写入一份数据，这份数据是一个整体，缺了就无法解析成功，由于共享内存不存在数据保护机制，所以就有可能出现这种情况：`A`还没把这份数据写完个，`B`就读了，这就会使得`B`得到一个残缺的数据，从而解析失败；又或者，`A`和`B`同时往共享内存中写，这样共享内存中的数据就会变得混乱，也会丧失原来的有效性。我们称这种问题叫做“数据不一致问题”。

对于共享内存这种完全没有数据保护的共享资源，就可以对其加锁，当一个进程访问共享资源时，这个资源就被锁上了，其它进程无法访问，直到进程访问结束，否则其它进程就会一直无法使用该共享资源，通过这种方式，就实现了对共享资源的同步互斥。

锁在生活中亦有体现，比如去自动取款机取钱时，只能一个一个人取，一个人使用ATM机时，那个小空间就会上锁，阻止其它人的使用请求。

此时，我们就可以为同步互斥下一个非常朴素的定义：任何时刻，只允许一个执行流对共享资源进行访问。我们称这种具有同步互斥特性的共享资源称之为临界资源，比如管道就是一种临界资源，一是它可以被多个执行流访问，所以是共享资源，二是，它不允许多个执行流同时访问管道，所以它是临界资源。一般来说，临界资源这个概念是对内存空间说的。

接下来我们想想让程序运行为什么要写代码，原因很简单，程序就是照着代码运行的，我们的代码实际上就是在描述程序运行的行为，程序之所以访问某个临界资源，是因为代码就这么写的，我们把让程序访问临界资源的那些代码称之为临界区。

接下来我们说一个现象：当我们使用多进程或者多线程并发往屏幕上打印信息时，会发现它们是混着打印的，有时候`bash`也会来插一脚，我们为什么能在屏幕上打印信息？因为进程打开了屏幕所对应的文件，于是屏幕文件就变成了共享资源，但由于我们未对屏幕文件进行保护，所以导致信息混合打印。

在有了上面的铺垫之后，我们来理解一下信号量。信号量的本质其实就是一个计数器，也就是`int count`，这个计数器的值表示一份临界资源中还剩几块临界资源可以被使用。就像内存或者文件系统一样，一个临界资源其内部也是会被分成多个块的，比如磁盘有分区，分区有各种块，而`Date blocks`里面还有一个个`Date block`。

![image-20241120194848049](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201948217.png)

一个临界资源也是如此，一个临界资源可以被细分成若干个块，就像一个电影院里面有多个座位，每个块在在某个时刻都只能被一个执行流访问，就像一个座位只能坐一个人一样，要么没执行流访问这个块，要访问，只能有一个执行流访问该块。

并且，块也是可以被预定的，电影院里座位的使用权是从买过票时候开始的，即使现在这个座位还没人坐，那也不能给别人，因为已经有人预定过了。电影院的座位是有限的，我们可以用一个计数器描述当前还空闲的座位个数，每卖出一张票，计数器就要减一，当计数器减到0了，就说明没座位了，所以就不卖票了，其它人想用，那要先等着。

现在，计算机里面有一份很大的资源，它可以被划分成多个小块，单个小块对于一个执行流来说是够用的，所以可以拆分，这样就可以提高多个执行流对该份临界资源的访问效率，从外面看上去，就像是多个执行流同时访问这份临界资源，但实际上，从内部来看，一个执行流只用一个小块，各个小块之间互不干扰。

很明显，我们需要对这些小块进行管理，比如我们可以创建一个整型变量，这个变量刚开始记录了这份临界资源中的所有小块个数。每当有一个执行流过来想要申请资源时，临界资源就会找一份没人用的空闲块分配给这个执行流，并且让变量减去一，当变量归0了，就说明现在没有空闲的块了，如果又有执行流申请资源，就让它等着，而当一个执行流不在需要资源时，它所使用的块就重新变为了空闲状态，并且让变量加一。

也就是说，这个变量描述的是这份临界资源还有多少未被预定的块。执行流对资源的申请，就类似于买票这种行为。这个计数器的作用就是传递一种信号——当前这份临界资源还有多少个块可被申请，所以被叫做`Semaphore`，其原义指的是一种用于远距离传递信息的信号装置或方法，尤其是指通过可视信号来传递信息的设备或系统，如旗语、灯光信号等，所以有些人把它翻译成信号灯，而`Semaphore`又来源于希腊语的**"sēma"**（标志、信号）和 **"phoros"**（携带、传送），更多人叫做“信号量”，量表示剩下块的数量，或者也可能表示它是一个变量，一个计数器。

当这份临界资源只有一个块时，一旦某个执行流申请到了这个块，看上去就像这份临界资源被改执行流独享，相当于该执行流为这份临界资源上了锁，其它执行流不能用，所以只有`0 or 1`两种状态的信号量被称为二元信号量，又被称为锁。

为什么这份临界资源只有一个块呢？因为这份临界资源太小了，分成小块不够一个执行流用，当然也不排除其它的特殊目的。

现在我们来看信号量的两个关键操作，加加和减减。

信号量作为内存中的一个数据，也可以被视为一份资源，虽说它身为系统内核数据，进程不能直接对其进行加加减减，但它们对于其它临界资源的申请，必然也会引发信号量的变化，所以信号量也可被视为一种共享资源，信号量自己会不会在加加减减的过程中因为多执行流并发访问而出错呢？这完全是有可能的。

一般来说，加加减减操作都会被分解成若干条汇编指令，一般来说，是三种，首先把变量拷贝到CPU的寄存器上，然后对这个寄存器的内容进行加加减减，之后再把它拷回去，这样就可能会出现一些问题。

现有两个执行流，其序号分别为`1`和`2`，`1`像让`i`加一，`2`想让`i`减一，如果加加减减操作是三条汇编的话，可能就会出现下面表格的情况，X表示寄存器，其上标表示对应哪个执行流。

| 执行次序 | 执行指令            | 执行后变量值                  | 执行流序号 |
| :------- | ------------------- | ----------------------------- | ---------- |
| 1        | i = 1               | i = 1, X<sup>[1]</sup> = 未知 | 1          |
| 2        | X<sup>[1]</sup> = i | i = 1, X<sup>[1]</sup> = 1    | 1          |
| 3        | X<sup>[2]</sup> = i | i = 1, X<sup>[2]</sup> = 1    | 2          |
| 4        | X<sup>[1]</sup>++   | i = 1, X<sup>[1]</sup> = 2    | 1          |
| 5        | X<sup>[2]</sup>--   | i = 1, X<sup>[2]</sup> = 0    | 2          |
| 6        | i = X<sup>[1]</sup> | i = 2, X<sup>[1]</sup> = 2    | 1          |
| 7        | i = X<sup>[2]</sup> | i = 0, X<sup>[2]</sup> = 0    | 2          |

为此，我们需要对信号量进行特别处理，使得其在加加减减的过程中是原子性的，什么叫原子性？就是说这个操作是不能被分割成更小的子操作的，要么这个操作干脆不做，要做就一口气做完，不存在正在做这种状态，对于这种场景，要么信号量不加加减减，要加加减减就一口气做完，它凭什么一口气做完，因为这个操作对应的汇编指令只有一条，可能还有一些其它方案，但我们先不考虑。

下面我们过一下信号量的相关接口

首先是获取信号量，其中`nsems`表示申请的信号量个数，也就是说`semget`可以一次性申请多个信号量，不过这里我们只是稍微过一下，所以就只申请一个信号量，如果申请多个，这些信号量实际上是放在一个数组中进行管理的，因此`nsems`应设为1，`semflg`表示创建方式

```c
int semget(key_t key, int nsems, int semflg);
```

`semctl`负责控制信号量，其中一种操作就是信号量的删除，其中`semnum`表示被删除信号量的下标，如果只有一个信号量，那这个参数就是0，`cmd`表示具体操作，可变参数用于传递操作中需要的内核结构，因为它包含信号量的设置功能，因此也可以对信号量进行初始化，设置最开始的值

```c
int semctl(int semid, int semnum, int cmd, ...);
```

信号量的加加减减有专门的接口，其中`struct sembuf`是包含三个成员的结构体，当前我们只需要知道前两个，第一个是`unsigned short`类型，表示数组下标，第二个是`short`类型，表示操作方式，是1就加加，是-1就减减。

信号量虽然没有直接在多执行流中进行数据传送，但是它确实传递了信息，从而有利于多执行流之间的协同配合，也起到了对其它共享资源进行保护的作用，或者说，信号量在通信中更多是发挥管理的职责，而不是单纯做数据的传送，因此是通信中不可分割的一部分，所以归类于通信范畴。

# end
