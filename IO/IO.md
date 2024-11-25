# IO

## preface

在从系统层面重新认识IO之前，让我们先运行几份测试代码，观察现象。

```cpp
#include<stdio.h>  // IO_c_interface
#include<string.h> // strlen
#include<unistd.h> // IO_system_interface

int main()
{
	const char* c_str = "hello fwrite\n";
	const char* s_str = "hello write\n";
	printf("hello printf\n");
	fprintf(stdout, "hello fprintf\n");
	fwrite(c_str, strlen(c_str), 1, stdout);
	write(1, s_str, strlen(s_str));
	return 0;
}
```

---------------

这里要略微说一下`fwrite`这个接口。

![image-20241116190429538](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411161904635.png)

`fwrite`与`printf`不同，它是一种非格式化函数。`printf`允许通过格式说明符（如`%d`、`%s`）指定输出格式，便于进行数据类型的转换和格式化。这在处理输出时非常重要，因为像显示器或键盘这样的字符设备最终只能处理字符数据。因此，语言接口必须将数据转化为字符形式，才能通过系统接口输出。例如，当`printf`打印整型变量时，它会先将该变量转换成字符数据，然后传递给系统接口。同样，`scanf`中的格式控制也起到了数据类型转换的作用。

`fwrite`的参数与`write`类似，使用`void*`作为数据指针，不关心数据的具体类型。它们的工作是将给定的数据直接写入输出设备，不进行任何转换或修饰，输出设备随后将这些数据作为字符解释。

`write`的参数相对简单，仅依赖一个`size_t`参数来指示写入的字节数——写多少数据就输出多少字节。而`fwrite`提供了更精细的控制，它有两个`size_t`参数：`size`和`nmemb`。`size`定义了每个数据块的字节大小，`nmemb`指定要写入的块数量。例如，对于字符串输入，`size`为字符串长度，`nmemb`为1。

需要特别注意的是，`fwrite`返回值表示成功写入的块数量，而不是字节数。`fread`的返回值具有相同的含义。

----------

先是输出到显示屏上

```shell
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ ls
IO  makefile
[wind@starry-sky Debug]$ ./IO
hello printf
hello fprintf
hello fwrite
hello write
[wind@starry-sky Debug]$
```

没什么问题，之后重定向到文件中

```shell
[wind@starry-sky Debug]$ ./IO > log.txt
[wind@starry-sky Debug]$ cat log.txt
hello write
hello printf
hello fprintf
hello fwrite
[wind@starry-sky Debug]$
```

这就有些超出预料了，打印顺序变了，原先在最末尾的系统调用现在写到了最前面。

接下来稍微修改一下代码，我们使用`close`关闭`1`

```cpp
int main()
{
	const char* c_str = "hello fwrite\n";
	//const char* s_str = "hello write\n";
	printf("hello printf\n");
	fprintf(stdout, "hello fprintf\n");
	fwrite(c_str, strlen(c_str), 1, stdout);
	//write(1, s_str, strlen(s_str));
	close(1);
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./IO
hello printf
hello fprintf
hello fwrite
[wind@starry-sky Debug]$
```

接下来去掉`\n`

```cpp
int main()
{
	const char* c_str = "hello fwrite";
	//const char* s_str = "hello write\n";
	printf("hello printf");
	fprintf(stdout, "hello fprintf");
	fwrite(c_str, strlen(c_str), 1, stdout);
	//write(1, s_str, strlen(s_str));
	close(1);
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./IO
[wind@starry-sky Debug]$
```

没有输出，用我们之前的知识解释就是缓冲区还没刷新，文件就已经关闭了，所以没有打印任何信息

接下来加上系统调用

```cpp
int main()
{
	const char* c_str = "hello fwrite";
	const char* s_str = "hello write";
	printf("hello printf");
	fprintf(stdout, "hello fprintf");
	fwrite(c_str, strlen(c_str), 1, stdout);
	write(1, s_str, strlen(s_str));
	close(1);
	return 0;
}
```

````shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./IO
hello write[wind@starry-sky Debug]$
````

能够看出来，系统接口和语言接口行为上似乎有些不同。

