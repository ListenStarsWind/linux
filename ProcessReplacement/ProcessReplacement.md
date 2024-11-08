# ProcessReplacement

## feel

在Linux中，有七个接口用于进程替换，其中六个在3号手册，还有一个在1号手册。我们先不管那么多，先直接写一个单进程版本的进程替换，直观地感受一下进程替换的效果，接口的具体说明将在`interface`中详细说明。

![image-20241104175407495](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411041754728.png)

![image-20241104203742209](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411042037338.png)

execute a file，其中的file必须是可以直接或者间接执行的文件，并且用户有对应的执行程序。

我们先使用`execl`，可以看到它的参数是可变的，就像`printf`那样，它的参数个数是可以变化的，其中的第一个参数`path`，用于指示`file`的位置，路径，而后面的参数，则是该文件执行的命令行参数，命令行参数的个数不一定，所以`execl`被设计成了可变参数的样子，需要注意的是，最后一个`arg`必须是`NULL`，此处的`NULL`就像是停止符，意思是说后面没有更多内容了，不要再往后找了，另一方面，这种形式也适合`argv[]`，还记得吗？之前我们说命令行参数的时候，也说了`argv[]`的最后一个元素是`NULL`。

```cpp
#include<iostream>
#include<unistd.h>

using namespace std;

int main()
{
	cout << "replacement before: " << "pid->" << getpid() << endl;

	// C接口不一定兼容C++，最后一个参数最好还是NULL
	execl("/usr/bin/ls", "ls", "-a", "-l", NULL);

	cout << "replacement after: " << "pid->" << getpid() << endl;

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
code.cpp  makefile  ProcessReplacement
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->23144
total 24
drwxrwxr-x 2 wind wind 4096 Nov  4 18:19 .
drwxrwxr-x 3 wind wind 4096 Nov  4 17:33 ..
lrwxrwxrwx 1 wind wind   33 Nov  4 17:35 code.cpp -> ./../../../ProcessReplacement.cpp
-rw-rw-r-- 1 wind wind  171 Nov  4 17:38 makefile
-rwxrwxr-x 1 wind wind 9144 Nov  4 18:19 ProcessReplacement
[wind@starry-sky Debug]$
```

我们发现`ProcessReplacement`在输出`replacement before: pid->23144`之后，执行了`ls -a -l`，并且`replacement after: pid->23144`并没有打印出来，或者说，`execl`后面的代码没有被执行。

## principle

下面我们直接谈谈单进程中`execl`的`principle`【原理】。我们知道，当源代码被编译器编译为可执行程序之后，就会被存到磁盘里，而当我们向`bash`输入`./ProcessReplacement`后，`bash`一方面会通过系统调用，创建进程控制块，进程地址空间，页表等数据结构，然后在磁盘上找到`ProcessReplacement`的二进制文件，把它加载到物理内存中，由于二进制可执行文件中的数据并不是杂乱无章地放的，而是以`ELF`的格式组织的，最开始的是表头，它存放着文件的一些最基本信息，比如第一行代码在文件中的具体位置，除此之外还有其它的一些部分，系统就会根据这些信息初始化进程控制块，进程地址空间，页表的某些成员，然后再让实际的物理地址与对应的虚拟地址建立起联系，这样，一个进程就被创建出来了。而当`execl`执行后，它会依据我们给的路径，在磁盘上找到需要被调用的文件，然后以替换原进程代码数据的方式把它加载到内存中，由于原来的代码被替换掉了，所以自然不会输出`replacement after: pid->23144`，之后对进程控制块，地址空间，页表的局部做些调整，以及让虚拟地址与物理地址在页表上建立起映射关系，然后从新加载的代码的起始位置开始执行。

