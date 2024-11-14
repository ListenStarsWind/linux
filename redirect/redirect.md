# redirect

## transition

在上一节课中，我们学习了文件描述符的本质。本节课将基于此，深入探讨重定向的本质。在正式开始之前，我们先回顾一下文件描述符的相关内容。

当进程启动时，会从操作系统继承标准输入流、标准输出流和标准错误流，这三者被自动填入进程的文件描述符表，因此每个进程默认拥有这三个文件流。

文件的打开本质上是将外部存储设备中的文件加载到内存中，并为其创建一个`struct file`结构体。该结构体用于存储文件的各种属性或访问这些属性的方法。

进程控制块（PCB）中包含一个指向`struct files_struct`结构体的指针，该结构体中维护着一个数组，称为文件描述符表。该数组存储了该进程已打开文件的`struct file*`指针。调用`open`时，系统将打开的文件对应的`struct file*`存入表中空闲的位置，并返回该位置的索引。`close`操作则会将`struct file`的`count`字段减一。

先写一份代码

```cpp
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>

typedef const char* message;
typedef int file_descriptor;
typedef file_descriptor fd;

#define FILENAME "logbook.txt"
#define GREETING "hello linux\n"
#define PERMISSION 0666

void function1()
{
	fd handle = open(FILENAME, O_CREAT| O_WRONLY| O_TRUNC, PERMISSION);
	if (handle < 0)
	{
		perror("failed open");
		return ;
	}
	fprintf(stdout, "fd->%d\n", handle);
	
	int count = 5;
	message sentence = GREETING;
	while (count--)
	{
		write(handle, sentence, strlen(sentence));
	}

	close(handle);
}

class function
{
	typedef void (*operation)();
public:
	void operator()() { _action(); }
private:
	operation _action = function1;
};

int main()
{
	function process;
	process();
	return 0;
}
```

后面如果没看懂没关系，你只要知道这里是运行`function1`就行了。

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ls
makefile
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ls
makefile  redirect
[wind@starry-sky Debug]$ ./redirect
fd->3
[wind@starry-sky Debug]$ ls
logbook.txt  makefile  redirect
[wind@starry-sky Debug]$ cat logbook.txt
hello linux
hello linux
hello linux
hello linux
hello linux
[wind@starry-sky Debug]$
```

和我们的预期没什么区别。

--------------

我们现在只加一行代码：

![image-20241114103351587](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141033940.png)

再运行一下试试？

```shell
[wind@starry-sky Debug]$ make clean ; make 
[wind@starry-sky Debug]$ ./redirect
fd->0
[wind@starry-sky Debug]$
```

我们看到，现在`logbook.txt`的文件描述符是0了

再改一点。

![image-20241114103626065](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141036950.png)

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./redirect
[wind@starry-sky Debug]$
```

没有输出，这也好理解，标准输出流被关上了。

不过经过上面这两次，我们发现，系统对于文件描述符的分配规则似乎是：将最前面的空位置分配给新打开的文件。接下来让我们关闭2看看。

![image-20241114104137054](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141041780.png)

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./redirect
fd->2
[wind@starry-sky Debug]$
```

确实是2，看样子符合我们的推测，实际上，我们甚至可以猜测它的分配机制是通过一个循环，遍历描述符表，找到第一个空位置，将其分配给最新打开的文件。

-------------

现在我们要整活了。

再略微修改一下代码：

![image-20241114105302516](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141053466.png)

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./redirect
hello linux
hello linux
hello linux
hello linux
hello linux
[wind@starry-sky Debug]$
```

这也好理解，1就是标准输出流，或者说显示器文件的描述符吗，输出到屏幕上没什么稀奇的。

现在再改动一下

