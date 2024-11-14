#  file

## preface

在学习C语言时，我们曾简单了解过一些文件接口，但对于文件的语言层面理解仍然不够深入。今天，我们将深入探讨Linux的第二座大山——文件。理解文件的工作原理，对于后续深入理解第三座大山——网络，至关重要。

在正式学习文件之前，我们需要了解以下几个概念。

文件与容器类似，都可以分为两部分——内部的数据本体和外部描述这些数据的属性。

文件有两种状态：打开和未打开。在C语言中，我们通过`fopen`函数来打开文件。深入来看，系统中的一切程序本质上都是进程，因此，研究文件的打开过程，实际上是在探讨文件与进程之间的关系。而对于未打开的文件，它们存储在像磁盘这样的存储介质中，就像放在快递架上的包裹一样。未打开的文件数量庞大，为了能够在需要时迅速找到它们，必须建立一套文件管理机制，类似于每个快递都有一个取件码，取件码标识了快递的位置。因此，研究未打开文件，实质上是研究文件与存储介质之间的关系。

只有在找到文件后，才能将其加载到内存中，进而对其内容进行增删查改。对于已经打开的文件，尽管数量少于未打开的文件，但依然需要管理。谈到管理，那就又要讲到系统上，系统为了对这些已经打开的文件进行管理，就要先描述，再组织。

系统通过结构体中的各个变量对每个文件的种种属性进行描述，然后再对这些结构体以某些数据结构，如链表的方式组织起来。就像进程一样，系统不会直接操作文件本身，而是对文件结构体进行增删查改从而实现文件的管理。

## review

我们先回顾一下C语言中的文件接口

![image-20241110191502661](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411101915767.png)

`fopen`返回的是C语言库中的自定义类型`FILE`的指针，`FILE`被称为文件指针，在windows环境下，也有句柄的叫法。

```cpp
#include<stdio.h>

int main()
{
	FILE* pf = fopen("logbook.txt", "w");
	if (pf == NULL)
	{
		perror("failed fopen");
		return 1;
	}

	fclose(pf);

	return 0;
}
```

随口说一下，`logbook`是日志的意思，以后我们可能要实现日志系统的。

让我们看看`"w"`mode的描述：Truncate file to zero length or create text file for writing.  The stream is positioned at the beginning of the file.

`truncate`是截断的意思，`file to zero length`截断的效果就是把文件长度变为0，也就是清空，或者，当文件没有时，构造或者说创建一个文件用来写。流的（初始）位置在文件开头。

```shell
[wind@starry-sky Debug]$ ls
file  makefile
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ls
file  logbook.txt  makefile
[wind@starry-sky Debug]$ cat logbook.txt
[wind@starry-sky Debug]$
```

这在C语言阶段我们已经有所了解，这里有个细节，其实之前说过，在我们的代码中，并没有指定绝对路径，而只是给出了文件名`logbook.txt`，那`file`在执行变成进程后，怎么知道我这`logbook.txt`到底指的是那个路径下的文件呢？在C阶段，我们一直说，这是什么当前路径，即程序运行的路径，为什么这样说呢？因为在C阶段，我们没学过`chdir`系统接口，不会改进程工作路径，所以程序运行之后，其工作路径一直在程序运行的路径下，所以我们说当前路径，下面我们改一下进程的工作路径，观察相应现象。

```cpp
#include<stdio.h>			//fopen
#include<unistd.h>			//chdir
#include <sys/types.h>		//getpid

int main()
{
	pid_t id = getpid();
	printf("pid-> %d\n", id);
	//sleep(15);

	FILE* pf = fopen("logbook.txt", "w");
	if (pf == NULL)
	{
		perror("failed fopen");
		return 1;
	}

	fclose(pf);

	sleep(50);

	return 0;
}
```

这次我们多睡一会，看看当前工作路径。通过`/proc/`路径

![image-20241110200527983](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411102005071.png)

```cpp
#include<stdio.h>			//fopen
#include<unistd.h>			//chdir
#include <sys/types.h>		//getpid

int main()
{
	pid_t id = getpid();
	printf("pid-> %d\n", id);
	chdir("/home/wind");

	FILE* pf = fopen("logbook.txt", "w");
	if (pf == NULL)
	{
		perror("failed fopen");
		return 1;
	}

	fclose(pf);

	sleep(50);

	return 0;
}
```

![image-20241110201404843](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411102014933.png)