下面我们看看多进程版本的情况

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		exit(0);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ while :; do ps ajx | head -1 ; ps ajx | grep ProcessReplacement | grep -v grep; sleep 1; done
```

```shell
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->30342
total 28
drwxrwxr-x 2 wind wind  4096 Nov  4 19:31 .
drwxrwxr-x 3 wind wind  4096 Nov  4 17:33 ..
lrwxrwxrwx 1 wind wind    33 Nov  4 17:35 code.cpp -> ./../../../ProcessReplacement.cpp
-rw-rw-r-- 1 wind wind   171 Nov  4 17:38 makefile
-rwxrwxr-x 1 wind wind 13528 Nov  4 19:31 ProcessReplacement
reclaim process resources:30343
replacement after: pid->30342
[wind@starry-sky Debug]$
```

```shell
[wind@starry-sky Debug]$ while :; do ps ajx | head -1 ; ps ajx | grep ProcessReplacement | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
26145 30342 30342 26145 pts/0    30342 S+    1002   0:00 ./ProcessReplacement
30342 30343 30342 26145 pts/0    30342 S+    1002   0:00 ./ProcessReplacement
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
26145 30342 30342 26145 pts/0    30342 S+    1002   0:00 ./ProcessReplacement
30342 30343 30342 26145 pts/0    30342 S+    1002   0:00 ./ProcessReplacement
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
26145 30342 30342 26145 pts/0    30342 S+    1002   0:00 ./ProcessReplacement
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
26145 30342 30342 26145 pts/0    30342 S+    1002   0:00 ./ProcessReplacement
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
^C
[wind@starry-sky Debug]$
```

`execl` 对于子进程的替换与单进程的替换过程相似，但这次替换只影响子进程特有的部分，而不会替换共享的代码和数据。之后，系统会对相关的数据结构进行局部调整。从某种角度来看，这实际上可以被视为一种写时拷贝。此时，子进程仍然是父进程的子进程，但由于代码和数据内容的显著不同，它们之间的关系看起来就像是没有血缘关系的父子。父进程的代码没有被替换，自然还能输出`after`信息。

从监控脚本上也可以看出，`execl`是进程替换，而不是创建进程，子进程的PID没有发生变化，子进程还是那个子进程，只不过内部不同了。

由于使用 `execl` 的进程原有的代码和数据会被替换，因此我们可以通过检查 `execl` 后是否执行了新的代码来判断其是否成功。成功后，原有的代码和数据被替换，因此 `execl` 不会有成功的返回值（因为原代码已经消失，无法返回给谁）。只会有一个失败的返回值，然而，这种返回值通常不常用。一般来说，我们直接通过检查 `execl` 后的代码是否执行来判断其成功与否。换句话说，`execl` 后的代码应该是错误处理代码，例如 `perror("exec failed"); exit(1);`。这种处理方式适用于所有 `exec` 系列的接口。

## interface

现在我们回到这张图，来说说`exec`系列接口

![image-20241104175407495](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411041754728.png)

`execl` 中的 `l` 代表 `list`，后面的参数就像一个个节点，分散在内存的不同位置。第一个参数 `path` 是一个文件路径，指示要执行的文件的位置。这一过程类似于 `bash` 执行 `ls` 命令时，首先会在环境变量 `PATH` 中的各个路径下寻找 `ls`，只有找到后，才能将其加载到物理内存中。实际上，`/usr/bin/` 就是 `PATH` 中的一个路径。

```shell
[wind@starry-sky Debug]$ echo $PATH
/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
[wind@starry-sky Debug]$
```

从第二个参数开始，则是命令行参数，和我们向`bash`界面输入的内容大致相同，只不过它是以字符串而不是空格的形式分割开来。

```shell
[wind@starry-sky Debug]$ ls -a -l
total 28
drwxrwxr-x 2 wind wind  4096 Nov  4 19:31 .
drwxrwxr-x 3 wind wind  4096 Nov  4 17:33 ..
lrwxrwxrwx 1 wind wind    33 Nov  4 17:35 code.cpp -> ./../../../ProcessReplacement.cpp
-rw-rw-r-- 1 wind wind   171 Nov  4 17:38 makefile
-rwxrwxr-x 1 wind wind 13528 Nov  4 19:31 ProcessReplacement
[wind@starry-sky Debug]$
```

`execlp` 中的 `p` 代表 `path`，这意味着 `execlp` 默认会使用环境变量中的 `PATH`，用户只需提供文件名，而不必指定具体路径。`execlp` 会在 `PATH` 中的各个路径下逐个寻找该文件。不过，用户也可以直接传递具体路径，这样 `execlp` 将使用提供的路径，而不再从 `PATH` 中查找。其他参数与 `execl` 相同。

```cpp
execlp("ls", "ls", "-a", "-l", NULL);
```

`execv` 中的 `v` 代表 `vector`，尽管翻译为向量，但实际上它是一个数组。`execv` 将命令行参数从可变参数形式转换为数组，最后一个元素仍然是 `NULL`，与我们之前学习的命令行参数一样。此外，`execv` 不带 `p`，意味着它不使用 `PATH`，需要提供完整的路径。

实际上，如果命令行参数以可变参数的形式输入，最终也会转换为数组形式，这与我们之前通过 `main` 函数接收命令行参数的方式类似。

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		char* const myargv[] = {"ls", "-a", "-l", NULL};
		execv("/usr/bin/ls", myargv);

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

`execvp`既带`v`，也带`p`，这意味着，它的第一个参数不用指定具体路径，可以只有文件名，它的命令行参数以字符数组而非可变参数的形式传参。

`execle` 中的 `e` 代表 `environment`，即环境变量。这意味着，使用带 `e` 的 `exec` 可以显式地传递环境变量。实际上，所有 `exec` 系列接口都传递了环境变量，区别在于，带 `e` 的版本允许显式指定环境变量，而不带 `e` 的则是隐式传递环境变量。接下来，我们将调用自己编写的可执行程序，以探讨进程替换中的环境变量继承关系。

-----------

我们再写一个 C++ 程序。需要说明的是，Linux 中的 C 源代码可以使用 `.cpp`、`.cc` 和 `.cxx` 这三种后缀。为了便于区分，我们的新代码将使用 `.cxx` 作为后缀。

```cpp
#include<iostream>    

using namespace std;    

int main()    
{    
    cout<<"hello Linux"<<endl;                                                                                             return 0;               
}        
```

接下来，我们来看看如何使用 `makefile` 一次性编译出两个可执行程序。之前提到过，`makefile` 会将第一个指令视为默认指令，因此当输入 `make` 时，它会根据这个指令编译生成输出文件。

```shell
[wind@starry-sky Debug]$ ls
code.cpp  code.cxx  makefile  ProcessReplacement
[wind@starry-sky Debug]$ cat makefile
ProcessReplacement:./../../../ProcessReplacement.cpp
	@g++ $^ -o $@
