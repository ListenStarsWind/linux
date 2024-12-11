# log

## initial encounter

我们或许对“日志”这个词有所耳闻，今天我们将初步了解日志，并实现一个简单的日志系统，随着未来学习的深入，逐步丰富和扩展它。

日志的核心功能是记录异常情况。虽然行业内并没有统一的标准，但通常日志包括时间、等级和内容等信息。在程序运行过程中，可能会遇到各种异常，日志的等级可以根据异常的严重程度进行划分。具体分类如下：

- **Info**：常规信息
- **Warning**：警告信息
- **Error**：较严重的错误，程序可能仍能继续运行，但需要尽快处理
- **Fatal**：致命错误，程序无法继续运行
- **Debug**：调试信息

## time

在使用时间戳获取时间之前，先让我们看看C语言中的可变参数。由于日志可能需要输入各种格式的信息，所以我们需要可变参数来实现格式化控制，就像`printf`那样。

为了维持可变函数的栈帧结构，C语言对可变参数提供了一系列的宏，用于实现可变参数的传参机制，一般情况下，无论是什么实参，在传参过程中都会实例化出一份副本，并被压栈，这份副本就是所谓的形参。

首先我们知道传参实例化是从右往左的，比如`func(a,b,c)`它是先把`c`传进去的，再把`b`传进去，最后再把`a`传进去，它是使用压栈的方式存储形参的。

![image-20241210172856887](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241210172856929.png)

对于可变参数函数来说，就有了一个问题，怎么找到所有的参数，为此，C语言提供了一系列的宏

![image-20241210172156683](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241210172156763.png)

首先我们看`va_list`，它是某个抽象的类型，可能是指针，也可能是某种结构体，这取决于具体的编译环境，不管它是什么，都描述了可变参数的状态，并索引着某个参数；通过`va_list`变量，再结合其它宏操作就可以解析出一个个参数。`va_start`是一个宏，该宏的第一个参数就是`va_list`类型，第二个就是最后一个被实例化的元素，该宏会通过一系列手段，对`va_list`对象进行初始化，以使其存储当前函数栈帧的可变参数状态，从而使得`va_list`变得有效，在经历过`va_start`后，`va_list`实际上就索引第二个形参，也就是倒数第二个被实例化的参数，对于上图来说，实际上就是`b`；`va_arg`宏会解析出`va_list`对象当前索引的参数，并将`va_list`的索引指向下一个参数。如果从C++的角度来说，用户自已定义`va_list`对象和使用`va_start`的行为类似于构造函数，`va_arg`则相当于迭代器，依据参数实例化的先后顺序提供了遍历可变参数的方式，至于`va_end`，则负责让`va_list`对象失效，从功能上来看，类似析构。

为了更形象的描述上述宏的运作机理，我们可以假设`va_list`是个指针，只是假设，不是说它就是指针，实际上对目前的编译器来说，它应该是某种结构体。若`va_list`是个指针，则其类型为`char*`，`char*`的类型有助于它进行以字节为单位的指针运算，最开始，用户刚刚创建`va_list`对象的时候，它可能指向一处未知区域，或者就是空指针，此时它是一个野指针；宏`va_start`会对`va_list`对象进行初始化，其具体行为是把第一个形参，或者说最后一个被实例化的形参的地址传给`va_list`对象，并且再根据最后一个形参的具体类型对`va_list`对象进行迭代，使其索引下一个参数。

```cpp
// 比如现在有一个支持多个整型相加求和的函数int Sum(int n, ...)
// 第一个参数n负责描述有多少个数字参与求和，比如我们可以让四个变量
// int a = 1, b = 2, c = 3, d = 4参与求和，调用方式为
// Sum(4, a, b, c, d) // 注意这里的abcd都是实参
// 实参会通过实例化副本加压栈的方式从右往左进行传参
// 首先是d, 实参d会被拷贝一份副本d,然后进行压栈
// 这里需要注意，栈是从高地址往低地址进行延伸的，我们以前说过原因
// 然后是c,实参c实例化出形参c,并压栈
// 然后是d,实参d实例化出形参d,并压栈
// 然后是a,实参a实例化出形参a,并压栈
// 最后是4,实参4实例化出形参4,并压栈

// 注意，我这里假设va_list就是用指针实现的
// 用户刚定义时，va_list是野指针
// 宏va_start的逻辑如下
// 1.取第一个形参，也就是4的地址，赋给va_list
// 2.让va_list自加sizeof(4)，更准确地说，是va_list自加sizeof(int),因为第一个形参是int类型
// 由于栈是从高地址往低地址延伸的，所以第一个形参位于低地址处，va_start的自加操作就让其索引了下一个形参a
#define va_start(ap, last) \
    (ap = (va_list)(&last + sizeof(last))
```