现在把`close`去掉，把代码恢复成原来的样子，区别是，在代码末尾，创建了一个子进程

```cpp
int main()
{
	const char* c_str = "hello fwrite\n";
	const char* s_str = "hello write\n";

	printf("hello printf\n");
	fprintf(stdout, "hello fprintf\n");
	fwrite(c_str, strlen(c_str), 1, stdout);

	write(1, s_str, strlen(s_str));

	fork();

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./IO
hello printf
hello fprintf
hello fwrite
hello write
[wind@starry-sky Debug]$ ./IO > log.txt
[wind@starry-sky Debug]$ cat log.txt
hello write
hello printf
hello fprintf
hello fwrite
hello printf
hello fprintf
hello fwrite
[wind@starry-sky Debug]$
```

我们发现重定向之后那三个语言接口的字符串又被打印了一遍。

不用担心，在学过下面的内容之后，这些现象都很容易解释。

## buffer

在过去的I/O操作中，经常会提到“缓冲区”这个概念。在文件系统中，我们提到`struct file`中存在一个内核级别的缓冲区。实际上，这两个缓冲区并不相同：缓冲区分为两类，一类是内核级的，另一类是语言层面的。

在进行I/O操作时，语言层的I/O接口会首先将数据写入语言层的缓冲区，然后再将其传输到内核级缓冲区。而系统I/O接口则跳过语言层缓冲区，直接操作内核级缓冲区。

![image-20241116205256784](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411162052885.png)

为了便于理解，我们可以将信息写入内核缓冲区视为硬件立即表现相应行为的触发条件。

此时这份代码就可以做出一定解释了

```cpp
int main()
{
	const char* c_str = "hello fwrite";
	//const char* s_str = "hello write\n";
	printf("hello printf");
	fprintf(stdout, "hello fprintf");
	fwrite(c_str, strlen(c_str), 1, stdout);
	write(1, s_str, strlen(s_str));
	close(1);
	return 0;
}
```

为什么在这份代码中，直接执行只会打印系统接口的字符串？

这是因为语言接口先将数据写入应用缓冲区，而这些数据没有触发刷新机制，所以它们保留在应用缓冲区中。而系统接口则直接将数据写入内核缓冲区，因此显示器会立即显示对应的字符。当`1`被关闭后，应用缓冲区中的数据就无法再写入内核缓冲区，导致这些内容不会显示。

那怎么触发应用缓冲区的刷新机制呢？对于显示器文件来说，是的，刷新机制还和打开的文件种类有关，后面会说的，`\n`就可以触发应用缓冲刷新机制。所以在下面的代码中

```cpp
int main()
{
	const char* c_str = "hello fwrite\n";
	const char* s_str = "hello write\n";

	printf("hello printf\n");
	fprintf(stdout, "hello fprintf\n");
	fwrite(c_str, strlen(c_str), 1, stdout);

	write(1, s_str, strlen(s_str));

	fork();

	return 0;
}
```