.PHONY:clean
clean:
	@rm -rf ProcessReplacement ProcessReplacement.out
execute:
	@./ProcessReplacement
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
code.cpp  code.cxx  makefile
[wind@starry-sky Debug]$ make
../../../ProcessReplacement.cpp: In function ‘int main()’:
../../../ProcessReplacement.cpp:22:49: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
   char* const myargv[] = {"ls", "-a", "-l", NULL};
                                                 ^
../../../ProcessReplacement.cpp:22:49: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
../../../ProcessReplacement.cpp:22:49: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
[wind@starry-sky Debug]$ ls
code.cpp  code.cxx  makefile  ProcessReplacement
[wind@starry-sky Debug]$
```

那如何一次性编译出两个可执行程序呢？我们需要用到伪指令。

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
code.cpp  code.cxx  makefile
[wind@starry-sky Debug]$ cat makefile
.PHONY:all
all:out ProcessReplacement

out:code.cxx
	@g++ $^ -o $@
ProcessReplacement:./../../../ProcessReplacement.cpp
	@g++ $^ -o $@
.PHONY:clean
clean:
	@rm -rf ProcessReplacement ProcessReplacement.out out
execute:
	@./ProcessReplacement
[wind@starry-sky Debug]$ make
../../../ProcessReplacement.cpp: In function ‘int main()’:
../../../ProcessReplacement.cpp:22:49: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
   char* const myargv[] = {"ls", "-a", "-l", NULL};
                                                 ^
../../../ProcessReplacement.cpp:22:49: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
../../../ProcessReplacement.cpp:22:49: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
[wind@starry-sky Debug]$ ls
code.cpp  code.cxx  makefile  out  ProcessReplacement
[wind@starry-sky Debug]$
```

第一个指令`all`是个伪指令，它的依赖关系是`out ProcessReplacement`，没有依赖方法。但现在没有`out ProcessReplacement`，所以`makefile`就会往下遍历，找到得到`out`和`ProcessReplacement`的方法，然后执行它们，于是就可以一次性得到两个可执行程序。

现在让我们以`execv`的形式执行`out`

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		char* const myargv[] = {"out", NULL};
		execv("./out", myargv);

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
code.c  code.cpp  code.cxx  makefile
[wind@starry-sky Debug]$ make out
[wind@starry-sky Debug]$ ls
code.c  code.cpp  code.cxx  makefile  out
[wind@starry-sky Debug]$ make 
../../../ProcessReplacement.cpp: In function ‘int main()’:
../../../ProcessReplacement.cpp:22:38: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
   char* const myargv[] = {"out", NULL};
                                      ^
[wind@starry-sky Debug]$ ls
code.c  code.cpp  code.cxx  makefile  out  ProcessReplacement
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->22283
hello Linux
reclaim process resources:22284
replacement after: pid->22283
[wind@starry-sky Debug]$ ./out
hello Linux
[wind@starry-sky Debug]$
```

尽管在`bash`界面我们输入的是`./out`但在`argv`中，一般还是用`out`，`bash`中输入`./out`的原因是`out`不在`bash`的`PATH`路径下，所以同样需要为`bash`描述`out`的位置。

下面我们写一个简单的`shell`脚本

```shell
[wind@starry-sky Debug]$ vim test.sh
[wind@starry-sky Debug]$ cat test.sh
#!/usr/bin/bash 

echo "hello 1"
echo "hello 2"
echo "hello 3"
echo "hello 4"
echo "hello 5"

