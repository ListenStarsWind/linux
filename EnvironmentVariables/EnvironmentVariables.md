# 环境变量

## 前日谈

上节课我们说了Linux进程状态和优先级，我们先来回顾一下。

我们首先通过`fork`和分支结构实现了子进程的创建，为以后的进程控制打下基础。同时我们也知道，Linux为了描述一个进程，会创建一个对象，我们称之为程序控制块`PCB`；Linux对程序的种种控制，其实上都是通过修改`PCB`来进行的。当我们创建一个子进程时，由于子进程没有自己的代码和数据，所以必须要用父进程的代码数据，代码是只读的，所以可以完全共用，有的数据是可写的，所以当子进程想要修改数据时，内核就会把对应的数据拷贝一个副本给子进程，从而保证父子进程间的独立性，防止父子进程相互干扰。

之后是进程状态。当进程已经做好进行下一步指令的准备时，它便处于就绪或者运行状态，而当下一步的指令需要其它计算机资源配合时，它便进入阻塞状态，被动地停止等待其它资源的响应，等需求被满足后，它就会重新回到运行状态。当内存资源过度紧张时，系统就会对某些进程进行挂起操作，只留`PCB`在各种队列里排队，代码和数据则会被直接压回磁盘，等被CPU执行时才写到内存中。

计算机的其它资源，例如各种硬件，同样遵循“先描述，再组织”的原则，系统对于它们的操作，实际上都是修改描述它们的结构体对象来实现的，而当某些进程需要这些资源时，它们就会被排到对应资源的等待队列。

而对于Linux，则具体讲了僵死状态，当子进程死亡，而父进程不对其进行资源回收时，子进程就会一直处于僵死状态，从而造成内存泄漏。而当父进程先死亡时，系统会自动领养子进程，以便于子进程死亡后的资源回收。

Linux优先级是进程的重要概念，但实际上由于操作方法有限，所以实际上不会管优先级；优先级可以通过`NI`有限间接的修改。Linux依据优先级的40个级别，使用了40成员大小的指针数组，每个成员都是一个小的运行队列，它们具有相同的优先级，为了让新建立的进程不直接插入指针数组，从而造成分不清谁已经执行过一次，谁又没被执行过的情况，Linux使用了两个40成员大小，CPU只会一个一个地遍历数组，当这个数组被遍历时，新来的待运行进程会把插入到另一个数组，每个小运行队列被遍历完后，相应的位图位就会被置为0，等到整个位图都是0了，这个数组即被完整地遍历一次，CPU就会遍历另一个数组，而让新来的进程插入到这个数组里。

## 进程的上下文

多个进程在多个CPU下分别，同时进行运行，这称之为并行 。多个进程在一个CPU下采用进程切换的方式，在一段时间之内，让多个进程都得以推进，称之为并发。CPU的速度太快了，所以人正常情况下感受不到并发时进程切换的卡顿感，造成了每个进程都在同时推进的假象。每个在CPU上运行的进程都会有一个叫做"时间片"的东西，当时间片耗尽时，这个进程就会被强行剥离下来，让下一个进程运行。为了让进程再次使用CPU时可以恢复上一次中断时的临时数据，在进程被强行剥离时，CPU会把一些描述进程运行状态的信息，比如下一行指令的位置，交给PCB，等到进程再次使用CPU时，先把中断数据交给CPU，让CPU接着上次中断时的状态继续运行。

CPU中包含着各种各样的寄存器，比如通用寄存器eax ebx ecx edx；维护栈帧的相关寄存器ebp esp eip ；状态寄存器status等。这些寄存器的读写速度很高，因此放着进程运行时需要被快速访问的数据，比如，返回值。当返回值较小时，它往往会被写在寄存器里，然后上一级函数接收返回值时，再把寄存器里的数据写到上一级函数里，从而实现不同函数栈帧的数据通信，而当对象较大时，寄存器就存不下了，于是可能就会在两个函数栈帧之间创建一些临时的空间块，等到被调用函数栈帧将被释放时，把被调用函数的返回值写到这些空间块里，然后上一级函数接收返回值就从空间块里读取，所以返回值往往都具有常属性。程序计数器也是需要被快速访问的数据，它其实就是`eip`，描述了程序下一行指令的位置，顺序结构，分支结构，循环结构，本质上就是改变程序计数器的变化方式；对于顺序结构来说，如果当前执行的是85行，那程序计数器指向的就是86行，而85行执行完了，就会来到86行，而程序计数器就会自加一，指向87行；对于分支结构，程序计数器可能就是跳着增加的；对于循环，则是增加到某处后再跳回到之前的某处位置。