![image-20241114105629434](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141056324.png)

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./redirect
[wind@starry-sky Debug]$ cat logbook.txt
hello linux
hello linux
hello linux
hello linux
hello linux
[wind@starry-sky Debug]$
```

我们看到，本来应该输出到屏幕上的五个句子，现在输出到`logbook.txt`里了。

这像什么？是不是像`redirect`【重定向】呀？更详细地说，是输出重定向。

## why

我们知道在一个进程刚运行时，它大概是这样的：

![image-20241114112234279](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141122333.png)

实际上我用`stdin  stdout  stderr`的名字不太好，因为这是语言里的东西，我们说的是系统，但为了大家方便理解，就这样画了。我想要表示的是`stderr and stdout`是显示器文件，`stdin`是键盘文件。

并且其实这张图是错的，因为按道理1和2应该指向同一个显示器文件，只不过这个显示器文件就当前场景来说，其`count`字段为2。但如果再把这个考虑进来就不方便我们理解，所以我们分开来看。

在上面的代码中

![image-20241114105629434](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141056324.png)

我们首先`close(1)`，所以1的位置现在就是空了。

![image-20241114113447865](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141134957.png)

接着我们打开了一个新文件，按照文件描述符的分配规则，`logbook.txt`的`struct file*`被加载到1中。

![image-20241114113733053](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141137126.png)

然后我们就看到`write`傻乎乎地往1索引的文件里写信息了。

这就是重定向最基本的原理。

## implementation

现在，我们就要谈谈重定向的`implementation`【实现】了。上面的实现方式是用来教学的，实际不这样写。想重定向还要手动关闭一个文件，然后立刻打开一个文件；而且，另一方面，这也不安全，如果前面还有空的位置，那新打开的文件就不会链接到刚关闭的文件描述符上。

这时候就需要使用一个系统借口了。

![image-20241114114820213](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141148368.png)

`dup` 是 "duplicate" 的缩写，表示“复制”或“重复”的意思。

不要看`dup2`的参数名，是有坑的，等用完这个接口之后，我会说这个坑是怎么来的。现在你只需要记住，`dup2`的第一个参数是新打开文件的描述符，而第二个参数是原来应该输出到的那个文件。比如在上面的例子中，五行`hello linux`原来应该显示在显示器文件上，但现在写到了`logbook.txt`文件里，所以第一个参数是`logbook.txt`的文件描述符，而第二个参数是`stdout`的描述符也就是1。

`dup2`会先把第二个参数索引的文件关闭，然后把第一个参数里的指针复写到第二个参数的位置上，并且对第一个参数索引的文件结构体`count`字段加一。

至于`dup2`之后要不要立刻关闭第一个参数索引的文件，要看具体情况，不用太担心因为没有立刻关闭而浪费描述符表的位置。

![image-20241114124723191](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141247715.png)

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ ll
total 8
-rw-rw-r-- 1 wind wind 60 Nov 14 12:48 logbook.txt
-rw-rw-r-- 1 wind wind 91 Nov 14 10:22 makefile
[wind@starry-sky Debug]$ > logbook.txt
[wind@starry-sky Debug]$ ll
total 4
-rw-rw-r-- 1 wind wind  0 Nov 14 12:49 logbook.txt
-rw-rw-r-- 1 wind wind 91 Nov 14 10:22 makefile
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ls
logbook.txt  makefile  redirect
[wind@starry-sky Debug]$ ./redirect
[wind@starry-sky Debug]$ ll
total 24
-rw-rw-r-- 1 wind wind    60 Nov 14 12:50 logbook.txt
-rw-rw-r-- 1 wind wind    91 Nov 14 10:22 makefile
-rwxrwxr-x 1 wind wind 16184 Nov 14 12:49 redirect
[wind@starry-sky Debug]$ cat logbook.txt
hello linux
hello linux
hello linux
hello linux
hello linux
[wind@starry-sky Debug]$
```

也可以这样写：

![image-20241114125155030](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141251485.png)

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ll
total 24
-rw-rw-r-- 1 wind wind    60 Nov 14 12:50 logbook.txt
-rw-rw-r-- 1 wind wind    91 Nov 14 10:22 makefile
-rwxrwxr-x 1 wind wind 16240 Nov 14 12:52 redirect
[wind@starry-sky Debug]$ ./redirect
[wind@starry-sky Debug]$ ll
total 24
-rw-rw-r-- 1 wind wind    60 Nov 14 12:52 logbook.txt
-rw-rw-r-- 1 wind wind    91 Nov 14 10:22 makefile
-rwxrwxr-x 1 wind wind 16240 Nov 14 12:52 redirect
[wind@starry-sky Debug]$ cat logbook.txt
hello linux
hello linux
hello linux
hello linux
hello linux
[wind@starry-sky Debug]$
```

因为`printf`默认使用`stdout`，而`stdout`封装的文件描述符是`1`。

上面的代码相当于输出重定向`>`，除此之外，还有追加重定向`>>`，就不写了，就是把open的方式换成`O_CREAT | O_WRONLY | O_APPEND`。接下来写写输入重定向`<`

![image-20241114140748374](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141407340.png)

从键盘文件中尝试读取`sizeof(buffer) - 1`大小的数据，read返回读取到的字符个数，在其后面追加终止符`\0`。随后再通过`printf`打印读取到的字符，用来表示读取成功。

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ls
logbook.txt  makefile  redirect
[wind@starry-sky Debug]$ ./redirect
hello linux
hello linux
[wind@starry-sky Debug]$
```