ls -a -l
[wind@starry-sky Debug]$ bash test.sh
hello 1
hello 2
hello 3
hello 4
hello 5
total 48
drwxrwxr-x 2 wind wind  4096 Nov  5 13:13 .
drwxrwxr-x 3 wind wind  4096 Nov  4 17:33 ..
-rw-rw-r-- 1 wind wind     0 Nov  5 08:14 code.c
lrwxrwxrwx 1 wind wind    33 Nov  4 17:35 code.cpp -> ./../../../ProcessReplacement.cpp
-rw-rw-r-- 1 wind wind    98 Nov  4 21:48 code.cxx
-rw-rw-r-- 1 wind wind   242 Nov  4 21:56 makefile
-rwxrwxr-x 1 wind wind  8968 Nov  5 12:54 out
-rwxrwxr-x 1 wind wind 13528 Nov  5 12:55 ProcessReplacement
-rw-rw-r-- 1 wind wind   103 Nov  5 13:13 test.sh
[wind@starry-sky Debug]$
```

`Shell`脚本本身不能直接执行，它需要通过对应的解释器来运行。因此，在`.sh`文件的开头，需要指定解释器的路径。简而言之，脚本是由一系列命令行组成的。当`bash`解释并执行`test.sh`时，它会逐条执行脚本中的命令。`Shell`语言并不是我们学习的重点，了解其基本概念即可。

下面我们来调用这个`shell`文件

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		//// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		//char* const myargv[] = {"out", NULL};
		//execv("./out", myargv);

		//真正执行的是脚本的解释器->bash
		char* const myargv[] = { "bash", "test.sh", NULL };
		execv("/usr/bin/bash", myargv);

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
code.c  code.cpp  code.cxx  makefile  out  test.sh
[wind@starry-sky Debug]$ make ProcessReplacement
../../../ProcessReplacement.cpp: In function ‘int main()’:
../../../ProcessReplacement.cpp:26:52: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
   char* const myargv[] = { "bash", "test.sh", NULL };
                                                    ^
../../../ProcessReplacement.cpp:26:52: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
[wind@starry-sky Debug]$ ls
code.c  code.cpp  code.cxx  makefile  out  ProcessReplacement  test.sh
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->24717
hello 1
hello 2
hello 3
hello 4
hello 5
total 48
drwxrwxr-x 2 wind wind  4096 Nov  5 13:28 .
drwxrwxr-x 3 wind wind  4096 Nov  4 17:33 ..
-rw-rw-r-- 1 wind wind     0 Nov  5 08:14 code.c
lrwxrwxrwx 1 wind wind    33 Nov  4 17:35 code.cpp -> ./../../../ProcessReplacement.cpp
-rw-rw-r-- 1 wind wind    98 Nov  4 21:48 code.cxx
-rw-rw-r-- 1 wind wind   242 Nov  4 21:56 makefile
-rwxrwxr-x 1 wind wind  8968 Nov  5 12:54 out
-rwxrwxr-x 1 wind wind 13528 Nov  5 13:28 ProcessReplacement
-rw-rw-r-- 1 wind wind   103 Nov  5 13:13 test.sh
reclaim process resources:24718
replacement after: pid->24717
[wind@starry-sky Debug]$ bash test.sh
hello 1
hello 2
hello 3
hello 4
hello 5
total 48
drwxrwxr-x 2 wind wind  4096 Nov  5 13:28 .
drwxrwxr-x 3 wind wind  4096 Nov  4 17:33 ..
-rw-rw-r-- 1 wind wind     0 Nov  5 08:14 code.c
lrwxrwxrwx 1 wind wind    33 Nov  4 17:35 code.cpp -> ./../../../ProcessReplacement.cpp
-rw-rw-r-- 1 wind wind    98 Nov  4 21:48 code.cxx
-rw-rw-r-- 1 wind wind   242 Nov  4 21:56 makefile
-rwxrwxr-x 1 wind wind  8968 Nov  5 12:54 out
-rwxrwxr-x 1 wind wind 13528 Nov  5 13:28 ProcessReplacement
-rw-rw-r-- 1 wind wind   103 Nov  5 13:13 test.sh
[wind@starry-sky Debug]$
```

调用`test.py`

```shell
[wind@starry-sky Debug]$ which python
/usr/bin/python
[wind@starry-sky Debug]$ python --version
Python 2.7.5
[wind@starry-sky Debug]$ vim test.py
[wind@starry-sky Debug]$ python test.py
hello python

[wind@starry-sky Debug]$ cat test.py
#/usr/bin/python2.7

print("hello python\n");
[wind@starry-sky Debug]$
```

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		//// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		//char* const myargv[] = {"out", NULL};
		//execv("./out", myargv);

		////真正执行的是脚本的解释器->bash
		//char* const myargv[] = { "bash", "test.sh", NULL };
		//execv("/usr/bin/bash", myargv);

		char* const myargv[] = { "python", "test.py", NULL };
		execv("/usr/bin/python", myargv);

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
code.c  code.cpp  code.cxx  makefile  test.py  test.sh
[wind@starry-sky Debug]$ make ProcessReplacement
../../../ProcessReplacement.cpp: In function ‘int main()’:
../../../ProcessReplacement.cpp:29:54: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
   char* const myargv[] = { "python", "test.py", NULL };
                                                      ^
../../../ProcessReplacement.cpp:29:54: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->25965
hello python

reclaim process resources:25966
replacement after: pid->25965
[wind@starry-sky Debug]$
```

对于所有的可执行程序，其在系统中都是以进程形式运行的，所以它们都可以互相调用。

-----------

下面我们就要正式探究进程替换中的环境变量继承关系了。

先改一改`code.cxx`，使其可以输出命令行参数和环境变量

```cpp
#include<iostream>

using namespace std;

