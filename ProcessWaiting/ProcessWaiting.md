# ProcessWaiting

## why

之前我们提到，父进程创建子进程的目的是让子进程执行特定的功能。无论子进程是未完成任务就终止、完成任务但出错，还是成功完成任务，父进程都需要获得子进程的反馈。系统在子进程终止后，会回收其代码和数据，但保留其进程控制块（PCB），因为PCB中存储着子进程的退出信息，以便向父进程反馈任务的完成情况。如果父进程不及时处理这些信息，子进程将一直处于僵死状态，导致系统级别的内存泄露。因此，即使父进程对子进程的退出情况不关心，也必须在子进程终止后回收其资源，这就是需要进程等待的原因。

## what

简而言之，进程等待是指父进程通过系统接口（如 `wait` 或 `waitpid`）检测和回收子进程的退出状态。

## how

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
	pid_t id = fork();
	if (id < 0)
	{
		// fork调用失败
		// errno被设置

		// perror查看errno
		// 并转录为错误信息
		perror("fork failure:");
	}
	else if (id == 0)
	{
		// child
		int count = 3;
		while (count--)
		{
			printf("child process: pid->%d ppid->%d count:%d\n", getpid(), getppid(), count);
			sleep(1);
		}
		exit(0);
	}
	else
	{
		int count = 6;
		while (count--)
		{
			printf("parent process: pid->%d child pid->%d count:%d\n", getpid(), id, count);
			sleep(1);
		}
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ while :; do ps ajx | head -1 ; ps ajx | grep ProcessWaiting | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
13685 13691 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
13691 13692 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
13685 13691 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
13691 13692 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
13685 13691 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
13691 13692 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
13685 13691 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
13691 13692 13685  1922 pts/0    13685 Z+    1002   0:00 [ProcessWaiting] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
13685 13691 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
13691 13692 13685  1922 pts/0    13685 Z+    1002   0:00 [ProcessWaiting] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
13685 13691 13685  1922 pts/0    13685 S+    1002   0:00 ./ProcessWaiting
13691 13692 13685  1922 pts/0    13685 Z+    1002   0:00 [ProcessWaiting] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
^C
[wind@starry-sky Debug]$
```

```shell
[wind@starry-sky Debug]$ make run
parent process: pid->13445 child pid->13446 count:5
child process: pid->13446 ppid->13445 count:2------
parent process: pid->13445 child pid->13446 count:4
child process: pid->13446 ppid->13445 count:1------
parent process: pid->13445 child pid->13446 count:3
child process: pid->13446 ppid->13445 count:0------
parent process: pid->13445 child pid->13446 count:2
parent process: pid->13445 child pid->13446 count:1
parent process: pid->13445 child pid->13446 count:0
[wind@starry-sky Debug]$ vim code.c
[wind@starry-sky Debug]$ make run
parent process: pid->13691 child pid->13692 count:5
child process: pid->13692 ppid->13691 count:2------
parent process: pid->13691 child pid->13692 count:4
child process: pid->13692 ppid->13691 count:1------
parent process: pid->13691 child pid->13692 count:3
child process: pid->13692 ppid->13691 count:0------
parent process: pid->13691 child pid->13692 count:2
parent process: pid->13691 child pid->13692 count:1
parent process: pid->13691 child pid->13692 count:0
[wind@starry-sky Debug]$
```

我们看看系统接口`wait`

![image-20241102123133733](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411021231858.png)

我们先用`wait`，`wait`实际上是`waitpid`的精简版。我们先不关心参数`status`，它的返回值就是等待到的进程PID，我们先来看看被等待之后，子进程能不能被回收。

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t id = fork();
	if (id < 0)
	{
		perror("fork failure:");
	}
	else if (id == 0)
	{
		// child
		int count = 3;
		while (count--)
		{
			printf("child process: pid->%d ppid->%d count:%d------\n", getpid(), getppid(), count);
			sleep(1);
		}
		exit(0);
	}
	else
	{
    // parent
		int count = 6;
		while (count--)
		{
			printf("parent process: pid->%d child pid->%d count:%d\n", getpid(), id, count);
			sleep(1);
		}
    // status不考虑时设为NULL
    pid_t ret = wait(NULL);
    printf("wait->%d\n",ret);
    sleep(5);
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ while :; do ps ajx | head -1 ; ps ajx | grep ProcessWaiting | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
14720 14721 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
14720 14721 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
14720 14721 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
14720 14721 14719  1922 pts/0    14719 Z+    1002   0:00 [ProcessWaiting] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
14720 14721 14719  1922 pts/0    14719 Z+    1002   0:00 [ProcessWaiting] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
14720 14721 14719  1922 pts/0    14719 Z+    1002   0:00 [ProcessWaiting] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
14719 14720 14719  1922 pts/0    14719 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
^C
[wind@starry-sky Debug]$
```

```shell
[wind@starry-sky Debug]$ make run
parent process: pid->14720 child pid->14721 count:5
child process: pid->14721 ppid->14720 count:2------
parent process: pid->14720 child pid->14721 count:4
child process: pid->14721 ppid->14720 count:1------
parent process: pid->14720 child pid->14721 count:3
child process: pid->14721 ppid->14720 count:0------
parent process: pid->14720 child pid->14721 count:2
parent process: pid->14720 child pid->14721 count:1
parent process: pid->14720 child pid->14721 count:0
wait->14721
^Cmake: *** [run] Interrupt
```

`wait`的等待策略是，随机地等待子进程，下面我们直接创建五个子进程。

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
  printf("begin\n");
  printf("parent process ID->%d\n", getpid()); 
  int count = 5;
  while(count--)
  {
    pid_t id = fork();

    if(id == 0)
    {
      int i = 3;
      while(i--)
      {
        printf("child process: pid->%d i->%d\n", getpid(), i);
        sleep(1);
      }
      exit(0);
    }
    else if(id > 0)
    {
      printf("Successfully created, child process PID->%d\n", id);
    }
    else 
    {
      perror("failed");
    }
    sleep(1);
  }

  sleep(4);

  count = 5;
  while(count--)
  {
    pid_t ret = wait(NULL);

    if(ret > 0)
    {
      printf("Reclaim process resources:%d\n", ret);
    }
    else if(ret < 0)
    {
      perror("failed wait");
    }
    sleep(1);
  }
  printf("end\n");
  return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make ; make run
begin
parent process ID->17708
Successfully created, child process PID->17709
child process: pid->17709 i->2
Successfully created, child process PID->17711
child process: pid->17709 i->1
child process: pid->17711 i->2
child process: pid->17709 i->0
Successfully created, child process PID->17712
child process: pid->17711 i->1
child process: pid->17712 i->2
Successfully created, child process PID->17714
child process: pid->17712 i->1
child process: pid->17711 i->0
child process: pid->17714 i->2
child process: pid->17712 i->0
Successfully created, child process PID->17717
child process: pid->17714 i->1
child process: pid->17717 i->2
child process: pid->17714 i->0
child process: pid->17717 i->1
child process: pid->17717 i->0
Reclaim process resources:17709
Reclaim process resources:17711
Reclaim process resources:17712
Reclaim process resources:17714
Reclaim process resources:17717
end
```

在上面的代码中，在创建五个子进程后，我们特别`sleep(4)`，以确保所有子进程都退出后再`wait`，那如果子进程一直不退出呢？

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
  printf("begin\n");
  printf("parent process ID->%d\n", getpid()); 
  int count = 5;
  while(count--)
  {
    pid_t id = fork();

    if(id == 0)
    {
      // child
      int i = 3;
      while(1)
      {
        printf("child process: pid->%d i->%d\n", getpid(), i);
        sleep(1);
      }
      exit(0);
    }
    else if(id > 0)
    {
      printf("Successfully created, child process PID->%d\n", id);
    }
    else 
    {
      perror("failed");
    }
    sleep(1);
  }

  //sleep(4);
  printf("wait begin\n");
  count = 5;
  while(count--)
  {
    pid_t ret = wait(NULL);

    if(ret > 0)
    {
      printf("Reclaim process resources:%d\n", ret);
    }
    else if(ret < 0)
    {
      perror("failed wait");
    }
    sleep(1);
  }
  printf("end\n");
  return 0;
}
```

此时当`wait begin`之后，没有进程退出，父进程就会一直卡在`wait`接口处，换言之，它进入阻塞状态了，父进程在等待子进程的响应，我们前面说过，阻塞状态，是进程得不到某个软硬件资源后进入的状态，前面我们大多看的都是等待硬件，今天我们看到了等待软件。

```shell
[wind@starry-sky Debug]$ make run
begin
parent process ID->18702
Successfully created, child process PID->18703
child process: pid->18703 i->3
child process: pid->18703 i->3
Successfully created, child process PID->18711
child process: pid->18711 i->3
child process: pid->18703 i->3
child process: pid->18711 i->3
Successfully created, child process PID->18719
child process: pid->18719 i->3
child process: pid->18703 i->3
child process: pid->18711 i->3
child process: pid->18719 i->3
Successfully created, child process PID->18726
child process: pid->18726 i->3
child process: pid->18703 i->3
child process: pid->18711 i->3
child process: pid->18719 i->3
child process: pid->18726 i->3
Successfully created, child process PID->18734
child process: pid->18734 i->3
child process: pid->18703 i->3
child process: pid->18711 i->3
child process: pid->18719 i->3
child process: pid->18726 i->3
wait begin
child process: pid->18734 i->3
child process: pid->18703 i->3
child process: pid->18711 i->3
child process: pid->18719 i->3

```

```shell
[wind@starry-sky Debug]$ while :; do ps ajx | head -1 ; ps ajx | grep ProcessWaiting | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18701 18702 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18703 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18701 18702 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18703 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18711 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18701 18702 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18703 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18711 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18719 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18701 18702 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18703 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18711 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18719 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18726 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18701 18702 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18703 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18711 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18719 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18726 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18734 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18701 18702 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18703 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18711 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18719 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18726 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18734 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18701 18702 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18703 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18711 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18719 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18726 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18734 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18701 18702 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18703 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18711 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18719 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18726 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
18702 18734 18701  1922 pts/0    18701 S+    1002   0:00 ./ProcessWaiting
^C
[wind@starry-sky Debug]$ 

```

在后面，父进程进入了`S+`状态，这是因为它迟迟等不到子进程的响应（因为子进程迟迟不退出），所以进入了阻塞状态，而其它子进程，因为大部分时间都在`sleep(1)`，所以也在`S+`状态。

-----------------------------

接下来我们说`waitpid`，它有三个参数`pid_t pid, int *status, int options`，第一个参数`pid`可以指定想要等待的子进程，这样就可以对特定子进程进行等待；如果不想指定，可以设为-1，此时`waitpid`就相当于`wait`，随机等待子进程；第二个参数`status`是个整型指针，有32个比特位，它其实在`waitpid/wait`中不是作为一个整型，而是作为几个部分存在的，不同的部分存着进程退出的相关信息；第三个参数`options`先不用管，它是控制`waitpid`的运行模式的。

```c
int main()
{
	pid_t id = fork();
	if (id < 0)
	{
		perror("fork failure:");
	}
	else if (id == 0)
	{
		int count = 3;
		while (count--)
		{
			printf("child process: pid->%d ppid->%d count:%d------\n", getpid(), getppid(), count);
			sleep(1);
		}
		exit(0);
	}
	else
	{
		int count = 6;
		while (count--)
		{
			printf("parent process: pid->%d child pid->%d count:%d\n", getpid(), id, count);
			sleep(1);
		}

    pid_t ret = waitpid(-1, NULL, 0);
    printf("wait->%d\n",ret);
    sleep(5);
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make ; make run
parent process: pid->20637 child pid->20638 count:5
child process: pid->20638 ppid->20637 count:2------
parent process: pid->20637 child pid->20638 count:4
child process: pid->20638 ppid->20637 count:1------
parent process: pid->20637 child pid->20638 count:3
child process: pid->20638 ppid->20637 count:0------
parent process: pid->20637 child pid->20638 count:2
parent process: pid->20637 child pid->20638 count:1
parent process: pid->20637 child pid->20638 count:0
wait->20638
```

`waitpid`在内部会对`status`指向的32比特位进行修改，从而让父进程获得子进程的退出状态，我们目前只看<span style="color: red;">低16位</span>：0000 0000 0000 0000 <span style="color: red;">0000 0000 0000 0000</span> 

<span style="color: yellow;">低7位</span>(0000 0000 0000 0000 0000 0000 0<span style="color: yellow;">000 0000</span>)存储着代码的异常退出信息编号，就是`kill -l`中的那些信息：

```shell
[wind@starry-sky Debug]$ kill -l
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
[wind@starry-sky Debug]$
```

<span style="color: green;">第八位</span>(0000 0000 0000 0000 0000 0000 <span style="color: green;">0</span>000 0000)是个特殊标志位，`core dump`标志位，目前不在我们的考虑范围内。

<span style="color: orange;">九到十六位</span>(0000 0000 0000 0000 <span style="color: orange;">0000 0000</span> 0000 0000)存储着进程的退出码。

```c
int main()
{
	pid_t id = fork();
	if (id < 0)
	{
		perror("fork failure:");
	}
	else if (id == 0)
	{
		int count = 3;
		while (count--)
		{
			printf("child process: pid->%d ppid->%d count:%d------\n", getpid(), getppid(), count);
			sleep(1);
		}
    sleep(10);
		exit(0);
	}
	else
	{
		int count = 5;
		while (count--)
		{
			printf("parent process: pid->%d child pid->%d count:%d\n", getpid(), id, count);
			sleep(1);
		}
 
    sleep(10);
    printf("waitpid begin\n");
    int status = 0;
    pid_t ret = waitpid(id, &status, 0);
    printf("wait->%d\n",ret);
    printf("Error message : %d\n", status & 0x7f);
    printf("Exit code : %d\n", (status >> 8) & 0xff);
    printf("waitpid end\n");
    sleep(3);
	}
	return 0;
}

```

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411021506822.mp4"></video>

不过我们对位操作不太熟，可以用相应的宏`WIFEXITED(status)`和`WEXITSTATUS(status)`

`WIFEXITED(status)`底层是个宏函数，当异常信息编号为0，也就是没有异常信息时，它会返回真，而`WEXITSTATUS(status)`就是退出码，异常退出也有自己的退出码宏`WTERMSIG(status)`。

```c
int main()
{
    pid_t id = fork();
    if (id < 0)
    {
        perror("fork failure:");
    }
    else if (id == 0)
    {
        int count = 3;
        while (count--)
        {
            printf("child process: pid->%d ppid->%d count:%d------\n", getpid(), getppid(), count);
            sleep(1);
        }
        sleep(10);
        exit(0);
    }
    else
    {
        int count = 5;
        while (count--)
        {
            printf("parent process: pid->%d child pid->%d count:%d\n", getpid(), id, count);
            sleep(1);
        }

        sleep(10);
        printf("waitpid begin\n");
        int status = 0;
        pid_t ret = waitpid(id, &status, 0);
        printf("wait->%d\n",ret);
        if(WIFEXITED(status))
        {
            printf("Normal exit, code : %d\n", WEXITSTATUS(status));
        }
        else 
        {
            printf("Exception Interrupt, code : %d\n", WTERMSIG(status));
        }
        //printf("Error message : %d\n", status & 0x7f);
        //printf("Exit code : %d\n", WEXITSTATUS(status));
        printf("waitpid end\n");
        sleep(3);
    }
    return 0;
}
```

进程的代码和数据都在用户层，当父进程调用`waitpid`时，系统就会到内核层去寻找子进程的PCB，如果子进程结束了，它会把退出信息存储在PCB中，此时系统就能获取子进程的退出信息并输入到父进程的代码和数据中，如果子进程没有结束，系统就会把父进程的PCB指针放在子进程的等待队列中，于是，父进程就阻塞了，当子进程结束后，系统就会从子进程PCB中的等待队列中获取等待子进程配合的父进程，从而唤醒它，并把子进程退出信息输入给父进程。

我们可以从LINUX内核原码中`.\linux\include\linuxsched.h`中看到PCB，也就是`task_struct`中的异常信息编号和退出码。

![image-20241102155434456](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411021554039.png)

![image-20241102155455614](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411021554187.png)

随口说一声，因为进程具有独立性，所以只能通过系统调用接口获取子进程退出码。比如不可以用全局变量作退出码（子进程退出时，把退出码赋给这个全局变量，父进程再查看这个全局变量，从而获取子进程退出码），因为这个全局变量只是虚拟地址相同而已，其物理地址已经不相同了，当子进程写入全局变量时，就会发生写时拷贝。父进程也不能直接接触子进程获得退出码，如果父进程可以直接访问子进程PCB，进程之间就会存在相互干扰的可能，这样也是不行的，系统不允许两个进程直接接触，必须要有中间层。

-----------------

接下来我们讨论第三个参数 `options`。`waitpid` 有两种运行模式：当 `options == 0` 时，采用阻塞等待模式。如果子进程仍在运行，父进程会被添加到子进程的 PCB 阻塞队列，直到子进程结束后才恢复运行。相反，当 `options != 0`（通常为宏 `WNOHANG`）时，采用非阻塞轮询模式。如果子进程仍在运行，`waitpid` 将返回 0，父进程不会进入阻塞队列。此时，父进程可以先执行其他操作，稍后再调用 `waitpid`，不断循环，直到子进程结束， `waitpid` 返回子进程的 `pid`。

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<string.h>

void other()
{
    printf("Other operations\n");
}

int main()
{
    printf("begin\n");
    printf("parent process ID->%d\n", getpid()); 
    int count = 5;
    while(count--)
    {
        pid_t id = fork();

        if(id == 0)
        {
            // child
            int i = 10;
            while(i--)
            {
                printf("child process: pid->%d i->%d\n", getpid(), i);
                sleep(1);
            }
            exit(0);
        }
        else if(id > 0)
        {
            printf("Successfully created, child process PID->%d\n", id);
        }
        else 
        {
            perror("failed");
        }
        sleep(1);
    }

    //sleep(4);
    printf("wait begin\n");
    count = 5;
    while(count)
    {

        int status = 0;
        pid_t ret = waitpid(-1, &status, WNOHANG);

        if(ret > 0)
        {
            count--;
            printf("Reclaim process resources:%d\n", ret);
            if(WIFEXITED(status))
            {
                printf("Normal exit, code : %d\n", WEXITSTATUS(status));
            }
            else 
            {
                printf("Exception Interrupt, code : %d\n", WTERMSIG(status));
            }

        }
        else if(ret < 0)
        {
            perror("failed wait");
        }
        else 
        {
            other();
        }
        sleep(1);
    }
    printf("end\n");
    return 0;
}
```

```shell
[wind@starry-sky Debug]$ make run
begin
parent process ID->30910
Successfully created, child process PID->30911
child process: pid->30911 i->9
Successfully created, child process PID->30913
child process: pid->30911 i->8
child process: pid->30913 i->9
child process: pid->30911 i->7
Successfully created, child process PID->30914
child process: pid->30914 i->9
child process: pid->30913 i->8
child process: pid->30911 i->6
Successfully created, child process PID->30916
child process: pid->30914 i->8
child process: pid->30913 i->7
child process: pid->30916 i->9
child process: pid->30911 i->5
child process: pid->30914 i->7
child process: pid->30913 i->6
Successfully created, child process PID->30918
child process: pid->30916 i->8
child process: pid->30918 i->9
child process: pid->30911 i->4
child process: pid->30914 i->6
child process: pid->30913 i->5
wait begin
child process: pid->30916 i->7
Other operations
child process: pid->30918 i->8
child process: pid->30911 i->3
child process: pid->30914 i->5
child process: pid->30913 i->4
child process: pid->30916 i->6
Other operations
child process: pid->30918 i->7
child process: pid->30911 i->2
child process: pid->30914 i->4
child process: pid->30913 i->3
child process: pid->30916 i->5
Other operations
child process: pid->30918 i->6
child process: pid->30911 i->1
child process: pid->30914 i->3
child process: pid->30913 i->2
child process: pid->30916 i->4
Other operations
child process: pid->30918 i->5
child process: pid->30911 i->0
child process: pid->30914 i->2
child process: pid->30913 i->1
child process: pid->30916 i->3
Other operations
child process: pid->30918 i->4
child process: pid->30913 i->0
child process: pid->30914 i->1
child process: pid->30916 i->2
Reclaim process resources:30911
Normal exit, code : 0
child process: pid->30918 i->3
child process: pid->30914 i->0
child process: pid->30918 i->2
Other operations
child process: pid->30916 i->1
child process: pid->30918 i->1
Reclaim process resources:30913
Normal exit, code : 0
child process: pid->30916 i->0
child process: pid->30918 i->0
Reclaim process resources:30914
Normal exit, code : 0
Reclaim process resources:30916
Normal exit, code : 0
Reclaim process resources:30918
Normal exit, code : 0
end
```

什么时候`waitpid`会失败呢？也就是返回值小于0的时候，此时可能`waitpid`的第一个参数`pid`进程不存在，或者不是这个父进程的子进程，是别的父进程的子进程。

------------------------

下面我们大概说一说非阻塞轮询的时候父进程是怎么干其它事的：

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<string.h>

void task1()
{
    //之后学习进程通信之后
    //可以让子进程报告进度
    //从而显示进度
}

void task2()
{
    //学了日志系统之后
    //可以顺便写写日志
}

void task3()
{
    //打印网络信息
}

void other()
{
    printf("Other operations\n");
}
// 总之，只是在等子进程的时候
// 顺手做点任务而已
// 所以，一般都是小任务
// 复杂度不高的
// 是一些没有子进程重要
// 的小任务

//对于C语言来说
//使用函数指针的方式对它们进行统一管理

//以宏的方式，定义函数指针元素个数
#define TASK_NUM 10

//对函数指针类型进行类型重命名
typedef void (*task)();

//定义一个函数指针数组
task task_arry[TASK_NUM];

//初始化
void Init_task()
{
    int i = 0;
    for(; i < TASK_NUM ; i++)
    {
        task_arry[i] = NULL;
    }
    return;
}

//添加任务
int push_task(task t)
{
    //寻找数组中空的位置
    int pos = 0;
    for(; pos < TASK_NUM; pos++)
    {
        //如果内容不为空，就跳过
        if(task_arry[pos]) continue;
        else break;
    } 
    if(pos == TASK_NUM)
        return -1;
    task_arry[pos] = t;
    return 0;
}

//执行任务
void execute_task()
{
    int pos = 0;
    for(pos =0; pos < TASK_NUM; pos++)
    {
        if(task_arry[pos]) task_arry[pos]();
    }
}


int main()
{
    printf("begin\n");
    printf("parent process ID->%d\n", getpid()); 
    int count = 5;
    while(count--)
    {
        pid_t id = fork();

        if(id == 0)
        {
            // child
            int i = 10;
            while(i--)
            {
                printf("child process: pid->%d i->%d\n", getpid(), i);
                sleep(1);
            }
            exit(0);
        }
        else if(id > 0)
        {
            printf("Successfully created, child process PID->%d\n", id);
        }
        else 
        {
            perror("failed");
        }
        sleep(1);
    }

    //sleep(4);
    printf("wait begin\n");
    Init_task();
    push_task(other);
    push_task(task1);
    push_task(task2);
    push_task(task3);
    count = 5;
    while(count)
    {

        int status = 0;
        pid_t ret = waitpid(-1, &status, WNOHANG);

        if(ret > 0)
        {
            count--;
            printf("Reclaim process resources:%d\n", ret);
            if(WIFEXITED(status))
            {
                printf("Normal exit, code : %d\n", WEXITSTATUS(status));
            }
            else 
            {
                printf("Exception Interrupt, code : %d\n", status & 0x7f);
            }

        }
        else if(ret < 0)
        {
            perror("failed wait");
        }
        else 
        {
            execute_task(); 
        }
        sleep(1);
    }
    printf("end\n");
    return 0;
}
```

这里只是稍微写写，表现一下大致意思。以后我们可能更多用C++，直接写个类来进行管理，通过这种方式，就能较为便捷地通过`push_task`插入子任务，让父程序在等待的间隙执行一些其它轻量化的任务。

# end