![image-20241114141445018](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141414116.png)

现在就是输入重定向了。

```shell
[wind@starry-sky Debug]$ echo "The quick brown fox jumps over a lazy dog." > logbook.txt
[wind@starry-sky Debug]$ cat logbook.txt
The quick brown fox jumps over a lazy dog.
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./redirect
The quick brown fox jumps over a lazy dog.
[wind@starry-sky Debug]$
```

本来应该从显示屏文件中读取信息，现在从`logbook.txt`中读取了。

--------------

现在来说说`dup2`的参数名字为什么这么奇怪了。

其实在之前的那张图中，可以看到`dup2`之上还有一个`dup`，最开始，只有这一个`dup`，它的逻辑是：输入原文件描述符 `oldfd`，系统会在文件描述符表中查找一个空闲位置，将 `oldfd` 对应的文件结构体指针复制到新位置，并将文件结构体的 `count` 字段加一。所以`dup`的参数名是`oldfd`。

后来有了`dup2`，为了避免混淆，“被复制的文件描述符”依旧使用`oldfd`，而“目标文件描述符”使用`newfd`。

------------------------

`shell`版本的重定向我就不写了。不过可以大概说一下思路，在指令解析阶段，增加新的分支，检测指令是否包含`>  >>  or <`，如果包含，就对其进行分割，并标记相关特征位，而在子进程阶段，在进程替换之前，使用`dup2`进行重定向，然后就行了。

之前我们在说进程地址空间的时候，曾提到过，进程地址空间范围用户层面和内核层面，用户层面是进程的代码和数据，内核层面则是一些诸如`struct task_struct`（进程控制块），`struct files_struct`， `mm_struct`（描述进程地址空间的）， `页表`， `struct file`等， 这些是系统用于管理进程或者与进程相关资源的数据。

进程替换只会替换用户层面的一部分，至于内核层面，则几乎完全不会动，所以进程替换不会影响替换之前的重定向。

## others

最后，让我们说说其它内容。

- 为什么显示器文件分为标准输出流和标准错误流
- 再次理解“Linux下一切皆文件”

--------------------------

显示器分为标准输出流和标准错误流的原因其实很简单：就是为了分流。比如之后如果我们要写日志系统的话，可以把程序中的关键步骤信息通过重定向的方式写入日志当中，这样，当程序出问题之后，就能更快地定位到错误步骤对其进行修正。

而对于错误流，则不进行重定向，如果程序失败，直接在屏幕上打印简短的错误信息，提示用户当前程序出现了问题，具体什么问题不清楚，需要查看日志来进行确认。

当然，现在我们不写日志，所以就随便找找信息打印看看效果。