int main(int argc, char* argv[], char* env[])
{
  cout<<"start execution"<<endl;
  int i = 0;

  cout<<endl<<"argv:"<<endl;
  for(i = 0; argv[i]; i++)
  {
    cout<<i<<"->"<<argv[i]<<endl;
  }

  cout<<endl<<"env:"<<endl;
  for(i = 0; env[i]; i++)
  {
    cout<<i<<"->"<<env[i]<<endl;
  }

  cout<<endl<<"execution finshed"<<endl;
  return 0;
}
```

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		char* const myargv[] = {"out", "-a", "-b", "-c", "-d", NULL};
		execv("./out", myargv);

		////真正执行的是脚本的解释器->bash
		//char* const myargv[] = { "bash", "test.sh", NULL };
		//execv("/usr/bin/bash", myargv);

		/*char* const myargv[] = { "python", "test.py", NULL };
		execv("/usr/bin/python", myargv);*/

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
code.cpp  code.cxx  makefile  test.py  test.sh
[wind@starry-sky Debug]$ make out
[wind@starry-sky Debug]$ make
../../../ProcessReplacement.cpp: In function ‘int main()’:
../../../ProcessReplacement.cpp:22:62: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
   char* const myargv[] = {"out", "-a", "-b", "-c", "-d", NULL};
                                                              ^
../../../ProcessReplacement.cpp:22:62: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
../../../ProcessReplacement.cpp:22:62: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
../../../ProcessReplacement.cpp:22:62: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
../../../ProcessReplacement.cpp:22:62: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
[wind@starry-sky Debug]$ ls
code.cpp  code.cxx  makefile  out  ProcessReplacement  test.py  test.sh
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->29983
start execution

argv:
0->out
1->-a
2->-b
3->-c
4->-d

env:
0->XDG_SESSION_ID=10894
1->HOSTNAME=starry-sky
2->TERM=xterm
3->SHELL=/bin/bash
4->HISTSIZE=1000
5->SSH_CLIENT=112.26.31.132 5424 22
6->OLDPWD=/home/wind/projects/ProcessReplacement
7->SSH_TTY=/dev/pts/0
8->USER=wind
9->LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
10->LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
11->MAIL=/var/spool/mail/wind
12->PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
13->PWD=/home/wind/projects/ProcessReplacement/bin/x64/Debug
14->LANG=en_US.UTF-8
15->HISTCONTROL=ignoredups
16->SHLVL=1
17->HOME=/home/wind
18->LOGNAME=wind
19->SSH_CONNECTION=112.26.31.132 5424 172.31.235.81 22
20->LESSOPEN=||/usr/bin/lesspipe.sh %s
21->XDG_RUNTIME_DIR=/run/user/1002
22->_=./ProcessReplacement

execution finshed
reclaim process resources:29984
replacement after: pid->29983
[wind@starry-sky Debug]$ 
```

`execv`没有`env`参数，它的环境变量是隐式传递的。那么如何传递呢？在进程替换过程中，`execv`并不会完全替换原有进程的所有数据，而是保留部分内容，比如环境变量。因此，被替换的进程会继承原进程的环境变量，从而可以打印出来。

而原先的环境变量又是从哪里来的呢？是从父进程那里继承下来的，父进程则是从`bash`那里继承的，下面我们在`bash`中导入一个新的环境变量，然后重新执行`ProcessReplacement`，看看效果。

```shell
[wind@starry-sky Debug]$ export HELLO=12345
[wind@starry-sky Debug]$ ech0 $HELLO
-bash: ech0: command not found
[wind@starry-sky Debug]$ echo $HELLO
12345
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->30771
start execution

argv:
0->out
1->-a
2->-b
3->-c
4->-d

env:
0->XDG_SESSION_ID=10894
1->HOSTNAME=starry-sky
2->TERM=xterm
3->SHELL=/bin/bash
4->HISTSIZE=1000
5->HELLO=12345
6->SSH_CLIENT=112.26.31.132 5424 22
7->OLDPWD=/home/wind/projects/ProcessReplacement
8->SSH_TTY=/dev/pts/0
9->USER=wind
10->LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
11->LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
12->MAIL=/var/spool/mail/wind
13->PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
14->PWD=/home/wind/projects/ProcessReplacement/bin/x64/Debug
15->LANG=en_US.UTF-8
16->HISTCONTROL=ignoredups
17->SHLVL=1
18->HOME=/home/wind
19->LOGNAME=wind
20->SSH_CONNECTION=112.26.31.132 5424 172.31.235.81 22
21->LESSOPEN=||/usr/bin/lesspipe.sh %s
22->XDG_RUNTIME_DIR=/run/user/1002
23->_=./ProcessReplacement

execution finshed
reclaim process resources:30772
replacement after: pid->30771
[wind@starry-sky Debug]$
```

我们也可以使用`putenv`在父进程里面导入一个环境变量

![image-20241105145104442](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411051451735.png)

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	putenv("PARENT_ENV=666666");
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		char* const myargv[] = {"out", "-a", "-b", "-c", "-d", NULL};
		execv("./out", myargv);

		////真正执行的是脚本的解释器->bash
		//char* const myargv[] = { "bash", "test.sh", NULL };
		//execv("/usr/bin/bash", myargv);

		/*char* const myargv[] = { "python", "test.py", NULL };
		execv("/usr/bin/python", myargv);*/

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->31907
start execution

argv:
0->out
1->-a
2->-b
3->-c
4->-d