`va_arg`会解析出当前`va_list`索引的形参，并将索引移到下一个参数

```cpp
// 还是要说明，这是va_list为指针的情况，当va_list不为指针时，其内部实现会与这里的说明有所不同，但功能都是一样的
// 宏va_arg的逻辑如下
// 1.解析出当前va_list索引的参数
// 2.让va_list向后迭代
#define va_arg(ap, type) \
    (*(type *)((ap += sizeof(type)) - sizeof(type)))
// ap += sizeof(type)让ap实现了自加
// 之后的- sizeof(type)则得到了指向（自加之后ap）上一个参数的指针
```

宏`va_end`并不重要，可能是把`va_list`直接置为空（如果`va_list`以指针方式实现）

```cpp
// log.hpp
#pragma once

#include<cstdarg>
#include<cstdio>

// 可变参数的示例:实现多个数的相加
// 第一个参数，n表示参与相加的数据个数
int example(int n, ...)
{
    va_list s;
    va_start(s, n);

    int sum = 0;
    while(n--)
    {
        sum += va_arg(s, int);
    }
    return sum;
}

// main.cc
#include"log.hpp"
#include<iostream>
using namespace std;

int main()
{
    cout << example(4, 1, 2, 3, 4) << endl;
    return 0;
}
```

```shell
[wind@starry-sky log]$ g++ main.cc
[wind@starry-sky log]$ ./a.out
10
[wind@starry-sky log]$
```

Linux中有多种获取时间的接口，比如系统接口`gettimeofday`

![image-20241210215535450](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241210215535576.png)

通过输出型参数`struct timeval *tv`便可以获知时间戳——从 1970年1月1日00:00:00 UTC 到当前时间所经过的 秒数，如果不足一秒，则以微秒方式展现，不过这种时间显然不是给人看的，我们要找个用人类语言描述的时间，此时我们就需要使用`localtime`

![image-20241210220350976](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241210220351063.png)

`localtime`可以将一个时间戳转化为`tm`结构体的接口，`tm`中包含年月日分秒等各类时间信息，它的时间戳参数可由`time`接口链式传参

![image-20241211080927185](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241211080927302.png)

```cpp
// 示例：时间的获取
void example()
{
    time_t t = time(nullptr);
    struct tm* ct = localtime(&t);

    printf("%d-%d-%d-%d-%d-%d\n", ct->tm_year + 1900, ct->tm_mon + 1, ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);
}
```

```shell
[wind@starry-sky log]$ ./a.out
2024-12-11-8-30-51
[wind@starry-sky log]$
```

要注意这个年是从`1900`开始的，也就是`1900`被映射为0，所以要加上`1900`，月是从`0`开始记的，所以要加上`1`。

接下来我们要对这些整型进行格式化控制，把它们都转换为字符串，这样才方便写信息。

格式控制是个繁琐但关键的操作，为此C语言有专门的格式控制接口，即下图中的`vsnprintf`接口

![image-20241210214911928](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241210214912027.png)

```cpp
#pragma once

#include<cstdarg>
#include<cstdio>
#include<ctime>
#include<string>
#include<cstdarg>

namespace wind
{
    // 日志等级
    enum level
    {
        Info = 1,
        Debug = 2,
        Warning = 3,
        Error = 4,
        Fatal = 5
    };

    const char* levelTostr(level le)
    {
        switch(le)
        {
            case Info : return "Info";
            case Debug : return "Debug";
            case Warning : return "Warning";
            case Error : return "Error";
            case Fatal : return "Fatal";
            default: return "None";
        }
    }

    class log
    {
        public:
        void operator()(level le, const char *format, ...)
        {
            // 日志固定部分的构建
            char fix[64] = {0};
            time_t t = time(nullptr);
            struct tm* ct = localtime(&t);
            snprintf(fix, sizeof(fix) - 1, "[%s][%d-%d-%d %d:%d:%d]", levelTostr(le), ct->tm_year + 1900, ct->tm_mon + 1, ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);

            // 构建用户输入的变化部分
            char dynamic[1024] = {0};
            va_list s;
            va_start(s, format);
            vsnprintf(dynamic, sizeof(dynamic) - 1, format, s);
            va_end(s);

            // 拼接上述两个部分
            char buffer[1088];
            snprintf(buffer, sizeof(buffer) - 1, "%s::%s\n", fix, dynamic);

            // 输出
            printf("%s", buffer);
        }
    };
}

// main.cc
int main()
{
    wind::log g;
    g(wind::Info, "hello");
    return 0;
}
```

```makefile
out:main.cc
	@g++ $^ -o $@
.PHONY:clean
clean:
	@rm -f out
```

```shell
[wind@starry-sky log]$ make
[wind@starry-sky log]$ ./out
[Info][2024-12-11 9:26:33]::hello
[wind@starry-sky log]$
```