![image-20241114151511927](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141515038.png)

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ > logbook.txt
[wind@starry-sky Debug]$ ./redirect
out
out
out
out
error
error
error
error
[wind@starry-sky Debug]$ ./redirect 1 > logbook.txt
error
error
error
error
[wind@starry-sky Debug]$ cat logbook.txt
out
out
out
out
[wind@starry-sky Debug]$ ./redirect > logbook.txt
error
error
error
error
[wind@starry-sky Debug]$ cat logbook.txt
out
out
out
out
[wind@starry-sky Debug]$
```

在输出重定向中，默认对1进行重定向。

这是对1和2都进行重定向

```shell
[wind@starry-sky Debug]$ ./redirect 1>normal.log 2>err.log
[wind@starry-sky Debug]$ cat normal.log
out
out
out
out
[wind@starry-sky Debug]$ cat err.log
error
error
error
error
[wind@starry-sky Debug]$
```

也可以把1和2重定向到一个文件里

```shell
[wind@starry-sky Debug]$ ls
err.log  logbook.txt  makefile  normal.log  redirect
[wind@starry-sky Debug]$ rm err.log normal.log
[wind@starry-sky Debug]$ ./redirect 1>all.log 2>&1
[wind@starry-sky Debug]$ cat all.log
error
error
error
error
out
out
out
out
[wind@starry-sky Debug]$
```

在1重定向到 `all.log` 后，`1` 的文件描述符指向描述 `all.log` 的文件结构体。接着，当2被重定向时，系统将 `1` 对应的文件描述符（即指向 `all.log` 的 `struct file*`）复制到 `2` 的位置。这样，标准错误输出也会被写入到同一个文件 `all.log` 中，最终两者的输出都指向同一个文件。

----------------

~~现在让我们再次理解“Linux下一切皆文件”。~~

~~计算机中的一切操作，归根结底都是通过进程进行的。而当前我们对于文件的所有访问都是依赖于进程进行的。~~

~~计算机中的各种外部设备，其实都可以视为信息交流的媒介，既然是交流，就必然会包括读写这两个基本方法，生产硬件的厂商，会为自己的硬件设计专门的读写方法，也就是驱动级别的接口，尽管硬件驱动的具体原理天差地别，但它们都遵循着一套标准，使得它们的接口都非常相似，比如对于磁盘来说，可能会有`read_disk()  write_disk()`，而对于显示器来说，可能有`read_tty()  write_tty()`，一般来说，显示器不支持读，那没关系，让`read_tty()`内部为空，里面什么指令都没有就行了；对于键盘，可能有`read_kekboard()  write_kekboard()`，没有写方法，那就把`write_kekboard()`置空即可。当然，除此之外，硬件肯定还有其他接口，比如状态检查。~~

~~操作系统为了链接这些驱动层面的接口，会为打开的硬件，创建对应的`struct operation_func`结构体对象，在该结构体中，存储着一系列的函数指针，这些指针指向驱动层面的那些接口。~~

~~于此同时，系统会为打开的硬件创建对应的`struct file`对象，在该对象中，存储着指向`struct operation_func`的指针，这样，所有的硬件都被系统抽象成了文件，对硬件的种种操作，都是通过进程对文件的操作间接实现的。而文件操作的系统接口则会调用`struct operation_func`中的指针成员，这样，所有硬件的打开，都是对其抽象文件的open，所有硬件的读写，都是对其抽象文件的read和write。~~

~~我们把硬件对应的那些抽象文件统称为虚拟文件系统。~~

~~在大量的工程范例中，诸如操作系统这种超大规模工程，通过一系列的中间层，对各种对象（比如上面的硬件）进行层层继承，封装，多态，这些工程中处处体现着面向对象的思想，不是人们先凭空想到面向对象好，所以发明了一个又一个面向对象语言，而是因为，大量的工程实践都有面向对象的需求，所以有了面向对象语言，所以说面向对象是一种趋势，一种不可阻挡的趋势。~~

------------------

让我们再次理解“Linux下一切皆文件”的含义。

计算机中的所有操作，归根结底都依赖于进程。而对文件的访问，也都是通过进程来完成的。

计算机中的外部设备，实际上可以看作是信息交流的媒介。既然是交流，就必然包含读写操作。硬件厂商为各自的设备设计了专门的读写方法，即驱动级别的接口。虽然不同硬件的驱动原理各异，但它们遵循统一的标准，使得接口形式非常相似。例如，磁盘可能有 `read_disk()` 和 `write_disk()`，而显示器则可能有 `read_tty()` 和 `write_tty()`，如果显示器不支持读操作，`read_tty()` 可以为空函数。类似地，键盘可能有 `read_keyboard()` 和 `write_keyboard()`，如果没有写操作，`write_keyboard()` 也可以为空。

操作系统为了链接这些硬件驱动接口，会为每个打开的硬件创建对应的 `struct operation_func` 结构体，结构体中存储着指向驱动接口函数的指针。同时，系统会为每个打开的硬件创建对应的 `struct file` 对象，指向 `struct operation_func` 的指针存储在其中。这样，操作系统将硬件抽象为文件，通过进程对文件的操作间接实现对硬件的操作。文件操作的系统接口会调用 `struct operation_func` 中的指针成员，实现对硬件的 `open`、`read` 和 `write` 操作。

这些抽象出来的硬件文件统称为**虚拟文件系统**。

在大规模工程中，例如操作系统这类复杂的系统，常常通过中间层对各类对象（如硬件）进行封装、继承和多态。这种设计思路处处体现着面向对象的思想。面向对象的概念并不是人们凭空想出的，而是因为大量工程实践中需要这种方式，才促使了面向对象编程语言的诞生。因此，面向对象编程是一种趋势，是不可逆转的工程需求。

![image-20241114184705193](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141847262.png)

最后，让我们看看内核原码

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411141734613.mp4"></video>

# end