来到家目录下，我们也看到了刚创建的文件了

![image-20241110201546517](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411102015594.png)

接下来我们往文件里写写信息

![image-20241110202327209](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411102023284.png)

The function fwrite() writes nmemb elements of data, each size bytes long, to the stream pointed to by stream, obtaining them  from  the  location given by ptr.

函数 `fwrite()` 将 `nmemb` 个元素 写入 `stream `所指向的流中，每个`each`元素的大小是 size 字节，这些数据元素从 `ptr` 指定的位置获取。

```cpp
#include<stdio.h>			//fopen
#include<unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include<string.h>			//strlen

int main()
{
	/*pid_t id = getpid();
	printf("pid-> %d\n", id);*/

	FILE* pf = fopen("logbook.txt", "w");
	if (pf == NULL)
	{
		perror("failed fopen");
		return 1;
	}

	const char* message = "hello C I/O Interfaces";
	fwrite(message, strlen(message), sizeof(char), pf);

	fclose(pf);

	return 0;
}
```

先不要纠结于细节，先看现象。message 是消息的意思。

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
makefile
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ls
file  logbook.txt  makefile
[wind@starry-sky Debug]$ cat logbook.txt
hello C I/O Interfaces[wind@starry-sky Debug]$
```

把 message 改一下，改成abcd，再运行一下。

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
logbook.txt  makefile
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ..file
-bash: ..file: command not found
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat ./logbook.txt
abcd[wind@starry-sky Debug]$
```

这也和文档的表述相符，文件被清空了，这次，我们直接注释 fwirte()，看看现象

```shell
abcd[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
logbook.txt  makefile
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat logbook.txt
[wind@starry-sky Debug]$ echo "abcd" > logbook.txt
[wind@starry-sky Debug]$ cat *.txt
abcd
[wind@starry-sky Debug]$ 实际上，只用"w"模式fopen，而不写信息，就相当于^C
-bash: $'实际上，只用w模式fopen，而不写信息，就相当于\003': command not found
[wind@starry-sky Debug]$ > logbook.txt
[wind@starry-sky Debug]$ cat logbook.txt
[wind@starry-sky Debug]$
```

接下来我们说一个细节：fwrite()中的第二个参数，它需要加一，也就是把那个`'\0'`也写进去吗？那我们现在加上去看看效果

```cpp
#include<stdio.h>			//fopen
#include<unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include<string.h>			//strlen

int main()
{
	/*pid_t id = getpid();
	printf("pid-> %d\n", id);*/

	FILE* pf = fopen("logbook.txt", "w");
	if (pf == NULL)
	{
		perror("failed fopen");
		return 1;
	}

	//const char* message = "abcd";
	const char* message = "hello C I/O Interfaces";
	fwrite(message, strlen(message) + 1, sizeof(char), pf);

	fclose(pf);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
logbook.txt  makefile
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ cat logbook.txt
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat logbook.txt
hello C I/O Interfaces[wind@starry-sky Debug]$ vim logbook.txt
```

![image-20241110204951103](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411102049181.png)

由于`\0`是不可见的，所以对于文本编辑器来说，它是乱码。用cat 可能体现不出来。

那到底要不要加这个`1`呢？我们从两个角度来考虑。

一是语言层面，由于 cat 这些指令都是用C/C++写的，而C规定字符串以`\0`为结尾，所以若`\0`后还有信息，就不会被打印出来。

```shell
[wind@starry-sky Debug]$ echo "abcd" >> logbook.txt
[wind@starry-sky Debug]$ cat logbook.txt
hello C I/O Interfacesabcd
[wind@starry-sky Debug]$ "abcd"就没有被打印出来^C
[wind@starry-sky Debug]$ vim logbook.txt
```

![image-20241110205937749](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411102059843.png)

不过这不是什么大问题。

第二个方面才是大问题，字符串以`\0`为结尾只是C/C++标准，不是系统标准，也不是别的语言的标准，C程序毕竟是在系统里运行的，其输出的信息要遵循系统的标准，系统标准就是不加任何修饰的纯字符串，没有`\0`，这个系统标准对其它语言写的程序也是如此。因此，为了与其他程序和语言保持一致，输出的数据应避免附带任何结尾字符，特别是 `\0`，以免因不同语言的处理差异导致文件内容的混乱。其他语言的程序在读取该文件时，并不会根据 `\0` 来判断字符串的结尾，而是通过其他机制来处理文件中的字符数据。

