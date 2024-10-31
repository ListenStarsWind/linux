# `ProcessControl`【进程控制】

## 进程终止

之前我们说过，当子进程死亡而父进程不回收资源，子进程就会一直处于僵死状态，我们先通过循环创建一些进程，观察子进程的僵死状态。

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
	printf("begin-> pid:%d\n", getpid());
	int i = 10;
	while (i--)
	{
		pid_t id = fork();
		if (id == 0)
		{
			int j = 5;
			while (j--)
			{
				printf("child process: pid->%d ppid->%d\n", getpid(), getppid());
				sleep(1);
			}
			exit(0);
		}
	}

	sleep(20);
	return 0;
}
```

对于父进程来说，它不会进入`if(id == 0)`，所以会跳过该分支重新循环再创建一个子进程，对于子进程来说，首先会进入`if(id == 0 )`，再打印几次之后，使用`exit`退出进程，防止它出了`if`之后创建自己的子进程。最后的`sleep(20)`可以防止父进程先结束，从而让子进程变成孤儿进程被系统收养，这样我们就看不到僵死状态了。

```shell
[wind@starry-sky Debug]$ ./ProcessControl
```

```shell
[wind@starry-sky Debug]$ while :; do ps ajx | head -1 ; ps ajx | grep ProcessControl | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2413 17331 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17332 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17333 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17334 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17335 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17336 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17337 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17338 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17339 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17340 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17341 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2413 17331 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17332 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17333 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17334 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17335 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17336 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17337 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17338 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17339 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17340 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17341 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2413 17331 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17332 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17333 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17334 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17335 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17336 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17337 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17338 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17339 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17340 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17341 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2413 17331 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17332 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17333 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17334 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17335 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17336 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17337 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17338 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17339 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17340 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17341 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2413 17331 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17332 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17333 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17334 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17335 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17336 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17337 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17338 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17339 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17340 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17341 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2413 17331 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17332 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17333 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17334 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17335 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17336 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17337 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17338 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17339 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17340 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17341 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2413 17331 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17332 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17333 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17334 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17335 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17336 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17337 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17338 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17339 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17340 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17341 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2413 17331 17331  2413 pts/0    17331 S+    1002   0:00 ./ProcessControl
17331 17332 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17333 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17334 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17335 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17336 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17337 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17338 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17339 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17340 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
17331 17341 17331  2413 pts/0    17331 Z+    1002   0:00 [ProcessControl] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
^C
[wind@starry-sky Debug]$
```

```shell[wind@starry-sky Debug]$ ./ProcessControl
begin-> pid:17331
child process: pid->17332 ppid->17331
child process: pid->17333 ppid->17331
child process: pid->17334 ppid->17331
child process: pid->17335 ppid->17331
child process: pid->17336 ppid->17331
child process: pid->17337 ppid->17331
child process: pid->17338 ppid->17331
child process: pid->17339 ppid->17331
child process: pid->17340 ppid->17331
child process: pid->17341 ppid->17331
child process: pid->17332 ppid->17331
child process: pid->17333 ppid->17331
child process: pid->17334 ppid->17331
child process: pid->17335 ppid->17331
child process: pid->17336 ppid->17331
child process: pid->17337 ppid->17331
child process: pid->17338 ppid->17331
child process: pid->17339 ppid->17331
child process: pid->17340 ppid->17331
child process: pid->17341 ppid->17331
child process: pid->17332 ppid->17331
child process: pid->17333 ppid->17331
child process: pid->17334 ppid->17331
child process: pid->17335 ppid->17331
child process: pid->17336 ppid->17331
child process: pid->17337 ppid->17331
child process: pid->17338 ppid->17331
child process: pid->17339 ppid->17331
child process: pid->17340 ppid->17331
child process: pid->17341 ppid->17331
child process: pid->17332 ppid->17331
child process: pid->17333 ppid->17331
child process: pid->17334 ppid->17331
child process: pid->17335 ppid->17331
child process: pid->17336 ppid->17331
child process: pid->17337 ppid->17331
child process: pid->17338 ppid->17331
child process: pid->17339 ppid->17331
child process: pid->17340 ppid->17331
child process: pid->17341 ppid->17331
child process: pid->17332 ppid->17331
child process: pid->17333 ppid->17331
child process: pid->17334 ppid->17331
child process: pid->17335 ppid->17331
child process: pid->17336 ppid->17331
child process: pid->17337 ppid->17331
child process: pid->17338 ppid->17331
child process: pid->17339 ppid->17331
child process: pid->17340 ppid->17331
child process: pid->17341 ppid->17331
^C
[wind@starry-sky Debug]$
```

观察一下运行结果，也能发现，进程间的先后执行顺序不是固定的，而是由调度器自己决定的。

## 终止场景

进程的终止无非三种情况：

- 代码运行完毕，结果正确
- 代码运行完毕，结果不正确
- 代码异常终止

判断结果是否正确的依据是看它是否达到我们想要的效果。如果达到需求，自然是结果正确的，达不到需求，结果就是不正确的。结果如果正确，自然不需要我们操心，我们要操心的情况是结果不正确和代码跑到一半不知道怎么就挂了。

那如何知晓结果正确与否呢？对于运行完代码的进程来说，它都会给父进程返回一个退出码，如果退出码是0，那就代表结果是正确的，是其它就代表结果不正确。

```c
#include<stdio.h>

