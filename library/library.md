# library

## preface

之前我们曾经稍微谈及过动静态库。我们说，静态库是`libname.a`，动态库是`libname.so`。静态链接是把静态库拷贝到项目文件夹中，然后与`.o`文件链接；而动态链接则是给程序一个入口，调用系统共用的动态库。我们也说过，编译器默认使用动态链接。接下来，我们先创建一个静态库，然后使用一下。

## create

生成什么库呢？就加减乘除吧。

```cpp
// windmath.h
#pragma once

extern int winderrno;

int add(int x, int y); 
int sub(int x, int y);
int mul(int x, int y);
int div(int x, int y);

// windmath.cpp
#include"windmath.h"

int winderrno = 0;

int add(int x, int y)
{
	return x + y;
}

int sub(int x, int y)
{
	return x - y;
}

int mul(int x, int y)
{
	return x * y;
}

int div(int x, int y)
{
	if (y == 0)
	{
		winderrno = -1;
		return -1;
	}
	return x / y;
}
```

`extern`关键字的作用是强调后面的`int winderrno`是个声明，不是定义；至于`winderrno`是我们这套库的错误码，当出现一些错误，比如除数为0时，我们就为标记该错误码。

首先我们要把库的方法，也就是`.c`变成`.o`文件

```shell
[wind@starry-sky Debug]$ ls
windmath.cpp  windmath.h
[wind@starry-sky Debug]$ g++ -c windmath.cpp
[wind@starry-sky Debug]$ ls
windmath.cpp  windmath.h  windmath.o
[wind@starry-sky Debug]$
```

然后再在`.o`的基础上编成库。

因为可能有多套方法，也就是多个`.o`文件，所以接下来我们要把所有的`.o`文件都放在一块，然后再对它们进行包装即可。

```shell
[wind@starry-sky Debug]$ ar -rc libwindmath.a windmath.o
[wind@starry-sky Debug]$ ls
libwindmath.a  windmath.cpp  windmath.h  windmath.o
[wind@starry-sky Debug]$
```

稍微说一下，`ar`就是生成库的指令，格式是先指令，再输出文件，最后是依赖文件。选项中的`r`是"replace"，也就是替换的意思，该选项的意思是，如果`libwindmath.a`存在，就用新生成的文件替换旧文件，`c`就是"create"，也就是生成的意思，如果没有`libwindmath.a`，就生成一份新的。

我们先生成静态库，注意静态库的格式是`libname.a`，要按照格式来写，最后是依赖文件，也就是所有的`.o`文件。

接下来我们要把`libwindmath.a`再包装一下，让其它人可以直接使用，我们称之为“发布”。怎么包装呢？其实也很简单，把`.h`和`libname.a`分类放在一个`lib`文件夹就行了。

```shell
[wind@starry-sky Debug]$ ls
libwindmath.a  windmath.cpp  windmath.h  windmath.o
[wind@starry-sky Debug]$ mkdir -p lib/include
[wind@starry-sky Debug]$ mkdir -p lib/windmath
[wind@starry-sky Debug]$ tree lib
lib
├── include
└── windmath

2 directories, 0 files
[wind@starry-sky Debug]$ cp *.h ./lib/include
[wind@starry-sky Debug]$ cp *.a ./lib/windmath
[wind@starry-sky Debug]$ tree lib
lib
├── include
│   └── windmath.h
└── windmath
    └── libwindmath.a

2 directories, 2 files
[wind@starry-sky Debug]$
```

头文件都放在一个文件夹下，具体的库要分文件夹去分别存储，方便区分。接下来把`lib`给用户即可。

接下来，我们把上面的各种指令写到`makefile`里

```makefile
lib=libwindmath.a    
$(lib):windmath.o    
  @ar -rc $@ $^    
windmath.o:windmath.cpp    
  @g++ -c $^    
    
.PHONY:clean    
clean:    
  @rm -f *.o *.a    
    
.PHONY:output    
output:    
  @mkdir -p ./lib/include     
  @mkdir -p ./lib/windmath     
  @cp *.h ./lib/include     
  @cp libwindmath.a ./lib/windmath 
```

第一行`lib=libwindmath.a`，定义了一个变量 `lib`，它的值是 `libwindmath.a`，表示要生成的静态库的文件名。

第二行`$(lib):windmath.o`表示依赖关系，`$(lib)`会被替换成`lib`的值，也就是说，写的更明白的话，它长这样`libwindmath.a:windmath.o`，至于依赖方法，就不说了。

两个伪指令也不说了。

```shell
[wind@starry-sky Debug]$ rm -rf lib
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
makefile  windmath.cpp  windmath.h
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ make output
[wind@starry-sky Debug]$ ls
lib  libwindmath.a  makefile  windmath.cpp  windmath.h  windmath.o
[wind@starry-sky Debug]$ tree lib
lib
├── include
│   └── windmath.h
└── windmath
    └── libwindmath.a

2 directories, 2 files
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
lib  makefile  windmath.cpp  windmath.h
[wind@starry-sky Debug]$
```

## using

若要使用库，则先要把`lib`移到项目路径下。

```shell
[wind@starry-sky Debug]$ mkdir project
[wind@starry-sky Debug]$ cd project
[wind@starry-sky project]$ mv ../lib .
[wind@starry-sky project]$ ls
lib
[wind@starry-sky project]$ tree lib
lib
├── include
│   └── windmath.h
└── windmath
    └── libwindmath.a

2 directories, 2 files
[wind@starry-sky project]$ vim main.cpp
[wind@starry-sky project]$ cat main.cpp
#include"windmath.h"
#include<stdio.h>

int main()
{
  printf("1+1=%d\n", add(1, 1));
  return 0;
}
[wind@starry-sky project]$ 
```

由于我们的代码中没有提供`windmath.h`的详细路径，所以无法直接编译通过。对此，有两种解决方案。一是代码中写明路径，二是编译环节为编译器添加额外的头文件搜索路径

```shell
[wind@starry-sky project]$ g++ main.cpp
main.cpp:1:9: fatal error: windmath.h: No such file or directory
    1 | #include"windmath.h"
      |         ^~~~~~~~~~~~
compilation terminated.
[wind@starry-sky project]$ g++ -I ./lib/include main.cpp
/opt/rh/devtoolset-11/root/usr/libexec/gcc/x86_64-redhat-linux/11/ld: /tmp/cc14TOI8.o: in function `main':
main.cpp:(.text+0xf): undefined reference to `add(int, int)'
collect2: error: ld returned 1 exit status
[wind@starry-sky project]$
```