env:
0->XDG_SESSION_ID=10894
1->HOSTNAME=starry-sky
2->TERM=xterm
3->SHELL=/bin/bash
4->HISTSIZE=1000
5->HELLO=12345
6->SSH_CLIENT=112.26.31.132 5424 22
7->OLDPWD=/home/wind/projects/ProcessReplacement
8->SSH_TTY=/dev/pts/0
9->USER=wind
10->LD_LIBRARY_PATH=:/home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64
11->LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:
12->MAIL=/var/spool/mail/wind
13->PATH=/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/wind/.local/bin:/home/wind/bin
14->PWD=/home/wind/projects/ProcessReplacement/bin/x64/Debug
15->LANG=en_US.UTF-8
16->HISTCONTROL=ignoredups
17->SHLVL=1
18->HOME=/home/wind
19->LOGNAME=wind
20->SSH_CONNECTION=112.26.31.132 5424 172.31.235.81 22
21->LESSOPEN=||/usr/bin/lesspipe.sh %s
22->XDG_RUNTIME_DIR=/run/user/1002
23->_=./ProcessReplacement
24->PARENT_ENV=666666

execution finshed
reclaim process resources:31908
replacement after: pid->31907
[wind@starry-sky Debug]$ 
```

环境变量就是在一步一步的继承中变得越来越多的。

如果要传全新的环境变量，那就要用到带`e`的`exec`接口了。

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	extern char** environ;
	//putenv("PARENT_ENV=666666");
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		//// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		//char* const myargv[] = {"out", "-a", "-b", "-c", "-d", NULL};
		//execv("./out", myargv);

		////真正执行的是脚本的解释器->bash
		//char* const myargv[] = { "bash", "test.sh", NULL };
		//execv("/usr/bin/bash", myargv);

		/*char* const myargv[] = { "python", "test.py", NULL };
		execv("/usr/bin/python", myargv);*/

		//使用environ传递全套环境变量
		execle("./out", "out", "-a", "-b", "-c", "-d", NULL, environ);

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

也可以在父进程自定义一套环境变量

```cpp
#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	//extern char** environ;
	//putenv("PARENT_ENV=666666");
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		//// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		//char* const myargv[] = {"out", "-a", "-b", "-c", "-d", NULL};
		//execv("./out", myargv);

		////真正执行的是脚本的解释器->bash
		//char* const myargv[] = { "bash", "test.sh", NULL };
		//execv("/usr/bin/bash", myargv);

		/*char* const myargv[] = { "python", "test.py", NULL };
		execv("/usr/bin/python", myargv);*/

		//使用environ传递全套环境变量

		char* const env[] = {"MYVAL=11111", "MYPATH=/usr/bin/xxx", NULL};
		execle("./out", "out", "-a", "-b", "-c", "-d", NULL, env);

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ./ProcessReplacement
replacement before: pid->2097
start execution

argv:
0->out
1->-a
2->-b
3->-c
4->-d

env:
0->MYVAL=11111
1->MYPATH=/usr/bin/xxx

execution finshed
reclaim process resources:2098
replacement after: pid->2097
[wind@starry-sky Debug]$
```

也就是说显式传递环境变量是一种覆写，而非追加。

上述六个接口都位于3号手册，也就是说，它们都是C语言的接口，很明显，语言的背后隐藏着系统，这六个接口实际上都是对系统接口`execve`的不同封装罢了。

![image-20241107181226146](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411071812287.png)

## myshell

下面我们就要写一个最基本的shell了，由于我们马上就要学习文件了，用C语言讲文件更好，所以我们的shell实现，采用的主要方法是C++的容器，C的接口。讲解在代码之后。建议先略过代码看说明。

```cpp
#include<vector>
#include<string>
#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


#define BEGIN "["
#define END "]"

extern char** environ;

namespace wind
{
	class shell
	{
		typedef std::string working_directory;
		typedef std::string work_pwd;
		typedef std::vector<std::string> command;
		typedef std::vector<std::string> local_variable_pool;
		typedef std::vector<std::string> environment_variable_pool;
		typedef shell self;

		std::string get_relative_path()
		{
			size_t pos = _absolute_path.rfind('/');
			if (pos == 0)
			{
				if (_absolute_path.size() > 1)
					return _absolute_path.substr(pos + 1);
				else
					return "/";
			}
			else
			{
				return _absolute_path.substr(pos + 1);
			}
		}

		std::string get_prompt()
		{
			if (std::string(getenv("USER")) == std::string("root"))
				return "#";
			else
				return "$";
		}

		std::vector<std::string> command_extraction()
		{
			std::string tmp;
			std::vector<std::string> ret;
			getline(std::cin, tmp);
			auto it = tmp.begin();
			auto Do = it;
			auto Done = it;
			while (it != tmp.end())
			{
				if (*it == ' ')
				{
					Done = it;
					ret.push_back(std::string(Do, Done));
					++it;
					Do = it;
				}
				else
				{
					++it;
				}
			}
			Done = it;
			ret.push_back(std::string(Do, Done));

			if (ret.size() > 1)
			{
				if (ret[0] == std::string("echo"))
				{
					if (ret[1][0] == '$')
					{
						char* i = (char*)malloc(sizeof(char) * ret[1].size());
						size_t j = 0;

						// test
						//std::cout << ret[1] << std::endl;

						auto it = ret[1].begin();
						++it;
						while (it != ret[1].end())
						{
							i[j++] = *it;
							++it;
						}
						i[j] = '\0';

						if (std::string(i) != "?")
						{
							// test
							//std::cout << i << std::endl;

							char* p = getenv(i);
							std::string s;
							if (p != NULL)
							{
								s = p;
								ret[1] = s;
							}
							else
							{
								ret.pop_back();
							}
						}
						else
						{
							ret[1] = std::to_string(_exit_code);
						}

						free(i);
					}
				}
			}

			// test
			/*for (auto e : ret)
			{
				std::cout << e << " ";
			}
			std::cout << std::endl;*/

			// ls --color -a -l

			_cmd = ret;

			return ret;
		}