所以不要加1.

---------------

`"a"`mode

Open for appending (writing at end of file).  The file is created if it does not exist.  The stream is positioned at the end of the file.

只要认识`appending`【追加】就行了。换成`a`模式重新运行程序。

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ > logbook.txt
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat logbook.txt
hello C I/O Interfaceshello C I/O Interfaceshello C I/O Interfaces[wind@starry-sky Debug]$ 看样子应该加个"\n"^C
[wind@starry-sky Debug]$
```

其它语言接口就不说了，毕竟我们以后要说系统接口的

----------------

接下来我们稍微提及一下，我们知道C/C++程序在运行时会默认打开三个流，或者说文件？叫文件流吧。

![image-20241110212426387](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411102124488.png)

它们是标准输入流 (`stdin`)、标准输出流(`stdout`) 和 标准错误流 (`stderr`)。在 Linux 中，“一切皆文件”是一个核心思想，这三个流实际上就是键盘和显示器的文件抽象。

- **`stdin`** 对应键盘，程序从这里读取输入。
- **`stdout`** 对应显示器，用于输出正常的信息。
- **`stderr`** 同样输出到显示器，但专门用于打印错误或诊断信息，以便与正常输出区分开来。

`stdout` 和 `stderr` 的区分，可以帮助我们更好地处理程序的输出，尤其在需要单独重定向错误信息时。比如，可以将正常输出和错误输出分别重定向到不同的文件中，方便调试和日志记录。

![image-20241110213429244](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411102134339.png)

写个代码看看？

```cpp
#include<stdio.h>			//fopen
#include<unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include<string.h>			//strlen