选项`I`表示"include"，再带上头文件路径后，就没有出现找不到头文件的问题。现在的问题是编译器找不到`add`的实现，是链接错误，或者说，它找不到链接库，库文件也是有自己的系统默认搜索路径的，所以编译时也要写明额外搜索的库路径。

```shell
[wind@starry-sky project]$ g++ -I lib/include -L lib/windmath main.cpp
/opt/rh/devtoolset-11/root/usr/libexec/gcc/x86_64-redhat-linux/11/ld: /tmp/cc1UGZKm.o: in function `main':
main.cpp:(.text+0xf): undefined reference to `add(int, int)'
collect2: error: ld returned 1 exit status
[wind@starry-sky project]$
```

选项`L`表示"link"，用于添加额外的库搜索路径，但现在依旧是找不到，因为编译器不知道该链接什么库，尽管库路径里只有一个库文件。

```shell
wind@starry-sky project]$ g++ main.cpp -I lib/include -L lib/windmath -llibwindmath
/opt/rh/devtoolset-11/root/usr/libexec/gcc/x86_64-redhat-linux/11/ld: cannot find -llibwindmath
/opt/rh/devtoolset-11/root/usr/libexec/gcc/x86_64-redhat-linux/11/ld: note to link with lib/windmath/libwindmath.a use -l:libwindmath.a or rename it to liblibwindmath.a
collect2: error: ld returned 1 exit status
```

选项`l`表示“lib”，用于指明链接的具体库文件，注意`l`之后紧跟名字。为什么还是错呢？因为`l`后面要跟库文件名字，另外，由于这里选项太多了，所以`main.cpp`最好移到选项前。为什么标准库不用带`l`选项呢？因为`gcc  g++`作为C/C++的编译器，它自已有一套机制知道什么头文件对应什么具体库文件，但对于第三方库来说，那就必须要用户手动指明。

```shell
[wind@starry-sky project]$ g++ main.cpp -I lib/include -L lib/windmath -lwindmath
[wind@starry-sky project]$ ls
a.out  lib  main.cpp
[wind@starry-sky project]$ ./a.out
1+1=2
[wind@starry-sky project]$
```

让我们修改一下`main.cpp`

```cpp
#include"windmath.h"
#include<stdio.h>

int main()
{
  int n = div(10, 0);
  if(winderrno == 0)
    printf("10/0=%d\n", n);
  else
    printf("failed div, errno:%d\n",winderrno);
  return 0;
}
```

```shell
[wind@starry-sky project]$ g++ main.cpp -I lib/include -L lib/windmath -lwindmath
[wind@starry-sky project]$ ./a.out
failed div, errno:-1
```

我们可以通过`ldd`指令查看可执行程序的动态链接关系

```shell
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007ffc4711a000)
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007fdd68de9000)
	libm.so.6 => /lib64/libm.so.6 (0x00007fdd68ae7000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007fdd688d1000)
	libc.so.6 => /lib64/libc.so.6 (0x00007fdd68503000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fdd6916a000)
[wind@starry-sky project]$
```

我们可以看到关系表中并没有`windmath`，为什么呢？也很简单，`windmath`是静态库，它怎么会出现在动态链接关系表中呢？对`gcc/g++`这些编译器来说，它最主要的任务就是生成可执行程序，当编译器可以找到，也知道该链接什么库，而用户又没有特殊要求时，它会优先使用动态链接，没有动态链接才尝试静态链接，而当用户有特殊要求时，比如，用户带上了`-static`选项，那就必须严格遵循用户的要求，直接尝试静态链接，链接不了就报错，也别管用户为什么要求使用静态链接，既然写明了这个特殊要求，用户自然有他的理由，不要管那么多，就应该严格满足用户提出的特殊要求；而在这里，我们要求编译器就链接`windmath`，说明用户默认使用该库对应的链接方式，而`windmath`是静态库，所以它自然采用静态链接，而`windmath`是静态库，所以它使用，也必须使用静态链接。从这个例子我们也可以看到，有时候程序是动静态链接混着用的。

----------------------

在上面的学习中，我们可以看到，对于第三方库来说，其`-l`选项肯定是不能省的，那前面的两个选项`-I`和`-L`能不能省掉呢？当然可以，我们也知道，带这些选项的原因是因为系统默认路径下没这些头文件和库文件，所以需要我们特别指定搜索路径，那我们在系统默认路径里直接或者间接加上这些文件不就行了？

第一种方法，简单粗暴。就是直接把文件拷到默认路径下。

```shell
[wind@starry-sky project]$ tree lib
lib
├── include
│   └── windmath.h
└── windmath
    └── libwindmath.a

2 directories, 2 files
[wind@starry-sky project]$ ls /usr/include/windmath.h
ls: cannot access /usr/include/windmath.h: No such file or directory
[wind@starry-sky project]$ ls /lib64/libwindmath.a
ls: cannot access /lib64/libwindmath.a: No such file or directory
[wind@starry-sky project]$ sudo cp lib/include/windmath.h /usr/include/
[sudo] password for wind: 
[wind@starry-sky project]$ sudo cp lib/windmath/libwindmath.a /lib64/
[wind@starry-sky project]$ ls /usr/include/windmath.h
/usr/include/windmath.h
[wind@starry-sky project]$ ls /lib64/libwindmath.a
/lib64/libwindmath.a
[wind@starry-sky project]$ rm a.out
[wind@starry-sky project]$ g++ main.cpp -lwindmath
[wind@starry-sky project]$ ./a.out
failed div, errno:-1
[wind@starry-sky project]$
```

其实这里文件的拷贝就是库安装的本质，我现在还记得，当初我这系统的`gcc/g++`版本太低，有些语法不支持，我去网上找了一圈，才成功更新了这两个编译器，那篇`gcc/g++`更新教程的文章作者是这样说的：“有些教程试图通过直接替换`gcc/g++`程序本体的方式来更新`gcc/g++` ，但很明显是不行的，因为这样做只更新了程序本体，没更新头文件和库文件。”也就是说，`gcc/g++`的版本更新不仅要更新程序本体，更要对库进行更新，不过我没有说过只更新这两个就行了，还有一些别的操作，让编译器本体和系统与新的库建立联系才行。

不过，对于不成熟的库，建议不要安装，以防止污染系统自己的库。或者这样说，我们自己写的库太垃圾了(〃'▽'〃)，配不上系统自己的库。

第二种方法是建立软链接，记得使用绝对路径