它们都被打印出来了

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./IO
hello printf
hello fprintf
hello fwrite
hello write
[wind@starry-sky Debug]$
```

这是因为数据都最终刷新到了内核缓冲区。语言接口会先写入应用缓冲区，然后再迅速转移到内核缓冲区，而系统接口直接写入内核缓冲区。由于代码从上到下顺序执行，所以输出顺序也与代码一致。

为什么显示器文件会使用`\n`触发刷新？因为显示器用于人类观察，`\n`是换行符，视觉效果明显，常用于字符串分隔符，因此一行写完后立刻刷新，称为“行刷新”。

此时就要提到`exit and _exit`的区别了，之前我们说`exit`做了一些善后处理，`_exit`没有，现在我们就可以更深刻地认识它们之间的差异：`exit`是语言接口，它会把语言缓冲区中的内容移到内核缓冲区，然后自己再调用`_exit`；`_exit`是系统接口。它不管语言层面有什么东西，它会直接让进程和文件断开关系，直接退出进程。

---------------

现在我们说一说应用缓冲区的刷新策略，内核缓冲刷新策略就先不说了

- 无缓冲
- 行缓冲
- 全缓冲

无缓冲机制就是让数据在应用缓冲走个过场，有数据写入应用缓冲之后，立刻调用`write`把数据挪到内核缓冲中，之后再清空自身内容（也可能不清空，从头开始覆写），毕竟数据已经被传到内核你了，原来的旧数据可以不要了。

对于显示器文件来说，其缓冲策略是行缓冲，只有一行写完之后，才会把应用缓冲中的数据写到内核缓冲中，何以判断一行有没有写完？通过有无`\n`来判断，同样，在把数据写到内核缓冲后，原数据也会不被认为是有效的。

对于普通文件来说，其缓冲策略是全缓冲。普通文件，又不会被实时观看，所以缓冲策略更侧重最大效率，而不是及时性，只有缓冲区都满了，才会把其内部数据一次性移到内核数据中，而后，应用缓冲中的旧数据就会被视为失效的。

除此之外，还有第四种刷新策略：进程退出时自动进行语言层面的刷新。这个没什么额外条件，不管什么文件，不论有无`\n`，进程退出都会进行刷新。

当然，要是刷新之前文件已经被关闭了，那此时的刷新就是一种假刷新，可能语言接口没有异常行为，不会报错，返回值可能也正常，但由于没有真的把数据写到内核缓冲区中，所以硬件不会有响应。

---------------

为什么要在语言层面再设计一个缓冲区呢？主要有以下原因：

- 增加效率：这是绝大多数缓冲区都有的性质，对数据进行一定积累后在一次性转移，操作上更为便捷高效。

  打个形象的比喻。如果没有快递公司，想把一件物品送到较远的地方就需要自己亲手去送。但有了快递，就可以把物品交给驿站，然后自己就不用管了，可以做自己的事。

- 抹平设备差异性：不同的操作系统会有不同的接口，尽管功能相似，但接口毕竟不同，Linux的接口不能用在Windows和macOS上，反之也亦然。C语言标准库通过穷举的方式给每一套系统都进行专门设计，再将它们编译成链接库，这样`printf`不论在哪个系统都是有效的。使用C接口的代码，不论在哪个系统里都能编译运行。

- 格式化操作：键盘和显示屏都是字符设备，它们输入输出的都是字符，这意味着，如果再键盘上输入`123`字符，如果以`%d`格式读取。就要对字符`123`进行转录，从而生成整型`123`。

- 权限分级：系统有系统层面的权限，它通过中间层来实现，同样的，语言也有语言层面的权限，同样是通过中间层来实现，有了语言层面的缓冲区后，就能在语言层面对文件权限进行管控。

数据输入时，语言缓冲区从内核缓冲区读取信息，再将数据供给进程使用；数据输出时，语言缓冲区从进程读取数据写入内核缓冲区。无论输入还是输出，缓冲区都起到数据流动的作用，因此称之为“流”。

## where

“流”在何处呢？此时，又要谈到C语言中的`FILE`结构体了，实际上， `FILE` 结构体正是用于描述这种“流”的关键数据结构。因此，它是语言接口中不可或缺的参数。`FILE` 结构体包含了缓冲区的引用或访问方法，并记录了与已打开文件相关的其他信息。

每次调用 `fopen` 打开文件时，都会为该文件创建一个对应的 `FILE` 结构体实例，并以指针形式返回。后续对该文件的所有操作，都会基于这个 `FILE` 指针进行。这样，`FILE` 结构体在语言接口中充当了 I/O 操作的核心角色，实现了对文件的语言层管理。

现在我们再次回到这份代码上：

```cpp
int main()
{
	const char* c_str = "hello fwrite\n";
	const char* s_str = "hello write\n";

	printf("hello printf\n");
	fprintf(stdout, "hello fprintf\n");
	fwrite(c_str, strlen(c_str), 1, stdout);

	write(1, s_str, strlen(s_str));

	fork();

	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make
[wind@starry-sky Debug]$ ./IO
hello printf
hello fprintf
hello fwrite
hello write
[wind@starry-sky Debug]$ ./IO > log.txt
[wind@starry-sky Debug]$ cat log.txt
hello write
hello printf
hello fprintf
hello fwrite
hello printf
hello fprintf
hello fwrite
[wind@starry-sky Debug]$
```

当程序以普通方式运行时，输出文件是显示器，遵循行缓冲策略。由于 `\n` 会触发刷新机制，字符串通过语言缓冲区被立即写入内核缓冲区，刷新后缓冲区内容被视为无效。创建子进程时，它会继承父进程的 I/O 信息，但由于此时父进程的语言缓冲区已被清空，子进程无法再次输出相同数据。

当程序以重定向方式运行时，输出文件为普通文件，遵循全缓冲策略。系统调用 `write` 直接将数据写入内核缓冲区，因此在输出时无需经过语言缓冲区。而语言接口函数的输出则存储在应用层的缓冲区内，由于输出字符数较少，不足以触发缓冲区的刷新机制，导致这些数据在缓冲区中保持有效。

子进程继承了父进程的语言缓冲区内容。由于进程之间是独立调度的，无论调度器先运行父进程还是子进程，两个进程的语言缓冲区都会将相同的数据刷新到内核缓冲区中。因此，语言接口的输出在`log.txt`中会出现两次，一次来自父进程，一次来自子进程。

下面我们再写一份测试代码

```cpp
#include<stdio.h>	  // IO_c_interface
#include<string.h>	  // strlen
#include<unistd.h>	  // IO_system_interface
#include <sys/wait.h> // waitpid

int main()
{
	const char* c_str = "hello fwrite\n";
	const char* s_str = "hello write\n";

	printf("hello printf\n");
	fflush(stdout);
	sleep(3);

	fprintf(stdout, "hello fprintf\n");
	fwrite(c_str, strlen(c_str), 1, stdout);
	sleep(3);

	write(1, s_str, strlen(s_str));
	sleep(5);

	pid_t id =fork();
	if (id > 0)
	{
		waitpid(id, NULL, 0);
		sleep(4);
	}

	return 0;
}
```

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411171205761.mp4"></video>

```shell
[wind@starry-sky Debug]$ while : ; do cat log.txt; sleep 1; echo "--------------"; done
--------------
--------------
--------------
--------------
--------------
--------------
--------------
--------------
--------------
--------------
--------------
--------------
hello printf
--------------
hello printf
--------------
hello printf
--------------
hello printf
--------------
hello printf
--------------
hello printf
--------------
hello printf
hello write
--------------
hello printf
hello write
--------------
hello printf
hello write
--------------
hello printf
hello write
--------------
hello printf
hello write
--------------
hello printf
hello write
hello fprintf
hello fwrite
--------------
hello printf
hello write
hello fprintf
hello fwrite
--------------
hello printf
hello write
hello fprintf
hello fwrite
--------------
hello printf
hello write
hello fprintf
hello fwrite
--------------
hello printf
hello write
hello fprintf
hello fwrite
hello fprintf
hello fwrite
--------------
hello printf
hello write
hello fprintf
hello fwrite
hello fprintf
hello fwrite
--------------
hello printf
hello write
hello fprintf
hello fwrite
hello fprintf
hello fwrite
--------------
hello printf
hello write
hello fprintf
hello fwrite
hello fprintf
hello fwrite
^C
[wind@starry-sky Debug]$
```

在这份代码中，我们首先通过`printf`将`hello printf\n`写到语言缓冲区中，然后通过`fflush`强制刷新缓冲区，随后再把`hello fprintf\n and hello fwrite\n`通过各自的接口写到应用缓冲区中，但字符信息太少，没有触发刷新机制，随后`hello write`通过系统接口直接写到内核缓冲中，于是文件立刻就有了对应的字符信息；在创建子进程之后，子进程继承了父进程语言缓冲区中还未刷新的数据，通过阻塞等待，我们确保子进程先结束运行，随后再通过休眠让父进程退出时的缓冲刷新与子进程的刷新间隔一定时间。

## simulated_implementation

下面我们模拟实现一个基础的`<stdio.h>`

代码明显是有不足之处的，但对于我们对IO的理解已经足够了，具体代码内容，请查看源代码文件。

# end