	public:
		shell()
			:_absolute_path(getenv("PWD"))
			, _exit_code(0)
		{

		}

		~shell() {}

		void command_line_prompt()
		{
			std::cout << BEGIN << getenv("USER") << "@" << getenv("HOSTNAME") << " ";
			std::cout << get_relative_path() << END << get_prompt() << " ";
		}

		void runing()
		{
			auto cmp = command_extraction();

			if (!cmp.empty())
			{
				if (cmp[0] == std::string("cd"))
				{
					char* d = NULL;
					if (cmp.size() == 1)
					{
						d = getenv("HOME");
					}
					else
					{
						d = (char*)malloc(sizeof(char) * (cmp[1].size() + 1));
						if (d == NULL)
						{
							perror("failed malloc");
						}

						size_t i = 0;
						for (auto e : cmp[1])
						{
							d[i++] = e;
						}
						d[i] = '\0';

						// test
						//std::cout << d << std::endl;

						int ret = chdir(d);

						if (ret == 0)
						{
							_absolute_path = d;
						}
						else
						{
							perror("failed chdir");
						}
						free(d);
						_exit_code = 0;
					}
				}
				else if (cmp[0] == std::string("pwd"))
				{
					// man getcwd
					std::cout << _absolute_path << std::endl;
					_exit_code = 0;
				}
				else if (cmp[0] == std::string("export"))
				{
					if (cmp.size() != 1)
					{
						_envp.push_back(cmp[1]);
						putenv((char*)(_envp.back().c_str()));
					}
					_exit_code = 0;
				}
				else
				{
					pid_t id = fork();

					if (id == 0)
					{
						char** p = (char**)malloc(sizeof(char*) * (cmp.size() + 1));
						size_t i = 0;
						for (auto e1 : cmp)
						{
							char* q = (char*)malloc(sizeof(char) * (e1.size() + 1));
							p[i++] = q;
							size_t j = 0;
							for (auto e2 : e1)
							{
								q[j++] = e2;
							}
							q[j] = '\0';
						}
						p[i] = NULL;

						execvpe(*p, p, environ);
						perror("failed exec");
						exit(1);
					}
					else if (id > 0)
					{
						int status = 0;
						pid_t ret = waitpid(id, &status, 0);

						if (ret > 0)
						{
							if (WIFEXITED(status))
							{
								_exit_code = WEXITSTATUS(status);

								// test
								//std::cout << _exit_code << std::endl;
							}
							else
							{
								_exit_code = status & 0x7f;

								// test
								//std::cout << _exit_code << std::endl;
							}
						}
						else
						{
							perror("failed waitpid");
						}

					}
					else
					{
						perror("failled fork");
					}
				}
			}
		}

	private:
		command _cmd;
		int _exit_code;
		working_directory _absolute_path;
		local_variable_pool _locp;
		environment_variable_pool _envp;
	};
};


int main()
{
	wind::shell bash;
	while (1)
	{
		bash.command_line_prompt();
		bash.runing();
	}
	return 0;
}
```

首先，我们需要打印提示符 `[wind@starry-sky Debug]$`，这通过 `command_line_prompt()` 函数实现。为了方便更改样式，字符 `[` 和 `]` 被定义为宏。

`wind` 表示当前用户的用户名。由于使用的是云服务器，仅能使用命令行，因此我们编写的 shell 将作为系统 shell 的子进程运行。这意味着它会继承系统 shell 的环境变量，因此我们可以通过 `getenv` 直接获取用户信息。`starry-sky` 表示主机名，而 `Debug` 是当前工作路径的简写。这些信息同样可通过 `getenv` 获取。由于环境变量中的路径是绝对路径，为了简化显示，我们使用 `get_relative_path()` 实现路径简写。

在 `shell` 类的构造函数中，工作路径的字面量被存储在成员变量 `_absolute_path` 中。这样，就有了下面的代码

```cpp
size_t pos = _absolute_path.rfind('/');
if (pos == 0)
{
	if (_absolute_path.size() > 1)
		return _absolute_path.substr(pos + 1);
	else
		return "/";
}
else
{
	return _absolute_path.substr(pos + 1);
}
```

`std::string::rfind` 返回匹配字符的最后位置。如果 `pos` 不为 0，我们可以使用 `substr` 提取并返回路径的简写部分。当 `pos` 为 0 时，需特殊处理：若当前路径为根目录 `/`，直接返回；若路径为 `/home`这种形式时，`pos` 也是 0，但应返回 `home`，因此也需要通过 `substr` 返回。

最后，通过 `get_prompt()` 确定显示 `#` 还是 `$`。逻辑很简单，使用 `getenv` 获取当前用户信息并根据结果返回对应字符。

-------------

