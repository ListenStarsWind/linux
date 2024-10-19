# 进程状态

上节课我们讲了三个操作系统中的通用进程状态，接下来我们将说一说Linux这个具体的操作系统的进程状态。

Linux内核中定义了7种进程状态

```cpp
/*
* The task state array is a strange "bitmap" of
* reasons to sleep. Thus "running" is zero, and
* you can test for combinations of others with
* simple bit tests.
*/
static const char * const task_state_array[] = {
"R (running)", /* 0 */
"S (sleeping)", /* 1 */
"D (disk sleep)", /* 2 */
"T (stopped)", /* 4 */
"t (tracing stop)", /* 8 */
"X (dead)", /* 16 */
"Z (zombie)", /* 32 */
};
```

为了方面描述，下文把描述进程的`task_struct`对象和及其构成的数据结构统称为`PCB`。

R，即运行状态，此时进程处于运行队列中，由于CPU的速度很快，并且进程的切换速度也很快，所以看起来就像是在一直运行。比如下面的程序：

```cpp
#include<iostream>

int main()
{
	while (1);
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ls
makefile  PracessState
[wind@starry-sky Debug]$ make run
```

上节课中我们主要用的是`ps`指令，这回我们先用`top`指令看一看：

![image-20241018194906613](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410181949906.png)

我们看到第一个就是，正在全速运行，占了CPU`99.7%`的利用率。不过这样不太方便观察，所以我们还是用回`ps`命令。

```shell
[wind@starry-sky ~]$ ps ajx | head -1 ; ps ajx | grep PracessState | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
31490 31491 31490 27212 pts/1    31490 R+    1002   7:09 ./PracessState
[wind@starry-sky ~]$
```

其中`STAT`那列便是进程的状态。我们看到`ps`的运行状态描述比`top`更详细，`top`光是`R`，而`ps`则是`R+`，什么叫`R+`呢？`R+`的意思是前台运行，前台运行会占用`bash`界面，导致`bash`界面无法响应其它指令：

```shell
[wind@starry-sky Debug]$ make run
clear
clear
ls
pwd

```

除此之外，还有后台运行，它用`R`来表示，此时`bash`就可以执行其它指令了。后台运行的方式是执行指令后面加个`&`：
```shell
clear
clear
ls
pwd
^Cmake: *** [run] Interrupt

[wind@starry-sky Debug]$ ./PracessState &
[1] 32715
[wind@starry-sky Debug]$ ps ajx | head -1 ; ps ajx | grep PracessState | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27212 32715 32715 27212 pts/1      336 R     1002   1:40 ./PracessState
[wind@starry-sky Debug]$ ls
makefile  PracessState
[wind@starry-sky Debug]$
```

不过此时`ctrl c`就终止不了这个程序了，所以系统把该进程的PID打印出来，方便你进行管理，比如使用`kill`直接手动终止：
```shell
[wind@starry-sky Debug]$ kill -9 32715
[wind@starry-sky Debug]$ ps ajx | head -1 ; ps ajx | grep PracessState | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
[1]+  Killed                  ./PracessState
[wind@starry-sky Debug]$
```

`top`可就不分什么前后台了。

--------------

稍微改一下代码，状态可就不一样了。

```cpp
#include<iostream>

int main()
{
	while(1)
	{
		std::cout << "sleeping" << std::endl;
	}
	return 0;
}
```

由于要在屏幕上打印信息，它不可避免地要使用硬件资源，比如屏幕；当然，我用的是云服务器，所以最起码要用网卡，把屏幕操作通过网络传到我本地的Xshell上打印字符。总之，这个进程要频繁地使用硬件资源，而硬件的操作速度相比CPU又很慢，故而该进程的绝大多数时间都是在等待硬件资源配合，所以查看它的状态极大都可能是`sleeping`。