int main()
{
	// 模拟一段逻辑的实现
	printf("Simulate the implementation of a logic segment.\n");

	// main函数的返回值就是一种退出码
	// 对于main函数框架之内的函数
	// return用于函数栈帧的相互通信
	// 或者作为休止符
	// 而对于main函数本身来说
	// 返回值是进程的退出码
	// 一般会被其父进程接收
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ./ProcessControl
Simulate the implementation of a logic segment.
[wind@starry-sky Debug]$ echo $?
0
[wind@starry-sky Debug]$ 它的父进程bash接受到了退出码^C
[wind@starry-sky Debug]$ echo $?可以打印最近的退出码^C
[wind@starry-sky Debug]$ 退出码存在一个名为？的变量里，$对其进行了转义^C
[wind@starry-sky Debug]$ echo $?
130
[wind@starry-sky Debug]$ ls ^C
[wind@starry-sky Debug]$ echo $?
130
[wind@starry-sky Debug]$ 这个130是被ctrl+c指令的退出码^C
[wind@starry-sky Debug]$
```

退出码返回给父进程很容易理解：当初创建子进程就是让它做某件事的，这事做的怎么样，子进程做完之后要报告一声。

如果退出码不是0，说明子进程没有达到父进程想要的结果，那就需要启动相应的应对策略。可以建立一套机制，明确每个返回码所代表的意思，C有自己的返回码标准，不过我们用的不多，一般都是自己建立一套映射机制。

![image-20241031143440609](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410311434867.png)

`strerror`就是C语言自带的退出码映射标准，它实际上就类似一个字符串数组，外部被封装了一下，退出码就能以下标的形式获取对应的字符串信息。

```c
#include<stdio.h>
#include<string.h>

int main()
{
	for (int i = 0; i < 200; i++)
		printf("%d->%s\n", i, strerror(i));
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make run
0->Success
1->Operation not permitted
2->No such file or directory
3->No such process
4->Interrupted system call
5->Input/output error
6->No such device or address
7->Argument list too long
8->Exec format error
9->Bad file descriptor
10->No child processes
11->Resource temporarily unavailable
12->Cannot allocate memory
13->Permission denied
14->Bad address
15->Block device required
16->Device or resource busy
17->File exists
18->Invalid cross-device link
19->No such device
20->Not a directory
21->Is a directory
22->Invalid argument
23->Too many open files in system
24->Too many open files
25->Inappropriate ioctl for device
26->Text file busy
27->File too large
28->No space left on device
29->Illegal seek
30->Read-only file system
31->Too many links
32->Broken pipe
33->Numerical argument out of domain
34->Numerical result out of range
35->Resource deadlock avoided
36->File name too long
37->No locks available
38->Function not implemented
39->Directory not empty
40->Too many levels of symbolic links
41->Unknown error 41
42->No message of desired type
43->Identifier removed
44->Channel number out of range
45->Level 2 not synchronized
46->Level 3 halted
47->Level 3 reset
48->Link number out of range
49->Protocol driver not attached
50->No CSI structure available
51->Level 2 halted
52->Invalid exchange
53->Invalid request descriptor
54->Exchange full
55->No anode
56->Invalid request code
57->Invalid slot
58->Unknown error 58
59->Bad font file format
60->Device not a stream
61->No data available
62->Timer expired
63->Out of streams resources
64->Machine is not on the network
65->Package not installed
66->Object is remote
67->Link has been severed
68->Advertise error
69->Srmount error
70->Communication error on send
71->Protocol error
72->Multihop attempted
73->RFS specific error
74->Bad message
75->Value too large for defined data type
76->Name not unique on network
77->File descriptor in bad state
78->Remote address changed
79->Can not access a needed shared library
80->Accessing a corrupted shared library
81->.lib section in a.out corrupted
82->Attempting to link in too many shared libraries
83->Cannot exec a shared library directly
84->Invalid or incomplete multibyte or wide character
85->Interrupted system call should be restarted
86->Streams pipe error
87->Too many users
88->Socket operation on non-socket
89->Destination address required
90->Message too long
91->Protocol wrong type for socket
92->Protocol not available
93->Protocol not supported
94->Socket type not supported
95->Operation not supported
96->Protocol family not supported
97->Address family not supported by protocol
98->Address already in use
99->Cannot assign requested address
100->Network is down
101->Network is unreachable
102->Network dropped connection on reset
103->Software caused connection abort
104->Connection reset by peer
105->No buffer space available
106->Transport endpoint is already connected
107->Transport endpoint is not connected
108->Cannot send after transport endpoint shutdown
109->Too many references: cannot splice
110->Connection timed out
111->Connection refused
112->Host is down
113->No route to host
114->Operation already in progress
115->Operation now in progress
116->Stale file handle
117->Structure needs cleaning
118->Not a XENIX named type file
119->No XENIX semaphores available
120->Is a named type file
121->Remote I/O error
122->Disk quota exceeded
123->No medium found
124->Wrong medium type
125->Operation canceled
126->Required key not available
127->Key has expired
128->Key has been revoked
129->Key was rejected by service
130->Owner died
131->State not recoverable
132->Operation not possible due to RF-kill
133->Memory page has hardware error
134->Unknown error 134
135->Unknown error 135
136->Unknown error 136
137->Unknown error 137
138->Unknown error 138
139->Unknown error 139
140->Unknown error 140
141->Unknown error 141
142->Unknown error 142
143->Unknown error 143
144->Unknown error 144
145->Unknown error 145
146->Unknown error 146
147->Unknown error 147
148->Unknown error 148
149->Unknown error 149
150->Unknown error 150
151->Unknown error 151
152->Unknown error 152
153->Unknown error 153
154->Unknown error 154
155->Unknown error 155
156->Unknown error 156
157->Unknown error 157
158->Unknown error 158
159->Unknown error 159
160->Unknown error 160
161->Unknown error 161
162->Unknown error 162
163->Unknown error 163
164->Unknown error 164
165->Unknown error 165
166->Unknown error 166
167->Unknown error 167
168->Unknown error 168
169->Unknown error 169
170->Unknown error 170
171->Unknown error 171
172->Unknown error 172
173->Unknown error 173
174->Unknown error 174
175->Unknown error 175
176->Unknown error 176
177->Unknown error 177
178->Unknown error 178
179->Unknown error 179
180->Unknown error 180
181->Unknown error 181
182->Unknown error 182
183->Unknown error 183
184->Unknown error 184
185->Unknown error 185
186->Unknown error 186
187->Unknown error 187
188->Unknown error 188
189->Unknown error 189
190->Unknown error 190
191->Unknown error 191
192->Unknown error 192
193->Unknown error 193
194->Unknown error 194
195->Unknown error 195
196->Unknown error 196
197->Unknown error 197
198->Unknown error 198
199->Unknown error 199
[wind@starry-sky Debug]$
```

```shell
[wind@starry-sky Debug]$ ls
makefile  ProcessControl
[wind@starry-sky Debug]$ ls myfile
ls: cannot access myfile: No such file or directory
[wind@starry-sky Debug]$ echo $?
2
[wind@starry-sky Debug]$
```

也可以自己建立映射关系

```c
#include<stdio.h>
#include<string.h>

const char* errorstring[] = {
	"success",
	"error 1",
	"error 2"
};

const char* errstr(int i)
{
	if (i < 3)
		return errorstring[i];
	else
		return "Undefined";
}

int main()
{
	for (int i = 0; i < 10; i++)
		printf("%d->%s\n", i, errstr(i));
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make ; make run
0->success
1->error 1
2->error 2
3->Undefined
4->Undefined
5->Undefined
6->Undefined
7->Undefined
8->Undefined
9->Undefined
[wind@starry-sky Debug]$
```

接下来说一下`errno`，`erron`是C中的一个全局变量，当库函数结果不正确时，`errno`就会获得相应的错误码，之后结合`strerror`就能获得库函数不正确的原因。要注意的是，`erron`中保存的是上一次库函数不正确的错误码，如果之后又有库函数不正确，`erron`就会被刷新。

![image-20241031152102585](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410311521686.png)

```c
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>

int main()
{
	int ret = 0;
	char* space = (char*)malloc(__INT_MAX__);
	if (space == NULL)
	{
		printf("malloc error:%s\n", strerror(errno));
		ret = errno;
	}
	else
		printf("malloc success\n");
	return ret;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ make run
malloc error:Cannot allocate memory
make: *** [run] Error 12
[wind@starry-sky Debug]$ ./ProcessControl
malloc error:Cannot allocate memory
[wind@starry-sky Debug]$ echo $?
12
[wind@starry-sky Debug]$
```

当代码异常终止了，退出码就不具有可信性了。这种异常一般都是系统或者硬件层面的异常，比如对页表中标记只读的地址或者干脆就不在进程地址空间各组分范围里的地址解引用，系统就会对进程发出终止信号；又比如除数为0，导致寄存器溢出错误，系统会识别到该硬件错误，然后对进程发出终止信号。

```c
int main()
{
	int* p = NULL;
	*p = 42;
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean
[wind@starry-sky Debug]$ make
[wind@starry-sky Debug]$ make run
make: *** [run] Segmentation fault
```

```c
int main()
{
	int a = 42;
	a /= 0;
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make
../../../main.cpp: In function ‘int main()’:
../../../main.cpp:9:4: warning: division by zero [-Wdiv-by-zero]
  a /= 0;
    ^
[wind@starry-sky Debug]$ make run
make: *** [run] Floating point exception
[wind@starry-sky Debug]$
```

`kill -l`可以查看各个信号：

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

其中`8) SIGFPE`就是`Floating point exception`，`11) SIGSEGV`就是`Segmentation fault`。

现在我们写一个正常程序。

```c
#include<stdio.h>
#include<unistd.h>

int main()
{
	while (1)
	{
		printf("pid->%d\n",getpid());
		sleep(1);
	}
	return 0;
}
```

```shell
wind@starry-sky Debug]$ make clean ; make ; make run
pid->1309
pid->1309
pid->1309
pid->1309
pid->1309
```

```shell
[wind@starry-sky Debug]$ kill -8 1309
```

```shell
pid->1309
pid->1309
pid->1309
pid->1309
pid->1309
pid->1309
pid->1309
make: *** [run] Floating point exception
[wind@starry-sky Debug]$
```

```shell
[wind@starry-sky Debug]$ ./ProcessControl
pid->1477
pid->1477
pid->1477
pid->1477
```

```shell
[wind@starry-sky Debug]$ kill -11 1477
```

```shell
pid->1477
pid->1477
pid->1477
pid->1477
Segmentation fault
[wind@starry-sky Debug]$
```

## 退出操作

上面我们主要用的是`return`，严格来说，是`main`函数的`return`，对于`main`函数框架内的函数，`return`用来进行函数栈帧通信或用作停止符。对于`main`本身来说，`return`则是为父进程返回退出码。

除此之外，还有`exit`，`exit`在任意函数里都是返回退出码，它是C语言中的一个接口，使用时会强制刷新缓冲区，执行清理函数，总的来说，它会做一些善后处理。

`_exit`则是系统接口，它不会做任何善后处理，直接退出，由于进程退出了，缓冲区就会直接关闭，里面的信息就会丢失了。

随口说一声，缓冲区在进程地址空间的用户部分。`exit`实际上是先做完善后操作之后，再自己调用`_exit`。

![image-20241031163043241](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410311630441.png)

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

void f()
{
	// 检查发现
	// 该做的事被别的进程做完了
	// 那就提前收工
	printf("Finish");
	exit(0);
}

int main()
{
	f();
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make ; make run
Finish[wind@starry-sky Debug]$
```

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

void f()
{
	// 检查发现
	// 前置事项没完成
	// 那就不用去做
	printf("Impossible");
	exit(127);
}

int main()
{
	f();
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ make clean ; make ; make run
Impossiblemake: *** [run] Error 127
[wind@starry-sky Debug]$
```

```c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

void f()
{
	// 检查发现
	// 前置事项没完成
	// 那就不用去做
	printf("Impossible");
	_exit(127);
}

int main()
{
	f();
	return 0;
}
```

```shell
[wind@starry-sky Debug]$ ./ProcessControl
[wind@starry-sky Debug]$ make clean ; make ; make run
make: *** [run] Error 127
```

标准输出流没被刷新就被关闭了，里面的信息直接丢失。

# end