## Implementation

现在我们可以实操了。我们将写一个日志类，该类对`()`进行了重载，对象被构造之后，可以直接以仿函数的形式进行调用。

为了方便进行管理，我们对类中的常量以枚举的形式进行管理，从而避开宏。日志的输出支持三种模式，第一种，输出到标准错误流，或者显示器上，第二种，创建一个文本文件，所有日志都会写到这个文件中，第三种，依据日志的具体等级，分别创建对应的文本文件，并依据具体日志等级输出到对应的文件中。

```cpp
#pragma once

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <string>
#include <cstdarg>
#include <vector>
#include <cassert>
#include<cstring>

// 面向系统
#ifdef __linux__

#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#define dMODE 0755 // 新建路径权限
#define fMODE 0666 // 新建文件权限

#endif

namespace wind
{
    // 日志等级
    // 枚举常量的值含有映射关系
    // 必须从0开始，并且连续递增
    enum level
    {
        Info = 0,
        Debug = 1,
        Warning = 2,
        Error = 3,
        Fatal = 4,
        count // 获知enum level中的有效成员个数
    };

    const char *levelTostr(level le)
    {
        switch (le)
        {
        case Info:
            return "Info";
        case Debug:
            return "Debug";
        case Warning:
            return "Warning";
        case Error:
            return "Error";
        case Fatal:
            return "Fatal";
        default:
            return "None";
        }
    }

    const char *levelTostr(int le)
    {
        switch (le)
        {
        case Info:
            return "Info";
        case Debug:
            return "Debug";
        case Warning:
            return "Warning";
        case Error:
            return "Error";
        case Fatal:
            return "Fatal";
        default:
            return "None";
        }
    }

    enum out_mode
    {
        display = 1,  // 所有信息都输出到显示屏幕上
        one_fife = 2, // 所有信息都输出到某个文件中
        category = 3  // 按照日志等级进行多文件分类输出
    };

    class log
    {
    public:
        // 实例化对象前不要关闭stderr，这会引发未知错误
        // flag用于指定日志的输出模式
        log(out_mode flag = display, std::string name = "log")
            : _name(name)
        {
#ifdef __linux__

            if (flag == one_fife || flag == category)
            {
                if (mkdir(_name.c_str(), dMODE) != 0);
                    // perror("Error");

                std::string s = "./";
                s += _name;
                s += "/log.txt";

                if (flag == one_fife)
                {
                    int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, fMODE);
                    if (fd < 0)
                        perror("Error");
                    _fd.push_back(fd);
                }
                else if (flag == category)
                {
                    s += ".";
                    int n = count;
                    for (int i = 0; i < n; i++)
                    {
                        std::string str = s + levelTostr(i);
                        int fd = open(str.c_str(), O_WRONLY | O_CREAT | O_APPEND, fMODE);
                        if (fd < 0)
                            perror("Error");
                        _fd.push_back(fd);
                    }
                }
                else
                {
                    // 不存在的分支
                    fprintf(stderr, "%s\n", "Non-existent branch");
                    assert(0);
                }
            }

            if (flag == display)
            {
                int fd = dup(2);
                if (fd < 0)
                    perror("Error");
                _fd.push_back(fd);
            }

#endif
        }

        void operator()(level le, const char *format, ...)
        {
            // 日志固定部分的构建
            char fix[64] = {0};
            time_t t = time(nullptr);
            struct tm *ct = localtime(&t);
            snprintf(fix, sizeof(fix) - 1, "[%s][%d-%d-%d %d:%d:%d]", levelTostr(le), ct->tm_year + 1900, ct->tm_mon + 1, ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);

            // 构建用户输入的变化部分
            char dynamic[1024] = {0};
            va_list s;
            va_start(s, format);
            vsnprintf(dynamic, sizeof(dynamic) - 1, format, s);
            va_end(s);

            // 拼接上述两个部分
            snprintf(_buffer, sizeof(_buffer) - 1, "%s::%s\n", fix, dynamic);

            out(le);
        }

        ~log()
        {
            for(auto e : _fd)
            {
                close(e);
            }
        }

    private:
        void out()
        {
            printf("%s", _buffer);
            _buffer[0] = 0;
        }

        void out(level le)
        {
            if(_fd.size() == 1)
            {
                write(_fd[0], _buffer, strlen(_buffer));
            }
            else if(_fd.size() > 1)
            {
                write(_fd[le], _buffer, strlen(_buffer));
            }
            else
            {
                fprintf(stderr, "%s\n", "Non-existent branch");
                    assert(0);
            }
        }

        std::string _name;       // 日志文件夹名称
        char _buffer[1088];
        std::vector<int> _fd;
    };
}
```

让我们看看效果

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241211163123922.mp4"></video>

# end