我们把CPU中寄存器的各种信息统称为进程的上下文。当进程的时间片被耗尽后，CPU就会把上下文交给`PCB`，这样，等到进程再次使用CPU时，就能恢复到上次中断时的临时状态，然后接着这个临时状态继续运行。这就保证了进程的连续性。

用人来打比方，这好比是写代码时查出了BUG，想了一会，大致有眉头了；但突然有什么急事，那就把大致想法打个注释写在代码旁，然后保存一下项目，等急事做完了，再接着之前的思路想。

## 先感受

环境变量见得比较少，所以我们直接说几个，之后再去讲定义。

我们首先写了一个简单的程序，我们发现执行我自己的程序必须要带上`.\`，或者说，指定路径，我们知道系统中的种种指令，其本质就是程序，为什么系统的这些指令，比如`ls`，不用指定路径，直接输名字就行呢？

```shell
[wind@starry-sky Debug]$ ./EnvironmentVariables
hello world
[wind@starry-sky Debug]$ EnvironmentVariables
-bash: EnvironmentVariables: command not found
[wind@starry-sky Debug]$ ls
EnvironmentVariables  makefile
[wind@starry-sky Debug]$
```

原因在于系统为我们提供了一个名为`PATH`的环境变量，这个环境变量描述了默认指令的路径，当输入`ls`时，`bash`就会从这些路径中找到相应的程序，然后以子进程的形式让它运行。

```bash
[wind@starry-sky Debug]$ 如果想要看一下PATH,就用$符号转一下义，提示bash这不是字符串，而是名为PATH的变量^C
[wind@starry-sky Debug]$ echo PATH
PATH
[wind@starry-sky Debug]$ echo 认为这就是一个普通字符^C
[wind@starry-sky Debug]$ echo $PATH
/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
[wind@starry-sky Debug]$ 这些路径以":"作分隔符^C
[wind@starry-sky Debug]$ 也就是说，如果输入ls指令，它会一个一个地从^C
[wind@starry-sky Debug]$ /usr/local/bin^C
[wind@starry-sky Debug]$ /usr/bin^C
[wind@starry-sky Debug]$ /usr/local/sbin^C
[wind@starry-sky Debug]$ /usr/sbin^C
[wind@starry-sky Debug]$ /home/wind/.local/bin^C
[wind@starry-sky Debug]$ /home/wind/bin^C
[wind@starry-sky Debug]$ 路径下寻找ls，如果都找不到，就会报错：command not found^C
[wind@starry-sky Debug]$
```

`=`可以修改变量，注意`=`两边不能有空格，要紧挨着内容，此外，还要注意，这种修改是覆写式修改，不是追加式修改，如果这样写就会丢失原有内容：

```bash
[wind@starry-sky Debug]$ pwd
/home/wind/projects/EnvironmentVariables/bin/x64/Debug
[wind@starry-sky Debug]$ PATH=/home/wind/projects/EnvironmentVariables/bin/x64/Debug
[wind@starry-sky Debug]$ echo $PATH
/home/wind/projects/EnvironmentVariables/bin/x64/Debug
[wind@starry-sky Debug]$ EnvironmentVariables
hello world
[wind@starry-sky Debug]$ ls
-bash: ls: command not found
[wind@starry-sky Debug]$ clear
-bash: clear: command not found
[wind@starry-sky Debug]$ pwd
/home/wind/projects/EnvironmentVariables/bin/x64/Debug
[wind@starry-sky Debug]$
```

先不要关心`pwd`为什么还能用，先看看如何恢复：恢复其实很简单，我们改的这个环境变量，其实是`bash`进程的环境变量，所以它其实是临时的，是内存数据，不是文件数据。当新建一个`bash`界面时，它就会读取系统的环境变量配置文件，所以我们刚才修改的只是`bash`的环境变量，这是系统环境变量的一份拷贝，所以只要退出这个界面。重建一个新的界面，`PATH`就会恢复如初。

```shell
[wind@starry-sky ~]$ echo $PATH
/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
[wind@starry-sky ~]$
```

如果要实现追加的效果，就需要用下面的手段：

```bash
[wind@starry-sky Debug]$ PATH=$PATH:/home/wind/projects/EnvironmentVariables/bin/x64/Debug
[wind@starry-sky Debug]$ echo $PATH
/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin:/home/wind/projects/EnvironmentVariables/bin/x64/Debug
[wind@starry-sky Debug]$ EnvironmentVariables
hello world
[wind@starry-sky Debug]$ ls
EnvironmentVariables  makefile
[wind@starry-sky Debug]$ mv EnvironmentVariables out
[wind@starry-sky Debug]$ out
hello world
[wind@starry-sky Debug]$
```

windows也有，是吧。搜索栏搜索环境变量。

![image-20241021141111527](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410211411600.png)

我这电脑装的东西有些多：

![image-20241021141249018](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410211412058.png)

---------

我们发现，登录某个账号，初始目录都是默认的，我们称之为家目录。再登录一个用户：

```bash
[whisper@starry-sky ~]$ pwd
/home/whisper
[whisper@starry-sky ~]$ echo $HOME
/home/whisper
[whisper@starry-sky ~]$
```

```bash
[wind@starry-sky Debug]$ echo $HOME
/home/wind
[wind@starry-sky Debug]$
```

```bash
[root@starry-sky ~]# echo $HOME
/root
[root@starry-sky ~]#
```

同样的，当某个`bash`界面登录时，它就会依据具体用户，去获取系统对应账号的环境变量，其中有一项就叫做`HOME`，于是`bash`就会来到这个目录下。

`env`指令可以查看`bash`界面从系统继承下来的环境变量集合：

```shell
[wind@starry-sky Debug]$ env
XDG_SESSION_ID=7739
HOSTNAME=starry-sky
TERM=xterm
SHELL=/bin/bash
HISTSIZE=1000
SSH_CLIENT=112.26.31.132 4277 22
SSH_TTY=/dev/pts/0
USER=wind
LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
MAIL=/var/spool/mail/wind
PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
PWD=/home/wind/projects/EnvironmentVariables/bin/x64/Debug
LANG=en_US.UTF-8
HISTCONTROL=ignoredups
SHLVL=1
HOME=/home/wind
LOGNAME=wind
SSH_CONNECTION=112.26.31.132 4277 172.31.235.81 22
LESSOPEN=||/usr/bin/lesspipe.sh %s
XDG_RUNTIME_DIR=/run/user/1002
_=/usr/bin/env
OLDPWD=/home/wind/projects/EnvironmentVariables
[wind@starry-sky Debug]$
```

我们来说一下`HISTSIZE`，这就是历史指令的存储大小，系统会存储最近的1000条历史指令，当历史指令超过1000后，就会覆写掉更旧的指令。`history`可以查看存储的历史指令。也许显示的指令不足1000条，那可能是因为它被其它配置干扰了，在`bash`退出后历史指令会写入到`~/.bash_history`文件中，下次重新登录时就能访问之前的历史记录。

`SSH_TTY`则描述了当前`bash`界面对应的文件，Linux下一切皆文件，所以像`bash`这种字符设备，是有相应的文件对应的，这个文件有什么用呢？它可以让不同的界面相互通信，不过好像只有`root`有这个级别的权限。

```shell
[whisper@starry-sky ~]$ echo $SSH_TTY
/dev/pts/1
[whisper@starry-sky ~]$
```

```shell
[root@starry-sky ~]# echo "hello" > /dev/pts/1
[root@starry-sky ~]#
```

```shell
[whisper@starry-sky ~]$ hello
```

下面一长串的`LS_COLORS`是配色方案。我们看到环境变量里也有一个`PWD`进程工作目录。`LOGNAME  USER`是当前用户，`OLDPWD`是上一个历史路径，所以`cd -`可以回到上一个路径。

那函数怎么获取环境变量呢？有几种方法，首先是系统调用接口：

![image-20241021152918915](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410211529097.png)

```cpp
#include<iostream>
#include<stdlib.h>