在学习过程中，我们了解到，shell 会解析用户输入的字符串来获取命令行参数。因此，我们需要 `command_extraction()` 函数来提取用户输入的内容。由于 C 和 C++ 默认将空格作为分隔符，我们使用 `getline` 获取用户输入的原始内容，然后进行解析。解析过程由两个迭代器维护，它们标记字符区间，将提取出的内容推入 `vector<string>` 中。由于迭代器是左闭右开的，当遇到空格时，我们刷新迭代器并将区间插入数组，然后让遍历迭代器前进，并更新起始迭代器。注意，跳出循环后，还需要额外进行一次 `push` 操作。

至于 `if` 语句部分，暂且不谈，这是用于处理 `$` 的转义字符。因为纯命令行环境不便于使用 gdb 调试，我们增加了调试代码来打印数据，确认无误后再注释掉。

至于成员变量 `_cmd`，在我们的 shell 实现中其实用处不大。这个 shell 是为了更好地实践和理解之前学习的知识，因此只实现了基本功能，不包括历史命令打印。最初添加 `_cmd` 是为了避免函数间的深拷贝传参，但后来我们放弃了这个想法。

--------------------

接下来是命令的执行，由 `runing()` 函数负责实现。我们知道，shell 命令分为两类：普通命令和内建命令。普通命令通过创建子进程和进程替换来实现，而内建命令由 shell 自己执行。由于我们刚才已经讨论过进程替换，这里先讲普通命令的实现。

虽然 `command_extraction()` 已经解析了命令行参数，但由于系统接口是 C 风格的，我们需要将 `cmp` 中的内容转换为 C 语言风格。我个人不喜欢使用静态数组，因此使用 `malloc` 进行适配。接下来，子进程负责进程替换，父进程则在子进程运行期间阻塞等待。子进程完成后，父进程接收退出码；若子进程运行中崩溃，父进程会保存错误编号。为了简化处理，崩溃码也存储在 `_exit_code` 中。

顺便提一下，我起初担心子进程在进行进程替换前开辟的内存空间会因替换而丢失指针，导致内存泄漏。不过后来了解到，进程替换时系统会自动释放内存空间，所以这方面可以放心使用。

-----------------------

接下来，我们谈谈内建命令。`cd` 指令是一个典型的内建命令，因为它需要改变 shell 本身的当前路径，所以必须实现为内建命令。

在系统 shell 中使用 `cd` 命令时，当不指定路径时，`cd` 会默认移动到用户的家目录。因此，当 `cd` 没有附带参数时，我们通过 `getenv` 获取指向 home 目录路径的指针。而当用户输入了具体路径时，我们需要将路径进行适当转换并传递给 `chdir` 系统调用。若路径变更成功，还需更新 `_absolute_path` 成员变量。此外，`chdir` 是支持相对路径的，不过像 `~` 和 `-` 这样的路径符号需要手动转换成具体路径，这里暂不做路径转义。

对于 `pwd`，可以简略带过。接下来讨论 `export`。`export` 命令将本地变量导入到环境变量中。在系统 shell 中，`export` 有两种方式：一种是先定义本地变量再导入，比如 `MYTEST=1234` 和 `export MYTEST`；另一种是直接导入，例如 `export MYTEST=1234`。然而在我们实现的 shell 中，仅支持第二种方式，第一种方式暂不实现。

需要注意的是，`putenv` 导入环境变量时使用的是浅拷贝，即它仅存储字符串的指针，而不复制字符串本身。因此，我们需要保留这些字符串，这也是 `_envp` 成员变量存在的原因。由于 `c_str` 方法无法直接用于 `putenv`，所以这里使用了强制类型转换来适配。

实际上，`_locp` 成员变量的设计初衷是用来存储本地变量，以便在 shell 中建立本地变量到环境变量的分层缓冲结构，从而支持 `export` 的第一种用法。不过，由于时间和精力有限，这一功能暂时被搁置。

----------------------

接下来讨论一个特殊的指令：`echo`。虽然 `echo` 本质上是一个普通命令，其特别之处在于它可以结合转义字符 `$` 打印环境变量。`command_extraction` 中的 `if` 语句正是为此而设计的。这段代码的目的在于解析 `echo` 后面的内容，并根据需要替换为对应的环境变量值。为避免在找不到环境变量时出现空指针解引用，我们对 `getenv` 的返回结果进行了检查。如果 `getenv` 未能找到相应的环境变量，该字符串会被pop。

最后提一下，为了区分我们的自定义 shell 和系统 shell，我们没有在命令行参数中添加 `--color` 选项，因此输出内容不会有颜色区分。

![image-20241108211418187](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411082114270.png)

-------------------------

最后，我们可以谈谈代码之外的一些内容。系统 shell 的环境变量是从用户家目录中的配置文件加载的。当用户登录服务器时，shell 会自动启动并加载这些配置文件，随后在用户退出前持续运行。我们可以在 home 目录下的 `.bash_profile`、`.bashrc` 

![image-20241108212139218](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411082121297.png)![image-20241108212312220](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411082123311.png)

以及全局配置文件 `/etc/bashrc` 中查看这些配置信息，不建议修改文件，修改前也要作备份。![image-20241108212444240](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411082124332.png)

# end