```shell
[wind@starry-sky ~]$ while :; do ps ajx | head -1 ; ps ajx | grep PracessState | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:02 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:02 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:03 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:03 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:03 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 R+    1002   1:03 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:03 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 R+    1002   1:04 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:04 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 R+    1002   1:05 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 R+    1002   1:05 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:05 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:05 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:06 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:06 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 R+    1002   1:06 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:06 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:07 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:07 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:07 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:08 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1820  1821  1820 27212 pts/1     1820 S+    1002   1:08 ./PracessState
^C
[wind@starry-sky ~]$
```

所以说`sleep`其实就是一种阻塞状态，处于此状态的进程在等待其它资源的响应，还可以来一个更明显的等待：

```cpp
#include<iostream>

int main()
{
	int a = 0;
	std::cin >> a;
	std::cout << a << std::endl;
	return 0;
}
```

如果不输入数字，该进程就一直等待，一直睡眠。

```shell
[wind@starry-sky ~]$ while :; do ps ajx | head -1 ; ps ajx | grep PracessState | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 3416  3417  3416 27212 pts/1     3416 S+    1002   0:00 ./PracessState
```

-------------------------

第三个也是睡觉`disk sleep`和第二个睡觉`sleep`有什么区别呢？`disk sleep`也叫深度睡眠，`sleep`则是浅度睡眠，`S`对外界是有响应的，睡得不是特别死，也可以随时`kill`该进程，`D`则是一种处于保护状态的睡眠，在该模式下，无论是用户的`kill`指令还是操作系统都完全无法关闭该进程，同时该进程也几乎不会响应其它任何指令，直到得到某个关键信号或者用户关电源，为什么`disk sleep`叫磁盘睡眠呢？因为它和磁盘有很大关系。

如果某个进程正在处理某些十分重要的数据，比如涉及金额上亿的银行交易记录，很明显这种级别的数据比系统更重要，无论如何这些数据都不能丢失，现在，这个进程已经把这些数据处理完毕了，它向磁盘提出了存储数据的请求，于是磁盘就开始存交易记录了，不过，由于数据量比较大，这磁盘又用久了，存数据比较慢，所以这些数据就要存不少时间，于是，进程就在这等着磁盘存数据，最起码如果等会发现存储失败可以报个警什么的，于是，这个进程就进入了`sleep`状态，等待磁盘的存储结果。

恰巧，此时内存就不够用了，于是操作系统就磨刀霍霍向进程，恰巧发现了上面的进程，发现它一直在`sleep`，什么事也不干，空占着内存，于是就把它干掉了；结果，运气太差了，磁盘存储失败了，想找进程汇报时，发现进程没了，磁盘又不知道这些数据的重要性，于是就把数据扔了。

为了避免这种情况的再次发生，就有了`disk sleep`，进入`disk sleep`状态的进程，除非拔电源，或者得到结果了，否则都不响应外界的任何指令，包括像`kill`这种杀进程的指令。

一般情况下，操作系统里不会出现，最起码不会出现长时间处于`disk sleep`状态的进程，如果`disk sleep`状态都能被用户看到了，那此时的操作系统离崩溃已经不远了，应该立刻检查修复。

`desk sleep`也是阻塞的一种。

-----------------

`kill -l`可以查看`kill`的选项：

```shell
[wind@starry-sky ~]$ kill -l
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
[wind@starry-sky ~]$
```

其中`19`就是停止选项，而`18`是解除停止：

```shell
[wind@starry-sky ~]$ ps ajx | head -1 ; ps ajx | grep PracessState | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379  7380  7379 27212 pts/1     7379 S+    1002   0:00 ./PracessState
[wind@starry-sky ~]$ kill -19 7380
[wind@starry-sky ~]$ ps ajx | head -1 ; ps ajx | grep PracessState | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379  7380  7379 27212 pts/1     7379 T+    1002   0:00 ./PracessState
[wind@starry-sky ~]$ kill -18 7380
[wind@starry-sky ~]$ ps ajx | head -1 ; ps ajx | grep PracessState | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379  7380  7379 27212 pts/1     7379 S+    1002   0:00 ./PracessState
[wind@starry-sky ~]$
```

1:05：10

# 完