int main()
{
	/*pid_t id = getpid();
	printf("pid-> %d\n", id);*/

	FILE* pf = fopen("logbook.txt", "a");
	if (pf == NULL)
	{
		perror("failed fopen");
		return 1;
	}

	//const char* message = "abcd";
	const char* message = "hello C I/O Interfaces";
	//fwrite(message, strlen(message) + 1, sizeof(char), pf);
	fprintf(pf, "%s %d\n", message, 1024);

	fclose(pf);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ > logbook.txt
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat logbook.txt
hello C I/O Interfaces 1024
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat logbook.txt
hello C I/O Interfaces 1024
hello C I/O Interfaces 1024
hello C I/O Interfaces 1024
hello C I/O Interfaces 1024
[wind@starry-sky Debug]$
```

把`pf`改成stdout or stderr

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./file
hello C I/O Interfaces 1024
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./file
hello C I/O Interfaces 1024
[wind@starry-sky Debug]$ 
```

看起来一样，不过别担心，再过几节课就体现出不同了。

其实不仅是C/C++程序，实际上，所有程序，不管用什么写的，都会默认打开这三个文件流，区别只是名字和操作方法不同罢了，接下来我们将会说这是为什么，下面我们从系统层面认识文件。

## system

首先我们知道文件是存储在各类外设中的，典型的就如磁盘。因此，对于装载了操作系统的计算机，用户对文件的访问必须经过系统和驱动等中间层。换句话说，在这种情况下，所有语言层面的文件接口都会封装系统调用。接下来，我们来了解一下这些接口的实现方式。

![image-20241111083311077](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411110833407.png)

我们主要说`int open(const char *pathname, int flags, mode_t mode);`这个说好了，`int open(const char *pathname, int flags);`就自然会用了，而对于`int creat(const char *pathname, mode_t mode);`，它其实只能创建文件，功能太少了，就不说了。

对于`int open(const char *pathname, int flags, mode_t mode);`来说，其第一个参数`pathname`，用来描述文件路径，第二个参数`flags`则是用来描述打开文件的方式，第三个参数`mode`是用来描述新创建文件的权限。它的返回值是个 int ，被称为文件描述符，稍后我们就会知道文件描述符其实是个很简单的东西，失败时，open 返回 -1。

第一个参数`pathname`既支持绝对路径，也支持相对路径，就不说了。第二个参数`flags`是一个整型，我们将一些宏作为它的可选参数，这些参数主要有`O_RDONLY, O_WRONLY, O_RDWR, O_CREAT,  O_APPEND`。

---------------

为了理解这些参数，我们需要说说Linux中比特位级别的传参方式。

```cpp
#define ONE (1<<0)   //1
#define TWO (1<<0)	 //2
#define THREE (1<<0) //4
#define FOUR (1<<0)  //8


void show(int flags)
{
	if (flags & ONE) printf("function 1\n");
	if (flags & TWO) printf("function 2\n");
	if (flags & THREE) printf("function 3\n");
	if (flags & FOUR) printf("function 4\n");
}


int main()
{
	printf("\nONE->\n");
	show(ONE);

	printf("\nTWO->\n");
	show(TWO);

	printf("\nONE|TWO->\n");
	show(ONE|TWO);

	printf("\nONE|TWO|THREE->\n");
	show(ONE|TWO|THREE);

	printf("\nONE|THREE->\n");
	show(ONE|THREE);

	printf("\nTHREE|FOUR->\n");
	show(THREE|FOUR);

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ rm logbook.txt
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file

ONE->
function 1
function 2
function 3
function 4

TWO->
function 1
function 2
function 3
function 4

ONE|TWO->
function 1
function 2
function 3
function 4

ONE|TWO|THREE->
function 1
function 2
function 3
function 4

ONE|THREE->
function 1
function 2
function 3
function 4

THREE|FOUR->
function 1
function 2
function 3
function 4
[wind@starry-sky Debug]$
```

一个整型共有32个位，其中每个位可以作为一个标志位，标志位为1时表示该功能被选中。通过按位移位操作，可以为每个功能分配一个唯一的标志位。使用按位或（`|`）操作可以同时选择多个功能，而这些选项之间是互不干扰的。这样，就可以通过组合多个标志位，执行多个功能性代码块。

当然，`show` 中的代码块并不是完全互不干扰的。例如，对于 `open` 操作，如果文件不存在，通常需要先创建文件再进行打开，因此相关代码块的顺序需要遵循逻辑顺序。也就是说，这些 `if` 语句的顺序不能随意调整，必须按照一定的逻辑流程来执行。上述提到的“互不干扰”是指在传参层面，标志位之间不会冲突，而实际执行时，仍然需要注意操作的先后顺序。

----------------

`O_WRONLY`

- 该宏表示以**只写**模式打开文件，文件必须存在。如果文件不存在，`open` 会返回错误。
- 只能写入文件内容，不能读取。

其中`O`表示`operation`【操作】，`WR`表示`write`，`ONLY`表示`only`。

```cpp
#include<stdio.h>			//fopen
#include<unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include<string.h>			//strlen


int main()
{
	int fd = open("logbook.txt", O_WRONLY);
	if (fd < 0)
	{
		perror("failed open");
		return fd;
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
makefile
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file
failed open: No such file or directory
[wind@starry-sky Debug]$
```

`O_CREAT`

- 该宏表示如果文件不存在，则**创建**文件。
- 如果文件已经存在，不会做任何修改，`open` 会成功打开文件。
- 创建文件时，通常还需要指定文件的权限（如 `mode_t` 类型的 `mode` 参数），用于设置新文件的访问权限。

其中`O`表示`operation`【操作】，`CREAT`表示`create`。

先看看不指定权限的后果。

```cpp
#include<stdio.h>			//fopen
#include<unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include<string.h>			//strlen


int main()
{
	int fd = open("logbook.txt", O_WRONLY|O_CREAT);
	if (fd < 0)
	{
		perror("failed open");
		return fd;
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ls
file  logbook.txt  makefile
[wind@starry-sky Debug]$ ll
total 20
-rwxrwxr-x 1 wind wind 15872 Nov 11 09:53 file
--wSrwx--T 1 wind wind     0 Nov 11 09:53 logbook.txt
-rw-rw-r-- 1 wind wind    85 Nov 10 19:12 makefile
[wind@starry-sky Debug]$ 
```

倒是确实创建过来了，但看着不太对劲，文件权限方面。有色彩更能看出来。

![image-20241111095507719](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411110955965.png)

实际上，如果不给权限就创建文件的话，系统就会随机给权限，此时就必须使用有第三个参数的 open 了。

```cpp
#include<stdio.h>			//fopen
#include<unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include<string.h>			//strlen


int main()
{
	int fd = open("logbook.txt", O_WRONLY|O_CREAT, 0666);
	if (fd < 0)
	{
		perror("failed open");
		return fd;
	}
	return 0;
}
```

之前我们在学习文件权限的时候，提到过`0666`后面三个6是什么，但没有特别提及最前面的0是什么，0表示的其实就是文件类型，也就是`-rw-rw-r--`最前面的`-`，表示普通文件。

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ rm -rf logbook.txt
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ll
total 20
-rwxrwxr-x 1 wind wind 15872 Nov 11 10:06 file
-rw-rw-r-- 1 wind wind     0 Nov 11 10:06 logbook.txt
-rw-rw-r-- 1 wind wind    85 Nov 10 19:12 makefile
[wind@starry-sky Debug]$
```

这是664，不是666，为什么呢？我们在文件权限那里也说过，因为有权限掩码

```shell
[wind@starry-sky Debug]$ umask
0002
[wind@starry-sky Debug]$
```

权限掩码把 others 的写权限筛除了。那如果就是要 666 怎么办？不用担心，可以调用系统接口`umask`，让进程使用自己的权限掩码，而不使用系统默认的系统掩码。

![image-20241111101309243](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411111013455.png)

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	umask(0);
	int fd = open("logbook.txt", O_WRONLY|O_CREAT, 0666);
	if (fd < 0)
	{
		perror("failed open");
		return fd;
	}
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ rm logbook.txt
[wind@starry-sky Debug]$ ls
file  makefile
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ll
total 20
-rwxrwxr-x 1 wind wind 15920 Nov 11 10:15 file
-rw-rw-rw- 1 wind wind     0 Nov 11 10:16 logbook.txt
-rw-rw-r-- 1 wind wind    85 Nov 10 19:12 makefile
[wind@starry-sky Debug]$
```

有open 就有 close ,close的参数就是open返回的文件描述符

![image-20241111102131723](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411111021952.png)

文件打开了就要被修改

![image-20241111102340856](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411111023014.png)

`ssize_t write(int fd, const void *buf, size_t count);`  第一个参数`fd`是文件描述符，第二个参数`buf`就是写入数据的起始地址，第三个参数`count`描述写入数据的字节数。

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	int fd = open("logbook.txt", O_WRONLY|O_CREAT, 0666);
	if (fd < 0)
	{
		perror("failed open");
		return fd;
	}

	const char* memssage = "hello file system interfaces";
	write(fd, memssage, strlen(memssage));

	close(fd);
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ rm logbook.txt
[wind@starry-sky Debug]$ ls
file  makefile
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ls
file  logbook.txt  makefile
[wind@starry-sky Debug]$ cat logbook.txt
hello file system interfaces[wind@starry-sky Debug]$
```

修改一下字符，再试一试

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	int fd = open("logbook.txt", O_WRONLY|O_CREAT, 0666);
	if (fd < 0)
	{
		perror("failed open");
		return fd;
	}

	const char* memssage = "abcde";
	write(fd, memssage, strlen(memssage));

	close(fd);
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat logbook.txt
abcde file system interfaces[wind@starry-sky Debug]$
```

这回就不是刷新后从头写了，而是直接从头写。那怎么截断呢？可以再带上`O_TRUNC`宏选项

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	int fd = open("logbook.txt", O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (fd < 0)
	{
		perror("failed open");
		return fd;
	}

	const char* memssage = "abcde\n";
	write(fd, memssage, strlen(memssage));

	close(fd);
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
file  logbook.txt  makefile
[wind@starry-sky Debug]$ cat logbook.txt
abcde file system interfaces[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat logbook.txt
abcde
[wind@starry-sky Debug]$
```

再来试试追加

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	int fd = open("logbook.txt", O_WRONLY|O_CREAT|O_APPEND, 0666);
	if (fd < 0)
	{
		perror("failed open");
		return fd;
	}

	const char* memssage = "abcde\n";
	write(fd, memssage, strlen(memssage));

	close(fd);
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ls
file  logbook.txt  makefile
[wind@starry-sky Debug]$ cat logbook.txt
abcde
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ ./file
[wind@starry-sky Debug]$ cat logbook.txt
abcde
abcde
abcde
abcde
[wind@starry-sky Debug]$
```

## essence

接下来我们已经可以说说文件访问的本质了。

当一个进程发出open 请求时，系统就会依据接口的参数从磁盘上找到这个文件，把它加载到内存中。同时为了描述这个文件，系统会为这个文件创建对应的结构体变量`struct file`。

`struct file`中存储着文件的相关信息，它是对文件本身的抽象描述，其中包括，文件的磁盘位置，文件的权限信息，当前的读写位置（光标的位置），哪个进程打开的，两个`struct file*`指针，用来索引上一个和下一个`struct file*`；除此之外，系统还会为该文件创建一个内核层面的缓冲区，定时地把缓冲区里的信息写回磁盘。

![image-20241111112939418](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411111129550.png)

在进程控制块中，有一个`struct files_struct *`类型的指针，名为`files`，`files`指向该进程专属的一个结构体，该结构体中又分为若干部分，其中一部分是个指针数组。

![image-20241111115744271](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411111157430.png)

当进程打开一个文件后，系统就会把文件对应的`struct file`结构体指针导入到`struct files_struct`中数组的空位置中，从而让进程和文件之间建立联系。实际上，open 返回的文件描述符就是这个数组的下标。

![image-20241111131958772](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411111319855.png)

下面我们用程序打印一下文件描述符。

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	int fd1 = open("logbook.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
	int fd2 = open("logbook.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
	int fd3 = open("logbook.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
	int fd4 = open("logbook.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
	printf("fd1-> %d\n", fd1);
	printf("fd2-> %d\n", fd2);
	printf("fd3-> %d\n", fd3);
	printf("fd4-> %d\n", fd4);

	/*if (fd < 0)
	{
		perror("failed open");
		return fd;
	}

	const char* memssage = "abcde\n";
	write(fd, memssage, strlen(memssage));

	close(fd);*/
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file
fd1-> 3
fd2-> 4
fd3-> 5
fd4-> 6
[wind@starry-sky Debug]$
```

我们看到下标是从3开始的，那`0, 1, 2`呢？其实`0, 1, 2`就是进程默认打开的三个文件流`stdin, stdout, stderr`，我们可以测试一下。

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	const char* message = "hello linux\n";
	write(1, message, strlen(message)); // stdout
	write(2, message, strlen(message)); // stderr
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file
hello linux
hello linux
[wind@starry-sky Debug]$
```

来些读操作

![image-20241111140612705](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411111406913.png)

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	char buffer[256] = {0};
	ssize_t sz = read(0, buffer, sizeof(buffer) - 1);
	if (sz < 0)
	{
		perror("failed read");
		return 1;
	}
	buffer[sz] = '\0';

	write(1, buffer, strlen(buffer));

	return 0;
}
```

从系统层面的文件流转换到C环境下，需要加上`\0`，所以这次要把`sizeof(buffer)`减一，注意不能用`strlen`，`strlen`是用来计算字符串的长度的，buffer 现在是空的，第一个元素就是`\0`，所以不能用`strlen`，read 会返回读取到的元素个数，失败时返回-1，因为文件流中的字符串没有`\0`所以读到C环境中要在末尾加上`\0`，不过因为这里对字符数组做初始化了，所以这里去掉其实没什么大问题，不过为了安全起见，最好还是不管有没有初始化，都在末尾添上`\0`。

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411111404241.mp4"></video>

----------

实际上C语言中的`FILE`，就是对`fd`的封装，来使用一下？

```cpp
#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask


int main()
{
	const char* message = "hello linux\n";
	write(stdout->_fileno, message, strlen(message)); 
	write(stderr->_fileno, message, strlen(message)); 
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ./file
hello linux
hello linux
[wind@starry-sky Debug]$
```

---------------

在`struct file`中除了之前所说的那些信息之外，还有一个名叫`count`的字段，这个字段或者说成员变量是一种引用计数。同一个文件可能被多个进程打开，文件打开的操作除了之前说的导入指针之外，其实还会把count 字段加一，而当某个进程关闭该文件时，一方面，会把`struct files_struct`对应的位置置空，另一方面，会把count字段减一，当count为0时，意味着这个文件没有被任何进程使用，此时系统就会把该文件从系统层面上关闭。

--------------------

系统本身在被“引导加载程序”加载并运行后，会自动开启标准输入、标准输出和标准错误这三个文件流，以便进行基本的人机交互。当系统中的其他进程启动时，系统会将这三个文件流的指针加载到进程的 `struct files_struct` 结构中，确保每个进程在运行时都有这些默认的文件流。因此，不管是用 C/C++、Java、Python，甚至是汇编语言或纯二进制代码编写的程序，一旦运行，都会变成一个进程，并默认打开这三个流。这种行为使得各类程序在操作输入、输出或错误处理时，都能使用这标准化的接口，从而简化了编程和交互。

# end