```shell
[wind@starry-sky project]$ sudo rm /usr/include/windmath.h
[sudo] password for wind: 
[wind@starry-sky project]$ sudo rm /lib64/libwindmath.a
[wind@starry-sky project]$ rm a.out
[wind@starry-sky project]$ g++ main.cpp -lwindmath
main.cpp:1:9: fatal error: windmath.h: No such file or directory
    1 | #include"windmath.h"
      |         ^~~~~~~~~~~~
compilation terminated.
[wind@starry-sky project]$ pwd
/home/wind/projects/library/bin/x64/Debug/project
[wind@starry-sky project]$ sudo ln -s /home/wind/projects/library/bin/x64/Debug/project/lib/include /usr/include/wind_link_h
[wind@starry-sky project]$ sudo ln -s /home/wind/projects/library/bin/x64/Debug/project/lib/windmath/libwindmath.a /lib64/libwindmath.a
[wind@starry-sky project]$ g++ main.cpp -lwindmath
main.cpp:2:9: fatal error: windmath.h: No such file or directory
    2 | #include"windmath.h"
      |         ^~~~~~~~~~~~
compilation terminated.
[wind@starry-sky project]$ cat main.cpp
#include"windmath.h"
#include<stdio.h>

int main()
{
  int n = div(10, 0);
  if(winderrno == 0)
    printf("10/0=%d\n", n);
  else
    printf("failed div, errno:%d\n",winderrno);
  return 0;
}
[wind@starry-sky project]$ vim main.cpp
[wind@starry-sky project]$ cat main.cpp
#include"wind_link_h/windmath.h"
#include<stdio.h>

int main()
{
  int n = div(10, 0);
  if(winderrno == 0)
    printf("10/0=%d\n", n);
  else
    printf("failed div, errno:%d\n",winderrno);
  return 0;
}
[wind@starry-sky project]$ g++ main.cpp -lwindmath
[wind@starry-sky project]$ ./a.out
failed div, errno:-1
[wind@starry-sky project]$
```

这里有些细节需要注意，我们头文件的软链接用的是`include`文件夹，而不是头文件本体，因为可能有多个头文件，链接`include`可以一起找，而库文件就一个，所以就直接链接了，不过这样做的话需要改代码，去改一下头文件的路径，这样才能找到。

-------------

接下来我们去看看动态库。

现在我们又创建了两个头文件及其对应的源文件。

![image-20241129094054168](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411290940357.png)

![image-20241129094002360](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411290940621.png)

```shell
[wind@starry-sky Debug]$ sudo unlink /usr/include/wind_link_h
[wind@starry-sky Debug]$ sudo unlink /lib64/libwindmath.a
[wind@starry-sky Debug]$ ll
total 32
-rw-rw-r-- 1 wind wind  261 Nov 25 15:59 makefile
drwxrwxr-x 3 wind wind 4096 Nov 29 08:27 project
-rw-rw-r-- 1 wind wind   82 Nov 28 21:56 windlog.cpp
-rw-rw-r-- 1 wind wind   46 Nov 28 21:56 windlog.h
-rw-rw-r-- 1 wind wind  285 Nov 25 14:42 windmath.cpp
-rw-rw-r-- 1 wind wind  136 Nov 25 14:42 windmath.h
-rw-rw-r-- 1 wind wind   79 Nov 29 08:32 windprint.cpp
-rw-rw-r-- 1 wind wind   33 Nov 29 08:32 windprint.h
[wind@starry-sky Debug]$
```

动态库的生成流程与静态库完全一样，都是先生成`.o`文件，然后再包装在一起，只是有些细节存在差异。

首先仍旧是生成`.o`文件，需要注意的是，对于动态库，需要带上选项`-fPIC`。

```shell
[wind@starry-sky Debug]$ g++ -fPIC -c windlog.cpp
[wind@starry-sky Debug]$ g++ -fPIC -c windprint.cpp
[wind@starry-sky Debug]$ ll
total 40
-rw-rw-r-- 1 wind wind  261 Nov 25 15:59 makefile
drwxrwxr-x 3 wind wind 4096 Nov 29 08:27 project
-rw-rw-r-- 1 wind wind   82 Nov 28 21:56 windlog.cpp
-rw-rw-r-- 1 wind wind   46 Nov 28 21:56 windlog.h
-rw-rw-r-- 1 wind wind 1584 Nov 29 08:38 windlog.o
-rw-rw-r-- 1 wind wind  285 Nov 25 14:42 windmath.cpp
-rw-rw-r-- 1 wind wind  136 Nov 25 14:42 windmath.h
-rw-rw-r-- 1 wind wind   79 Nov 29 08:32 windprint.cpp
-rw-rw-r-- 1 wind wind   33 Nov 29 08:32 windprint.h
-rw-rw-r-- 1 wind wind 1576 Nov 29 08:38 windprint.o
[wind@starry-sky Debug]$
```

对于动态库来说，打包时使用的是`gcc/g++`指令，而不是`ar`，也就是说，`gcc/g++`自己有着动态库的生成，使用的全流程功能，或者说，`gcc/g++`默认动态链接更优。另外，当生成动态库时，还需要带上选项`-shared`。

```shell
[wind@starry-sky Debug]$ g++ *.o -shared -o libwindmethod.so 
[wind@starry-sky Debug]$ ll
total 56
-rwxrwxr-x 1 wind wind 15488 Nov 29 08:48 libwindmethod.so
-rw-rw-r-- 1 wind wind   261 Nov 25 15:59 makefile
drwxrwxr-x 3 wind wind  4096 Nov 29 08:27 project
-rw-rw-r-- 1 wind wind    82 Nov 28 21:56 windlog.cpp
-rw-rw-r-- 1 wind wind    46 Nov 28 21:56 windlog.h
-rw-rw-r-- 1 wind wind  1584 Nov 29 08:38 windlog.o
-rw-rw-r-- 1 wind wind   285 Nov 25 14:42 windmath.cpp
-rw-rw-r-- 1 wind wind   136 Nov 25 14:42 windmath.h
-rw-rw-r-- 1 wind wind    79 Nov 29 08:32 windprint.cpp
-rw-rw-r-- 1 wind wind    33 Nov 29 08:32 windprint.h
-rw-rw-r-- 1 wind wind  1576 Nov 29 08:38 windprint.o
[wind@starry-sky Debug]$
```

动态库要被其它进程共用，这意味着它必须被加载到内存中，所以它默认有执行权限。也就是说动态库自身不能单独运行，它必须依靠其它程序运行，毕竟它自身没有`main`。

接下来我们把这些指令写到`markfile`中