int main()
{
	std::cout << "PATH->" << getenv("PATH") << std::endl;
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ make run
PATH->/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
[wind@starry-sky Debug]$
```

```cpp
#include<iostream>
#include<stdlib.h>

int main()
{
	std::cout << "USER->" << getenv("USER") << std::endl;
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make ; make run
USER->wind
[wind@starry-sky Debug]$ sudo make run
[sudo] password for wind: 
USER->root
[wind@starry-sky Debug]$
```

## 再描述

环境变量是系统提过的一组`NAME=value`形式的变量，不同的用户有不同的环境变量，环境变量通常具有全局属性。

环境变量凭什么有全局属性呢？因为我们目前接触的绝大多数进程都是由`bash`创建的子进程，这些子进程如果要获取环境变量，都是从父进程`bash`那里拿的，所以具有全局属性。

为了说明这个全局属性，我们需要讲讲main函数的参数，或者说命令行参数。形如

```cpp
int main(int argc, char* argv[])
{
    // others
    return 0;
}
```

其中`argc`就是`argv`的成员个数，让我们看看这数组里面到底有什么。

```cpp
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char* argv[])
{
	int i = 0;
	for (; i < argc; i++)
	{
		printf("argv[%d]-> %s\n", i, argv[i]);
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
EnvironmentVariables  makefile
[wind@starry-sky Debug]$ ./EnvironmentVariables
argv[0]-> ./EnvironmentVariables
[wind@starry-sky Debug]$ ./EnvironmentVariables -a
argv[0]-> ./EnvironmentVariables
argv[1]-> -a
[wind@starry-sky Debug]$ ./EnvironmentVariables -a -b
argv[0]-> ./EnvironmentVariables
argv[1]-> -a
argv[2]-> -b
[wind@starry-sky Debug]$ ./EnvironmentVariables -a -b -c
argv[0]-> ./EnvironmentVariables
argv[1]-> -a
argv[2]-> -b
argv[3]-> -c
[wind@starry-sky Debug]$ ./EnvironmentVariables -a -b -c -d
argv[0]-> ./EnvironmentVariables
argv[1]-> -a
argv[2]-> -b
argv[3]-> -c
argv[4]-> -d
[wind@starry-sky Debug]$
```

不过，我要特别说一声，指针数组`argv`的实际成员个数是`argc + 1`，最后一个成员是`NULL`，作为终止位，所以还可以这样写：

```cpp
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char* argv[])
{
	int i = 0;
	for (; argv[i]; i++)
	{
		printf("argv[%d]-> %s\n", i, argv[i]);
	}
	return 0;
}
```

我们知道`main`函数实际上不是程序运行的第一个函数，所以亦可以被传参，这样就可以进行命令行传参了。比如`ls -a -b -c`会被`bash`解析为`ls  -a   -b   -c`这四个字符串，这样它就可以有各种选项了。

```cpp
#include<iostream>
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "help->" << std::endl;
		std::cout << "-a:Function One" << std::endl;
		std::cout << "-b:Function Two" << std::endl;
		std::cout<<"-c:Function Three" << std::endl;
		return 0;
	}
	else if (strcmp(argv[1], "-a") == 0)
	{
		std::cout << "Function One" << std::endl;
	}
	else if (strcmp(argv[1], "-b") == 0)
	{
		std::cout << "Function Two" << std::endl;
	}
	else if (strcmp(argv[1], "-c") == 0)
	{
		std::cout << "Function Three" << std::endl;
	}
	else
	{
		std::cout << "Function Default" << std::endl;
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ./EnvironmentVariables
help->
-a:Function One
-b:Function Two
-c:Function Three
[wind@starry-sky Debug]$ ./EnvironmentVariables -a
Function One
[wind@starry-sky Debug]$ ./EnvironmentVariables -b
Function Two
[wind@starry-sky Debug]$ ./EnvironmentVariables -c
Function Three
[wind@starry-sky Debug]$ ./EnvironmentVariables -d
Function Default
[wind@starry-sky Debug]$
```

main函数还可以再加一个参数

```cpp
int main(int argc, char* argv[], char* env[])
{
    // others
    return 0;
}
```

体验一下：

```cpp
#include<iostream>
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[], char* env[])
{
	int cir = 0;
	for (; env[cir]; cir++)
	{
		std::cout<<env[cir]<<std::endl;
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ./EnvironmentVariables
XDG_SESSION_ID=7810
HOSTNAME=starry-sky
TERM=xterm
SHELL=/bin/bash
HISTSIZE=1000
SSH_CLIENT=112.26.31.132 4340 22
SSH_TTY=/dev/pts/0
USER=wind
LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
MAIL=/var/spool/mail/wind
PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
PWD=/home/wind/projects/EnvironmentVariables/bin/x64/Debug
LANG=en_US.UTF-8
HISTCONTROL=ignoredups
SHLVL=1
HOME=/home/wind
LOGNAME=wind
SSH_CONNECTION=112.26.31.132 4340 172.31.235.81 22
LESSOPEN=||/usr/bin/lesspipe.sh %s
XDG_RUNTIME_DIR=/run/user/1002
_=./EnvironmentVariables
OLDPWD=/home/wind/projects/EnvironmentVariables
[wind@starry-sky Debug]$
```

我们把`argv`和`env`称为核心向量表，`argv`是命令行参数核心向量表，`env`是环境变量核心向量表。

我们所运行的进程绝大多数都是`bash`的子进程。`bash`启动时，会从系统配置文件中读取环境变量信息，而`bash`的子进程就通过主函数参数或系统调用接口的形式把环境变量继承下来，所以环境变量具有全局性。

怎么添加一个环境变量呢？使用`export NAME=val`指令即可：

```shell
[wind@starry-sky Debug]$ export MY_VALUE=1234
[wind@starry-sky Debug]$ echo $MY_VALUE
1234
[wind@starry-sky Debug]$
```

我们再次运行之前的程序，发现我们这个程序对`bash`环境变量的修改也继承了：

```shell
[wind@starry-sky Debug]$ ./EnvironmentVariables
XDG_SESSION_ID=7810
HOSTNAME=starry-sky
TERM=xterm
SHELL=/bin/bash
HISTSIZE=1000
SSH_CLIENT=112.26.31.132 4340 22
MY_VALUE=1234
OLDPWD=/home/wind/projects/EnvironmentVariables
SSH_TTY=/dev/pts/0
USER=wind
LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
MAIL=/var/spool/mail/wind
PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
PWD=/home/wind/projects/EnvironmentVariables/bin/x64/Debug
LANG=en_US.UTF-8
HISTCONTROL=ignoredups
SHLVL=1
HOME=/home/wind
LOGNAME=wind
SSH_CONNECTION=112.26.31.132 4340 172.31.235.81 22
LESSOPEN=||/usr/bin/lesspipe.sh %s
XDG_RUNTIME_DIR=/run/user/1002
_=./EnvironmentVariables
[wind@starry-sky Debug]$
```

`unset NAME`可以取消我们设置的环境变量：

```shell
[wind@starry-sky Debug]$ unset MY_VALUE
[wind@starry-sky Debug]$ echo $MY_VALUE

[wind@starry-sky Debug]$
```

再次运行我们的程序，就会发现也没了：

```shell
[wind@starry-sky Debug]$ ./EnvironmentVariables
XDG_SESSION_ID=7810
HOSTNAME=starry-sky
TERM=xterm
SHELL=/bin/bash
HISTSIZE=1000
SSH_CLIENT=112.26.31.132 4340 22
OLDPWD=/home/wind/projects/EnvironmentVariables
SSH_TTY=/dev/pts/0
USER=wind
LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
MAIL=/var/spool/mail/wind
PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
PWD=/home/wind/projects/EnvironmentVariables/bin/x64/Debug
LANG=en_US.UTF-8
HISTCONTROL=ignoredups
SHLVL=1
HOME=/home/wind
LOGNAME=wind
SSH_CONNECTION=112.26.31.132 4340 172.31.235.81 22
LESSOPEN=||/usr/bin/lesspipe.sh %s
XDG_RUNTIME_DIR=/run/user/1002
_=./EnvironmentVariables
[wind@starry-sky Debug]$
```

### 本地变量

说实话，本地变量实际用不到，但还是要说一下。就是临时存一下，没其他用法。如果我们添加变量的时候，不带`export`的话，得到的就是本地变量，尽管能打印出来，但它不属于环境变量。

```shell
[wind@starry-sky Debug]$ a=1
[wind@starry-sky Debug]$ b=2
[wind@starry-sky Debug]$ c=3
[wind@starry-sky Debug]$ echo $a
1
[wind@starry-sky Debug]$ echo $b
2
[wind@starry-sky Debug]$ echo $c
3
[wind@starry-sky Debug]$ env
XDG_SESSION_ID=7810
HOSTNAME=starry-sky
TERM=xterm
SHELL=/bin/bash
HISTSIZE=1000
SSH_CLIENT=112.26.31.132 4340 22
OLDPWD=/home/wind/projects/EnvironmentVariables
SSH_TTY=/dev/pts/0
USER=wind
LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
MAIL=/var/spool/mail/wind
PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
PWD=/home/wind/projects/EnvironmentVariables/bin/x64/Debug
LANG=en_US.UTF-8
HISTCONTROL=ignoredups
SHLVL=1
HOME=/home/wind
LOGNAME=wind
SSH_CONNECTION=112.26.31.132 4340 172.31.235.81 22
LESSOPEN=||/usr/bin/lesspipe.sh %s
XDG_RUNTIME_DIR=/run/user/1002
_=/usr/bin/env
[wind@starry-sky Debug]$
```

`set`可以查看所有变量，什么变量都能显示。因人而异，有些服务器内容比较多，有的比较少，我是多的那种，就不打印了。

本地变量只会在`bash`内部有效，不会被子进程继承。比如` PS1='[\u@\h \W]\$ '`就是规定`bash`的提示符的，`[wind@starry-sky Debug]$`，`PS2='> '`是续行提示符。

```shell
[wind@starry-sky Debug]$ ls \
> -a \
> -b \
> -c 
test  .  EnvironmentVariables  makefile  ..
[wind@starry-sky Debug]$
```

`export`实际上就是把本地变量导入到环境变量中。

### 内建命令

上面我们遇到一个奇怪的现象——`echo`可以读取本地变量，如果`echo`是`bash`的子进程，那理论上来说，本地变量是不会被子进程继承的，那它怎么把本地变量的值打印出来的。

为什么呢？因为`echo`不是`bash`的子进程，而是`bash`自身的一部分。我们接触的命令有两部分：

- 常规命令：由`bash`通过创建子进程来完成
- 内建命令：由`bash`亲自完成，`bash`自己调用了自己的或者系统的接口来实现。

我们接下来说说`cd`，`cd`移动的是`bash`工作路径，这意味着它就是一种内建命令。下面我们通过系统接口`chdir`来创建一个会自己移动工作目录的程序，来模拟一下`bash`中的`cd`。

![image-20241021180510459](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410211805600.png)

```cpp
#include<iostream>
#include<unistd.h>

int main(int argc, char* argv[], char* env[])
{
	// 如果argv[0] == "cd"
	// 那就不要创建子进程
	// 自己移动工作路径
	std::cout << "begin" << std::endl;
	sleep(30);
	chdir(argv[1]);
	std::cout << "end" << std::endl;
	sleep(30);
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
EnvironmentVariables  makefile
[wind@starry-sky Debug]$ mkdir test
[wind@starry-sky Debug]$ ls
EnvironmentVariables  makefile  test
[wind@starry-sky Debug]$ ./EnvironmentVariables ./test
begin
end
[wind@starry-sky Debug]$
```

```shell
[wind@starry-sky ~]$ ps ajx | head -1 ; ps ajx | grep EnvironmentVariables | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
30859  7607  7607 30859 pts/0     7607 S+    1002   0:00 ./EnvironmentVariables ./test
[wind@starry-sky ~]$ ll /proc/7607
total 0
dr-xr-xr-x 2 wind wind 0 Oct 21 18:22 attr
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 autogroup
-r-------- 1 wind wind 0 Oct 21 18:22 auxv
-r--r--r-- 1 wind wind 0 Oct 21 18:21 cgroup
--w------- 1 wind wind 0 Oct 21 18:22 clear_refs
-r--r--r-- 1 wind wind 0 Oct 21 18:21 cmdline
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 comm
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 coredump_filter
-r--r--r-- 1 wind wind 0 Oct 21 18:22 cpuset
lrwxrwxrwx 1 wind wind 0 Oct 21 18:22 cwd -> /home/wind/projects/EnvironmentVariables/bin/x64/Debug
-r-------- 1 wind wind 0 Oct 21 18:22 environ
lrwxrwxrwx 1 wind wind 0 Oct 21 18:22 exe -> /home/wind/projects/EnvironmentVariables/bin/x64/Debug/EnvironmentVariables
dr-x------ 2 wind wind 0 Oct 21 18:21 fd
dr-x------ 2 wind wind 0 Oct 21 18:22 fdinfo
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 gid_map
-r-------- 1 wind wind 0 Oct 21 18:22 io
-r--r--r-- 1 wind wind 0 Oct 21 18:22 limits
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 loginuid
dr-x------ 2 wind wind 0 Oct 21 18:22 map_files
-r--r--r-- 1 wind wind 0 Oct 21 18:22 maps
-rw------- 1 wind wind 0 Oct 21 18:22 mem
-r--r--r-- 1 wind wind 0 Oct 21 18:22 mountinfo
-r--r--r-- 1 wind wind 0 Oct 21 18:22 mounts
-r-------- 1 wind wind 0 Oct 21 18:22 mountstats
dr-xr-xr-x 6 wind wind 0 Oct 21 18:22 net
dr-x--x--x 2 wind wind 0 Oct 21 18:21 ns
-r--r--r-- 1 wind wind 0 Oct 21 18:22 numa_maps
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 oom_adj
-r--r--r-- 1 wind wind 0 Oct 21 18:22 oom_score
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 oom_score_adj
-r--r--r-- 1 wind wind 0 Oct 21 18:22 pagemap
-r-------- 1 wind wind 0 Oct 21 18:22 patch_state
-r--r--r-- 1 wind wind 0 Oct 21 18:22 personality
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 projid_map
lrwxrwxrwx 1 wind wind 0 Oct 21 18:22 root -> /
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 sched
-r--r--r-- 1 wind wind 0 Oct 21 18:22 schedstat
-r--r--r-- 1 wind wind 0 Oct 21 18:22 sessionid
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 setgroups
-r--r--r-- 1 wind wind 0 Oct 21 18:22 smaps
-r--r--r-- 1 wind wind 0 Oct 21 18:22 stack
-r--r--r-- 1 wind wind 0 Oct 21 18:21 stat
-r--r--r-- 1 wind wind 0 Oct 21 18:22 statm
-r--r--r-- 1 wind wind 0 Oct 21 18:21 status
-r--r--r-- 1 wind wind 0 Oct 21 18:22 syscall
dr-xr-xr-x 3 wind wind 0 Oct 21 18:22 task
-r--r--r-- 1 wind wind 0 Oct 21 18:22 timers
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 uid_map
-r--r--r-- 1 wind wind 0 Oct 21 18:22 wchan
[wind@starry-sky ~]$ ll /proc/7607
total 0
dr-xr-xr-x 2 wind wind 0 Oct 21 18:22 attr
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 autogroup
-r-------- 1 wind wind 0 Oct 21 18:22 auxv
-r--r--r-- 1 wind wind 0 Oct 21 18:21 cgroup
--w------- 1 wind wind 0 Oct 21 18:22 clear_refs
-r--r--r-- 1 wind wind 0 Oct 21 18:21 cmdline
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 comm
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 coredump_filter
-r--r--r-- 1 wind wind 0 Oct 21 18:22 cpuset
lrwxrwxrwx 1 wind wind 0 Oct 21 18:22 cwd -> /home/wind/projects/EnvironmentVariables/bin/x64/Debug/test
-r-------- 1 wind wind 0 Oct 21 18:22 environ
lrwxrwxrwx 1 wind wind 0 Oct 21 18:22 exe -> /home/wind/projects/EnvironmentVariables/bin/x64/Debug/EnvironmentVariables
dr-x------ 2 wind wind 0 Oct 21 18:21 fd
dr-x------ 2 wind wind 0 Oct 21 18:22 fdinfo
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 gid_map
-r-------- 1 wind wind 0 Oct 21 18:22 io
-r--r--r-- 1 wind wind 0 Oct 21 18:22 limits
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 loginuid
dr-x------ 2 wind wind 0 Oct 21 18:22 map_files
-r--r--r-- 1 wind wind 0 Oct 21 18:22 maps
-rw------- 1 wind wind 0 Oct 21 18:22 mem
-r--r--r-- 1 wind wind 0 Oct 21 18:22 mountinfo
-r--r--r-- 1 wind wind 0 Oct 21 18:22 mounts
-r-------- 1 wind wind 0 Oct 21 18:22 mountstats
dr-xr-xr-x 6 wind wind 0 Oct 21 18:22 net
dr-x--x--x 2 wind wind 0 Oct 21 18:21 ns
-r--r--r-- 1 wind wind 0 Oct 21 18:22 numa_maps
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 oom_adj
-r--r--r-- 1 wind wind 0 Oct 21 18:22 oom_score
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 oom_score_adj
-r--r--r-- 1 wind wind 0 Oct 21 18:22 pagemap
-r-------- 1 wind wind 0 Oct 21 18:22 patch_state
-r--r--r-- 1 wind wind 0 Oct 21 18:22 personality
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 projid_map
lrwxrwxrwx 1 wind wind 0 Oct 21 18:22 root -> /
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 sched
-r--r--r-- 1 wind wind 0 Oct 21 18:22 schedstat
-r--r--r-- 1 wind wind 0 Oct 21 18:22 sessionid
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 setgroups
-r--r--r-- 1 wind wind 0 Oct 21 18:22 smaps
-r--r--r-- 1 wind wind 0 Oct 21 18:22 stack
-r--r--r-- 1 wind wind 0 Oct 21 18:21 stat
-r--r--r-- 1 wind wind 0 Oct 21 18:22 statm
-r--r--r-- 1 wind wind 0 Oct 21 18:21 status
-r--r--r-- 1 wind wind 0 Oct 21 18:22 syscall
dr-xr-xr-x 3 wind wind 0 Oct 21 18:22 task
-r--r--r-- 1 wind wind 0 Oct 21 18:22 timers
-rw-r--r-- 1 wind wind 0 Oct 21 18:22 uid_map
-r--r--r-- 1 wind wind 0 Oct 21 18:22 wchan
[wind@starry-sky ~]$
```

对比一下`cwd`，它确实进去了。

也可以通过`extern char** environ;`的系统接口访问全套的环境变量。`environ`直接指向环境变量的核心向量表。

```cpp
#include<iostream>
#include<unistd.h>

int main()
{
	extern char** environ;
	int i = 0;
	for (; environ[i]; i++)
	{
		std::cout << environ[i] << std::endl;
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ make run
SHELL=/bin/bash
_=/usr/bin/make
HISTCONTROL=ignoredups
LESSOPEN=||/usr/bin/lesspipe.sh %s
SSH_CONNECTION=112.26.31.132 4372 172.31.235.81 22
PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
SSH_TTY=/dev/pts/0
XDG_RUNTIME_DIR=/run/user/1002
LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
PWD=/home/wind/projects/EnvironmentVariables/bin/x64/Debug
HOME=/home/wind
LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
LOGNAME=wind
HOSTNAME=starry-sky
SHLVL=1
XDG_SESSION_ID=7861
USER=wind
OLDPWD=/home/wind/projects/EnvironmentVariables
MAKEFLAGS=
MFLAGS=
SSH_CLIENT=112.26.31.132 4372 22
MAIL=/var/spool/mail/wind
HISTSIZE=1000
LANG=en_US.UTF-8
TERM=xterm
MAKELEVEL=1
[wind@starry-sky Debug]$
```



# 完