```makefile
dynamic-lib=libwindmethod.so    
static-lib=libwindmath.a    
    
.PHONY:all    
all:$(static-lib) $(dynamic-lib)    
      
    
$(static-lib):windmath.o    
  @ar -rc $@ $^    
windmath.o:windmath.cpp    
  @g++ -c $^    
    
$(dynamic-lib):windlog.o windprint.o    
  @g++ $^ -shared -o $@                                                                                                                      
    
windlog.o:windlog.cpp    
  @g++ windlog.cpp -fPIC -c    
    
windprint.o:windprint.cpp    
  @g++ $^ -fPIC -c    
    
.PHONY:clean    
clean:    
  @rm -rf *.o *.a *.so windlib    
    
.PHONY:output    
output:    
  @mkdir -p ./windlib/include     
  @mkdir -p ./windlib/windmath     
  @mkdir -p ./windlib/windmethod     
  @cp *.h ./windlib/include     
  @cp libwindmath.a ./windlib/windmath    
  @cp libwindmethod.so ./windlib/windmethod 
```

我们新定义了一个伪指令`all`，该指令的的依赖关系是两个库，没有依赖方法，两个库的具体生成方式在下面，另外`clean`我们也增加了删除整个库文件夹的功能，库文件夹的构建也做了相应更新。

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ll
total 32
-rw-rw-r-- 1 wind wind  617 Nov 29 09:17 makefile
drwxrwxr-x 3 wind wind 4096 Nov 29 08:27 project
-rw-rw-r-- 1 wind wind   82 Nov 28 21:56 windlog.cpp
-rw-rw-r-- 1 wind wind   46 Nov 28 21:56 windlog.h
-rw-rw-r-- 1 wind wind  285 Nov 25 14:42 windmath.cpp
-rw-rw-r-- 1 wind wind  136 Nov 25 14:42 windmath.h
-rw-rw-r-- 1 wind wind   79 Nov 29 08:32 windprint.cpp
-rw-rw-r-- 1 wind wind   33 Nov 29 08:32 windprint.h
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ll
total 64
-rw-rw-r-- 1 wind wind  1934 Nov 29 09:22 libwindmath.a
-rwxrwxr-x 1 wind wind 15488 Nov 29 09:22 libwindmethod.so
-rw-rw-r-- 1 wind wind   617 Nov 29 09:17 makefile
drwxrwxr-x 3 wind wind  4096 Nov 29 08:27 project
-rw-rw-r-- 1 wind wind    82 Nov 28 21:56 windlog.cpp
-rw-rw-r-- 1 wind wind    46 Nov 28 21:56 windlog.h
-rw-rw-r-- 1 wind wind  1584 Nov 29 09:22 windlog.o
-rw-rw-r-- 1 wind wind   285 Nov 25 14:42 windmath.cpp
-rw-rw-r-- 1 wind wind   136 Nov 25 14:42 windmath.h
-rw-rw-r-- 1 wind wind  1736 Nov 29 09:22 windmath.o
-rw-rw-r-- 1 wind wind    79 Nov 29 08:32 windprint.cpp
-rw-rw-r-- 1 wind wind    33 Nov 29 08:32 windprint.h
-rw-rw-r-- 1 wind wind  1576 Nov 29 09:22 windprint.o
[wind@starry-sky Debug]$ make output
[wind@starry-sky Debug]$ ll
total 68
-rw-rw-r-- 1 wind wind  1934 Nov 29 09:22 libwindmath.a
-rwxrwxr-x 1 wind wind 15488 Nov 29 09:22 libwindmethod.so
-rw-rw-r-- 1 wind wind   617 Nov 29 09:17 makefile
drwxrwxr-x 3 wind wind  4096 Nov 29 08:27 project
drwxrwxr-x 5 wind wind  4096 Nov 29 09:22 windlib
-rw-rw-r-- 1 wind wind    82 Nov 28 21:56 windlog.cpp
-rw-rw-r-- 1 wind wind    46 Nov 28 21:56 windlog.h
-rw-rw-r-- 1 wind wind  1584 Nov 29 09:22 windlog.o
-rw-rw-r-- 1 wind wind   285 Nov 25 14:42 windmath.cpp
-rw-rw-r-- 1 wind wind   136 Nov 25 14:42 windmath.h
-rw-rw-r-- 1 wind wind  1736 Nov 29 09:22 windmath.o
-rw-rw-r-- 1 wind wind    79 Nov 29 08:32 windprint.cpp
-rw-rw-r-- 1 wind wind    33 Nov 29 08:32 windprint.h
-rw-rw-r-- 1 wind wind  1576 Nov 29 09:22 windprint.o
[wind@starry-sky Debug]$ tree windlib
windlib
├── include
│   ├── windlog.h
│   ├── windmath.h
│   └── windprint.h
├── windmath
│   └── libwindmath.a
└── windmethod
    └── libwindmethod.so

3 directories, 5 files
[wind@starry-sky Debug]$
```

现在我们来使用它们

```shell
[wind@starry-sky Debug]$ rm -rf project/lib
[wind@starry-sky Debug]$ mv windlib project/
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ cd project
[wind@starry-sky project]$ ll
total 24
-rwxrwxr-x 1 wind wind 16024 Nov 28 21:21 a.out
-rw-rw-r-- 1 wind wind   205 Nov 28 21:20 main.cpp
drwxrwxr-x 5 wind wind  4096 Nov 29 09:24 windlib
[wind@starry-sky project]$ vim main.cpp
[wind@starry-sky project]$ cat main.cpp
#include"windmath.h"
#include"windlog.h"
#include"windprint.h"
#include<stdio.h>

int main()
{
  int n = div(10, 0);
  if(winderrno == 0)
    printf("10/0=%d\n", n);
  else
    printf("failed div, errno:%d\n",winderrno);

  print();
  Log("A system is never complete.");
  return 0;
}
[wind@starry-sky project]$ g++ main.cpp -I windlib/include -L windlib/windmath -L windlib/windmethod -lwindmath -lwindmethod
[wind@starry-sky project]$ ll
total 24
-rwxrwxr-x 1 wind wind 16112 Nov 29 09:46 a.out
-rw-rw-r-- 1 wind wind   285 Nov 29 09:43 main.cpp
drwxrwxr-x 5 wind wind  4096 Nov 29 09:24 windlib
[wind@starry-sky project]$
```

我们发现可执行程序确实是生成了，但有个问题

```shell
[wind@starry-sky project]$ ./a.out
./a.out: error while loading shared libraries: libwindmethod.so: cannot open shared object file: No such file or directory
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007ffc795f6000)
	libwindmethod.so => not found
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007fd953460000)
	libm.so.6 => /lib64/libm.so.6 (0x00007fd95315e000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007fd952f48000)
	libc.so.6 => /lib64/libc.so.6 (0x00007fd952b7a000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fd9537e1000)
[wind@starry-sky project]$
```

它跑不动，使用`ldd`指令可以发现它找不到`libwindmethod.so`，命名编译的时候已经指定好库文件了，为什么还是找不到呢？因为主句换了，编译的时候告诉的是编译器库的路径和名字，而这里说“它不知道”中的“它”是系统，而非编译器，更具体的说，是加载器，加载器负责把文件加载到内存中，可执行程序`a.out`它当然可以找到，但`libwindmethod.so`它找不到，后面我们会说，使用动态链接的程序在需要使用对应动态库的时候会请求系统将动态库加载到内存中，程序本身并不存储动态库的具体位置，程序默认系统自己知道动态库的位置，可在该场景下，`libwindmethod.so`不在系统默认路径下，所以系统也不知道这个动态库到底在哪，所以无法加载，该程序自然无法运行。

如何解决这个问题，该问题的关键在于加载器不知道动态库在哪里，那让系统知道不就行了，还是分为直接和间接方法，直接方法就是把库文件拷贝到系统默认路径下，在此省略。

第一个间接方法仍是软链接

```shell
[wind@starry-sky project]$ realpath windlib/windmethod/*
/home/wind/projects/library/bin/x64/Debug/project/windlib/windmethod/libwindmethod.so
[wind@starry-sky project]$ sudo ln -s /home/wind/projects/library/bin/x64/Debug/project/windlib/windmethod/libwindmethod.so /lib64/libwindmethod.so
[sudo] password for wind: 
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007ffc48dfb000)
	libwindmethod.so => /lib64/libwindmethod.so (0x00007f8d1acd0000)
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007f8d1a73a000)
	libm.so.6 => /lib64/libm.so.6 (0x00007f8d1a438000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f8d1a222000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f8d19e54000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f8d1aabb000)
[wind@starry-sky project]$ ./a.out
failed div, errno:-1
This sentence is false.
failed:A system is never complete.
[wind@starry-sky project]$
```

第二个方法是使用用户环境变量`LD_LIBRARY_PATH`，该变量专门负责描述用户自定义的动态库路径。较干净的用户可能都没有这个环境变量，此时创建一下即可。

```shell
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007ffc42ff5000)
	libwindmethod.so => /lib64/libwindmethod.so (0x00007f7723d13000)
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007f772377d000)
	libm.so.6 => /lib64/libm.so.6 (0x00007f772347b000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f7723265000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f7722e97000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f7723afe000)
[wind@starry-sky project]$ sudo unlink /lib64/libwindmethod.so
[sudo] password for wind: 
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007ffdbe9a7000)
	libwindmethod.so => not found
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007fae73040000)
	libm.so.6 => /lib64/libm.so.6 (0x00007fae72d3e000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007fae72b28000)
	libc.so.6 => /lib64/libc.so.6 (0x00007fae7275a000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fae733c1000)
[wind@starry-sky project]$ echo $LD_LIBRARY_PATH
/opt/rh/devtoolset-11/root/usr/lib64:/opt/rh/devtoolset-11/root/usr/lib:/opt/rh/devtoolset-11/root/usr/lib64/dyninst:/opt/rh/devtoolset-11/root/usr/lib/dyninst:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
[wind@starry-sky project]$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/wind/projects/library/bin/x64/Debug/project/windlib/windmethod
[wind@starry-sky project]$ echo $LD_LIBRARY_PATH
/opt/rh/devtoolset-11/root/usr/lib64:/opt/rh/devtoolset-11/root/usr/lib:/opt/rh/devtoolset-11/root/usr/lib64/dyninst:/opt/rh/devtoolset-11/root/usr/lib/dyninst:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64:/home/wind/projects/library/bin/x64/Debug/project/windlib/windmethod
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007ffcaee9e000)
	libwindmethod.so => /home/wind/projects/library/bin/x64/Debug/project/windlib/windmethod/libwindmethod.so (0x00007f1056018000)
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007f1055a7c000)
	libm.so.6 => /lib64/libm.so.6 (0x00007f105577a000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f1055564000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f1055196000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f1055dfd000)
[wind@starry-sky project]$ ./a.out
failed div, errno:-1
This sentence is false.
failed:A system is never complete.
[wind@starry-sky project]$
```

不过这种方法具有临时性，想要持久保存该环境变量，需要修改`bash`的相关配置文件。

```shell
[wind@starry-sky project]$ logout

Connection closed.

Disconnected from remote host(starry-sky) at 10:36:29.

Type `help' to learn how to use Xshell prompt.
[C:\~]$ ssh wind@120.55.90.240:22


Connecting to 120.55.90.240:22...
Connection established.
To escape to local shell, press 'Ctrl+Alt+]'.

WARNING! The remote SSH server rejected X11 forwarding request.
Last login: Fri Nov 29 10:39:58 2024 from 117.136.20.88

Welcome to Alibaba Cloud Elastic Compute Service !

[wind@starry-sky ~]$ cd /home/wind/projects/library/bin/x64/Debug/project/
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007fff49fee000)
	libwindmethod.so => not found
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007f8934c47000)
	libm.so.6 => /lib64/libm.so.6 (0x00007f8934945000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f893472f000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f8934361000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f8934fc8000)
[wind@starry-sky project]$
```

第三个方法是直接动系统配置文件，这些配置文件描述了什么路径是系统默认库路径。在系统的`/etc/ld.so.conf.d`路径下有一系列`.conf`文件，实际上就是文本文件，创建一个新的文件把路径写进去就行了。这是全局有效的，不看用户。

```shell
[wind@starry-sky project]$ cd /etc/ld.so.conf.d
[wind@starry-sky ld.so.conf.d]$ ll
total 16
-rw-r--r--  1 root root 26 Jun 11 22:41 bind-export-x86_64.conf
-r--r--r--  1 root root 63 Jun  4 22:48 kernel-3.10.0-1160.119.1.el7.x86_64.conf
-r--r--r--. 1 root root 63 Oct 20  2020 kernel-3.10.0-1160.el7.x86_64.conf
-rw-r--r--. 1 root root 17 Oct  2  2020 mariadb-x86_64.conf
[wind@starry-sky ld.so.conf.d]$ sudo echo "/home/wind/projects/library/bin/x64/Debug/project/windlib/windmethod" > wind_link.conf
-bash: wind_link.conf: Permission denied
[wind@starry-sky ld.so.conf.d]$ su
Password: 
[root@starry-sky ld.so.conf.d]# echo "/home/wind/projects/library/bin/x64/Debug/project/windlib/windmethod" > wind_link.conf
[root@starry-sky ld.so.conf.d]# ll
total 20
-rw-r--r--  1 root root 26 Jun 11 22:41 bind-export-x86_64.conf
-r--r--r--  1 root root 63 Jun  4 22:48 kernel-3.10.0-1160.119.1.el7.x86_64.conf
-r--r--r--. 1 root root 63 Oct 20  2020 kernel-3.10.0-1160.el7.x86_64.conf
-rw-r--r--. 1 root root 17 Oct  2  2020 mariadb-x86_64.conf
-rw-r--r--  1 root root 69 Nov 29 10:55 wind_link.conf
[root@starry-sky ld.so.conf.d]# ldconfig
[root@starry-sky ld.so.conf.d]# exit
exit
[wind@starry-sky ld.so.conf.d]$ cd -
/home/wind/projects/library/bin/x64/Debug/project
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007ffcf65e8000)
	libwindmethod.so => /home/wind/projects/library/bin/x64/Debug/project/windlib/windmethod/libwindmethod.so (0x00007f273ffb2000)
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007f273fa1c000)
	libm.so.6 => /lib64/libm.so.6 (0x00007f273f71a000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f273f504000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f273f136000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f273fd9d000)
[wind@starry-sky project]$ ./a.out
failed div, errno:-1
This sentence is false.
failed:A system is never complete.
[wind@starry-sky project]$ su
Password: 
[root@starry-sky project]# rm /etc/ld.so.conf.d/wind_link.conf
rm: remove regular file ‘/etc/ld.so.conf.d/wind_link.conf’? y  
[root@starry-sky project]# ldconfig
[root@starry-sky project]# exit
exit
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007ffd935fd000)
	libwindmethod.so => not found
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007f2e4c239000)
	libm.so.6 => /lib64/libm.so.6 (0x00007f2e4bf37000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f2e4bd21000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f2e4b953000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f2e4c5ba000)
[wind@starry-sky project]$
```

切记，写好之后要执行`ldconfig`，让系统重新加载一下这些文件。

介绍了一种直接方法和三种间接方法，但实际情况下，一般直接用直接方法，因为别人的库既然发布在网络上了，理论上是成熟的，所以直接拷贝到默认路径就行了，当然也不用自己手动拷贝，既然是成熟的库，应该是有相应的默认安装方式的，比如脚本什么的。

--------------------

我们知道，静态链接是直接把库拷贝到程序中的，所以即使其删除，也对程序运行没有影响。

```shell
[wind@starry-sky project]$ ls
a.out  main.cpp  windlib
[wind@starry-sky project]$ ldd a.out
	linux-vdso.so.1 =>  (0x00007fff1ddb3000)
	libwindmethod.so => not found
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007f643f75d000)
	libm.so.6 => /lib64/libm.so.6 (0x00007f643f45b000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f643f245000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f643ee77000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f643fade000)
[wind@starry-sky project]$ sudo cp windlib/windmath/libwindmath.a /lib64/libwindmath.a 
[sudo] password for wind: 
[wind@starry-sky project]$ sudo cp windlib/windmethod/libwindmethod.so /lib64/libwindmethod.so
[wind@starry-sky project]$ rm a.out
[wind@starry-sky project]$ g++ main.cpp -I windlib/include -lwindmath -lwindmethod
[wind@starry-sky project]$ g++ main.cpp -I windlib/include -lwindmath -lwindmethod -o b.out
[wind@starry-sky project]$ ls
a.out  b.out  main.cpp  windlib
[wind@starry-sky project]$ ./a.out
failed div, errno:-1
This sentence is false.
failed:A system is never complete.
[wind@starry-sky project]$ ./b.out
failed div, errno:-1
This sentence is false.
failed:A system is never complete.
[wind@starry-sky project]$ sudo rm /lib64/libwindmath.a
[wind@starry-sky project]$ ./a.out
failed div, errno:-1
This sentence is false.
failed:A system is never complete.
[wind@starry-sky project]$ ./b.out
failed div, errno:-1
This sentence is false.
failed:A system is never complete.
```

而动态库是要被加载到内存的，是要被其它进程共享的，所以它也叫共享库，当动态库被删除后，使用对应动态链接的程序就无法运行了。

```shell
[wind@starry-sky project]$ sudo rm /lib64/libwindmethod.so
[wind@starry-sky project]$ ./a.out
./a.out: error while loading shared libraries: libwindmethod.so: cannot open shared object file: No such file or directory
[wind@starry-sky project]$ ./b.out
./b.out: error while loading shared libraries: libwindmethod.so: cannot open shared object file: No such file or directory
[wind@starry-sky project]$
```

动态库是被共享的，所以当多个进程都要使用动态链接的时候，系统只在内存中加载一份动态库，然后供需要的进程共享，这就是动态库节省内存的原因。

## principle

下面我们来说说动态链接的原理。

我们知道，当用户要求执行某个程序时，系统就会通过一系列的机制把对应的文件加载到物理内存中，并通过页表将物理内存映射为虚拟内存，这样进程在实际运行是只要关注自己的虚拟地址空间即可，而不必关注数据在物理内存的存储位置和存储形式。

![image-20241129133052929](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411291330021.png)

动态库也是文件，当进程需要动态库时，就会触发缺页中断，之后系统就会在磁盘中找到对应的动态库文件将其加载到物理内存中。既然程序是在虚拟地址空间的视角下进行运行的，很明显，被加载到内存中的动态库数据也会通过页表映射到虚拟地址空间上，映射到哪里呢？就是虚拟地址空间的共享区。也就是说共享区中存储着动态库的代码和数据，当进程调用动态库时，就会从正文代码跳转到共享区的代码上，调用结束后就重新跳回正文代码。

很容易衍生出，系统中会存在多个动态库，系统对这些动态库都进行了相应的抽象描述并将它们统一的管理起来，也就是先描述再组织。具体的细节我们不再论述，总体上与其它的先描述再组织是相同的。

对于动态库中的数据，比如像我们上面代码中的`winderrno`，当某个进程将要对其进行修改时，就会触发写时拷贝，从而避免使用同个动态库的多个进程间的相互干扰。再如之前我们从系统角度理解`IO`时，所做的实验，父进程创建子进程，它们之间都使用标准库中的对应动态库，如果语言缓冲区还未被刷新就被子进程继承，那在之后，父子进程中的某一个刷新缓冲区时并不会影响另一个进程，因为会触发动态库的写时拷贝，这样就保证了进程之间的相对独立性。

---------------------------

上面的原理解释看起来还是有些空泛，所以接下来我们将会从细节出发。

首先让我们重回“地址”这个概念。我们对地址的重新认识将分为三个阶段：一，程序被内存加载前的地址，二，程序加载到内存后的地址，三，动态库的地址。

让我们先回到计算机早期阶段，那时候的程序是用二进制写的，打在纸带上，所以纸带的地址就是程序还未运行的地址，当然，那时候地址还没现在划分这么细，但可以肯定的是，可程序文件中是有地址的，大概长这样。

![image-20241129143217714](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411291432914.png)

就是汇编指令前面的那堆地址，重新调试，会发现这些前面的地址都是不变的。

而在可执行程序文件中，它的内容就依照进程地址空间中用户部分的分布而排列，我们把这种组织形式称之为“平坦模式”。

我们把这种地址称为逻辑地址，逻辑地址在编译生成可执行程序的时候就已经存在。

现在我们随手写一个程序，用来看一些细节。

```cpp
#include<stdio.h>

int main()
{
    printf("No system can fully understand itself.\n");
    return 0;
}
```

```shell
[wind@starry-sky test]$ objdump -S out

out:     file format elf64-x86-64


Disassembly of section .init:

0000000000401000 <_init>:
  401000:	48 83 ec 08          	sub    $0x8,%rsp
  401004:	48 8b 05 ed 2f 00 00 	mov    0x2fed(%rip),%rax        # 403ff8 <__gmon_start__>
  40100b:	48 85 c0             	test   %rax,%rax
  40100e:	74 05                	je     401015 <_init+0x15>
  401010:	e8 3b 00 00 00       	call   401050 <__gmon_start__@plt>
  401015:	48 83 c4 08          	add    $0x8,%rsp
  401019:	c3                   	ret    

Disassembly of section .plt:

0000000000401020 <puts@plt-0x10>:
  401020:	ff 35 e2 2f 00 00    	push   0x2fe2(%rip)        # 404008 <_GLOBAL_OFFSET_TABLE_+0x8>
  401026:	ff 25 e4 2f 00 00    	jmp    *0x2fe4(%rip)        # 404010 <_GLOBAL_OFFSET_TABLE_+0x10>
  40102c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000000401030 <puts@plt>:
  401030:	ff 25 e2 2f 00 00    	jmp    *0x2fe2(%rip)        # 404018 <puts@GLIBC_2.2.5>
  401036:	68 00 00 00 00       	push   $0x0
  40103b:	e9 e0 ff ff ff       	jmp    401020 <_init+0x20>

0000000000401040 <__libc_start_main@plt>:
  401040:	ff 25 da 2f 00 00    	jmp    *0x2fda(%rip)        # 404020 <__libc_start_main@GLIBC_2.2.5>
  401046:	68 01 00 00 00       	push   $0x1
  40104b:	e9 d0 ff ff ff       	jmp    401020 <_init+0x20>

0000000000401050 <__gmon_start__@plt>:
  401050:	ff 25 d2 2f 00 00    	jmp    *0x2fd2(%rip)        # 404028 <__gmon_start__>
  401056:	68 02 00 00 00       	push   $0x2
  40105b:	e9 c0 ff ff ff       	jmp    401020 <_init+0x20>

Disassembly of section .text:

0000000000401060 <_start>:
  401060:	31 ed                	xor    %ebp,%ebp
  401062:	49 89 d1             	mov    %rdx,%r9
  401065:	5e                   	pop    %rsi
  401066:	48 89 e2             	mov    %rsp,%rdx
  401069:	48 83 e4 f0          	and    $0xfffffffffffffff0,%rsp
  40106d:	50                   	push   %rax
  40106e:	54                   	push   %rsp
  40106f:	49 c7 c0 c0 11 40 00 	mov    $0x4011c0,%r8
  401076:	48 c7 c1 50 11 40 00 	mov    $0x401150,%rcx
  40107d:	48 c7 c7 36 11 40 00 	mov    $0x401136,%rdi
  401084:	e8 b7 ff ff ff       	call   401040 <__libc_start_main@plt>
  401089:	f4                   	hlt    
  40108a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000000401090 <deregister_tm_clones>:
  401090:	b8 38 40 40 00       	mov    $0x404038,%eax
  401095:	48 3d 38 40 40 00    	cmp    $0x404038,%rax
  40109b:	74 13                	je     4010b0 <deregister_tm_clones+0x20>
  40109d:	b8 00 00 00 00       	mov    $0x0,%eax
  4010a2:	48 85 c0             	test   %rax,%rax
  4010a5:	74 09                	je     4010b0 <deregister_tm_clones+0x20>
  4010a7:	bf 38 40 40 00       	mov    $0x404038,%edi
  4010ac:	ff e0                	jmp    *%rax
  4010ae:	66 90                	xchg   %ax,%ax
  4010b0:	c3                   	ret    
  4010b1:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
  4010b8:	00 00 00 00 
  4010bc:	0f 1f 40 00          	nopl   0x0(%rax)

00000000004010c0 <register_tm_clones>:
  4010c0:	be 38 40 40 00       	mov    $0x404038,%esi
  4010c5:	48 81 ee 38 40 40 00 	sub    $0x404038,%rsi
  4010cc:	48 89 f0             	mov    %rsi,%rax
  4010cf:	48 c1 ee 3f          	shr    $0x3f,%rsi
  4010d3:	48 c1 f8 03          	sar    $0x3,%rax
  4010d7:	48 01 c6             	add    %rax,%rsi
  4010da:	48 d1 fe             	sar    %rsi
  4010dd:	74 11                	je     4010f0 <register_tm_clones+0x30>
  4010df:	b8 00 00 00 00       	mov    $0x0,%eax
  4010e4:	48 85 c0             	test   %rax,%rax
  4010e7:	74 07                	je     4010f0 <register_tm_clones+0x30>
  4010e9:	bf 38 40 40 00       	mov    $0x404038,%edi
  4010ee:	ff e0                	jmp    *%rax
  4010f0:	c3                   	ret    
  4010f1:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
  4010f8:	00 00 00 00 
  4010fc:	0f 1f 40 00          	nopl   0x0(%rax)

0000000000401100 <__do_global_dtors_aux>:
  401100:	f3 0f 1e fa          	endbr64 
  401104:	80 3d 29 2f 00 00 00 	cmpb   $0x0,0x2f29(%rip)        # 404034 <completed.0>
  40110b:	75 13                	jne    401120 <__do_global_dtors_aux+0x20>
  40110d:	55                   	push   %rbp
  40110e:	48 89 e5             	mov    %rsp,%rbp
  401111:	e8 7a ff ff ff       	call   401090 <deregister_tm_clones>
  401116:	c6 05 17 2f 00 00 01 	movb   $0x1,0x2f17(%rip)        # 404034 <completed.0>
  40111d:	5d                   	pop    %rbp
  40111e:	c3                   	ret    
  40111f:	90                   	nop
  401120:	c3                   	ret    
  401121:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
  401128:	00 00 00 00 
  40112c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000000401130 <frame_dummy>:
  401130:	f3 0f 1e fa          	endbr64 
  401134:	eb 8a                	jmp    4010c0 <register_tm_clones>

0000000000401136 <main>:
?#include<stdio.h>

int main()
{
  401136:	55                   	push   %rbp
  401137:	48 89 e5             	mov    %rsp,%rbp
    printf("No system can fully understand itself.\n");
  40113a:	bf 10 20 40 00       	mov    $0x402010,%edi
  40113f:	e8 ec fe ff ff       	call   401030 <puts@plt>
    return 0;
  401144:	b8 00 00 00 00       	mov    $0x0,%eax
  401149:	5d                   	pop    %rbp
  40114a:	c3                   	ret    
  40114b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000000401150 <__libc_csu_init>:
  401150:	41 57                	push   %r15
  401152:	41 89 ff             	mov    %edi,%r15d
  401155:	41 56                	push   %r14
  401157:	49 89 f6             	mov    %rsi,%r14
  40115a:	41 55                	push   %r13
  40115c:	49 89 d5             	mov    %rdx,%r13
  40115f:	41 54                	push   %r12
  401161:	4c 8d 25 b0 2c 00 00 	lea    0x2cb0(%rip),%r12        # 403e18 <__frame_dummy_init_array_entry>
  401168:	55                   	push   %rbp
  401169:	48 8d 2d b0 2c 00 00 	lea    0x2cb0(%rip),%rbp        # 403e20 <__do_global_dtors_aux_fini_array_entry>
  401170:	53                   	push   %rbx
  401171:	4c 29 e5             	sub    %r12,%rbp
  401174:	31 db                	xor    %ebx,%ebx
  401176:	48 c1 fd 03          	sar    $0x3,%rbp
  40117a:	48 83 ec 08          	sub    $0x8,%rsp
  40117e:	e8 7d fe ff ff       	call   401000 <_init>
  401183:	48 85 ed             	test   %rbp,%rbp
  401186:	74 1e                	je     4011a6 <__libc_csu_init+0x56>
  401188:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
  40118f:	00 
  401190:	4c 89 ea             	mov    %r13,%rdx
  401193:	4c 89 f6             	mov    %r14,%rsi
  401196:	44 89 ff             	mov    %r15d,%edi
  401199:	41 ff 14 dc          	call   *(%r12,%rbx,8)
  40119d:	48 83 c3 01          	add    $0x1,%rbx
  4011a1:	48 39 eb             	cmp    %rbp,%rbx
  4011a4:	75 ea                	jne    401190 <__libc_csu_init+0x40>
  4011a6:	48 83 c4 08          	add    $0x8,%rsp
  4011aa:	5b                   	pop    %rbx
  4011ab:	5d                   	pop    %rbp
  4011ac:	41 5c                	pop    %r12
  4011ae:	41 5d                	pop    %r13
  4011b0:	41 5e                	pop    %r14
  4011b2:	41 5f                	pop    %r15
  4011b4:	c3                   	ret    
  4011b5:	90                   	nop
  4011b6:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
  4011bd:	00 00 00 

00000000004011c0 <__libc_csu_fini>:
  4011c0:	f3 c3                	repz ret 

Disassembly of section .fini:

00000000004011c4 <_fini>:
  4011c4:	48 83 ec 08          	sub    $0x8,%rsp
  4011c8:	48 83 c4 08          	add    $0x8,%rsp
  4011cc:	c3                   	ret    
[wind@starry-sky test]$
```

我们看到，每条指令根据其特定的形式，都有着确定的长度，比如110行那里，`push   %rbp`是占一个字节，`mov    %rsp,%rbp`是三个字节，`mov    $0x402010,%edi`是五个字节，两个`mov` 的大小不一样是因为其后面操作参数的不同，这样，就可以通过段地址加偏移量的方式访问这些指令。硬件厂家在设计CPU的时候，会在其中内置一种名叫“指令集”的东西，它就像词典那样，帮助CPU认识这些汇编指令，从而执行相应的操作。

前面说的都是可执行程序还未加载到内存时的情况。当程序被加载到内存之后，逻辑地址就变成了虚拟地址，毕竟逻辑地址本身就是按照进程地址空间的分步而划分的；当然，这些指令也肯定会存储在物理内存中，这样这些指令就有了两种地址，虚拟地址和物理地址，它们通过页表相互映射。当CPU执行到了诸如`call   401030 <puts@plt>`的时候，CPU就会通过页表找到虚拟地址`401030`对应的物理地址，从而`call`某个函数对象。

当然，由于惰性加载原则，可执行文件不是直接一次性加载到内存中的，比如它可能先加载可执行文件的表头，表头描述了该程序的大致信息，这些信息一方面可以对该进程对应的内核数据进行初始化。另一方面可以告诉系统第一条指令，或者程序的入口在哪里，这样，程序计数器就会跳到这个地址上，然后可能发现页表中此处的虚拟地址没有对应的物理地址，于是就会触发缺页中断，把，或许是一个页框大小的数据再加载到内存中，这样程序就会跑起来了。

现在我们还没有把动态库牵扯进来，很明显，对于动态库中的种种内容，肯定也要有自己对应的逻辑地址，我们之前也说过了，动态库中的内容会被映射到共享区，现在的问题是，该如何决定动态库中的逻辑地址呢？如果还是按照进程地址空间的分步来划分，就会引发一些问题，比如动态库中的某些虚拟地址与程序本体里的虚拟地址重合了，又或者，不同的动态库之间存在重合的虚拟地址，虚拟地址作为唯一标识符，是一定不能有重复的，所以就需要修改动态库中的逻辑地址分配模式，这就是我们创建动态库时必须带上`-fPIC`(与地址无关码)选项的原因，它会让`gcc`以另一种逻辑地址分配策略生成`.o`。

当带上`-fPIC`时，动态库的逻辑地址分配策略叫做偏移量更加合适，其实就是直接从0开始，没有按照进程地址空间的分步，直接分配，当`gcc`把`.o`文件打包在一起时，实际就是把这些`.o`文件的偏移量简单地相加，而当`gcc`使用这个动态库编译其它程序时，它会在程序的共享区中找个合适的逻辑地址作为动态库的起始地址，或许也会大致看看动态库有多大，给它在逻辑地址中预留一定空间，接着再加其它动态库(如果有的话)，当程序实际运行时，它对动态库的调用实际上就是通过编译时确定的动态库起始逻辑地址，加上动态库的偏移量来进行的。《程序员的自我修养》好像把这个叫做装载时重定位。

之后若是对动态库中的某些内容进行修改时，就会触发写时拷贝，从而保证进程独立性。

也不用担心程序分不清动态库，它们的起始逻辑地址编译时就已经确定了。

```shell
[wind@starry-sky test]$ ldd out
	linux-vdso.so.1 =>  (0x00007ffcc3941000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f580a296000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f580a664000)
[wind@starry-sky test]$
```

# end