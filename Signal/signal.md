# Signal

## Concept

在我们的日常生活中，经常接触到各种信号。信号本质上是一种映射，承载并传递信息，帮助我们依据这些信息作出不同的反应。例如：红灯亮起，表示车辆可以通过，此时行人需要停止穿越马路；清晨闹钟响起，意味着该起床了，我们随即开始穿衣、洗漱；婴儿刚出生时，会本能地尝试吮吸乳汁。类似地，在计算机中也存在许多信号，用于在不同的组件或进程之间传递信息，驱动系统的运行。例如，键盘输入会触发中断信号，通知CPU处理输入内容；操作系统接收到硬件故障的信号，会启动相应的错误处理流程；而在多任务运行时，信号也被用来实现进程间的通信和控制。这些信号帮助计算机协调复杂的操作，类似于人类对外部信号的反应机制。

信号具有一些共同的特征。首先，无论是碳基生命还是硅基系统，能够对信号做出响应的前提是存储了信号与其含义的映射关系。对于人类来说，婴儿出生后本能地尝试吮吸乳汁，是源于基因中天生的非条件反射；而看到红灯知道需要停下，则是后天学习形成的条件反射，存储在神经系统中。人们知道闯红灯是具有危险性的，所以会停下，并且即使现在信号并没有产生，我也知道那个信号若是出现，意味着什么，又要干什么。

其次，信号被接收后，并不总是立即触发响应，因为当前可能有更高优先级的任务需要处理。因此，无论是生物系统还是计算机系统，都需要一种机制来存储信号，以便在适当的时机处理。这种机制保证了即使信号无法被及时响应，也不会因被忽略而丢失，从而实现对复杂环境的有效管理和协调。

想要利用信号，首先要学会认识信号，而认识信号可以分为三步。第一步是了解信号的外在特征，即“信号到底长什么样子”。对于刚有意识的孩子来说，他们并不知道红灯是什么样子，若直接告诉他们“红灯亮了要停下”，他们只会茫然不解。第二步是具备接收信号的能力。我们依靠眼睛看到交通灯，并能够分辨出红、黄、绿三种颜色，这使得信号可以被准确接收。第三步是理解信号的含义，掌握信号背后的信息。例如，当看到红灯亮起，我们知道这意味着车辆将通过道路，继而根据这一信息作出相应的行为。

在计算机系统中，进程扮演着人的角色。每个进程都有特定的职能，共同维护系统的稳定运行。与人类能够通过学习适应新信号不同，普通的进程并不具备自我学习能力。它们只能依赖预先定义的信号处理机制，这些机制在进程创建时便已嵌入，因此能够快速响应常见的信号。

然而，进程接收到信号后，并不一定会立刻进行处理。这是因为计算机系统需要根据任务的优先级和当前状态合理分配资源。为了实现这一点，进程通常会将信号暂存，待当前任务完成或系统繁忙程度降低后，再对信号进行处理。

对信号的学习，实际上是围绕信号生命周期展开的。信号从产生到最终被处理，其过程大致可以分为三个阶段：信号的产生、信号的保存以及信号的处理。了解这三个阶段能够帮助我们全面掌握信号的运行机制。因此，接下来我们先从信号的产生开始讲起。

### generation

先来段测试代码

```cpp
#include<iostream>
#include<unistd.h>

using namespace std;

int main()
{
    while(true)
    {
        cout<<"This is a process. pid:"<<getpid()<<endl;
        sleep(1);
    }
    return 0;
}
```

```bash
[wind@starry-sky generation]$ make clean ; make ; ./out
This is a process. pid:27341
This is a process. pid:27341
This is a process. pid:27341
This is a process. pid:27341
^C
[wind@starry-sky generation]$
```

之前我们一直说，若想终止一个前台进程，可以使用组合键`Ctrl C`，什么叫前台进程，直观上来看，前台进程的运行会导致`bash`的无响应，并且这个进程可以被`Ctrl C`终止

```shell
[wind@starry-sky generation]$ ./out
This is a process. pid:27538
lThis is a process. pid:27538
s
This is a process. pid:27538
This is a process. pid:27538
cleThis is a process. pid:27538
arThis is a process. pid:27538
This is a process. pid:27538
This is a process. pid:27538
pwd
This is a process. pid:27538
This is a process. pid:27538
This is a process. pid:27538
This is a process. pid:27538
topThis is a process. pid:27538

This is a process. pid:27538
^C
[wind@starry-sky generation]$
```

后台进程则不会导致`bash`无法响应，但无法被`Ctrl C`终止

```shell
[wind@starry-sky generation]$ ./out &
[1] 27604
[wind@starry-sky generation]$ This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
ls
main.cc  makefile  out
[wind@starry-sky generation]$ This is a process. pid:27604
This is a process. pid:27604
clThis is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
pwd
-bash: clpwd: command not found
[wind@starry-sky generation]$ This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
pwd
/home/wind/projects/Signal/Concept/generation
[wind@starry-sky generation]$ This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
^C
[wind@starry-sky generation]$ This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604
^C
[wind@starry-sky generation]$ This is a process. pid:27604
This is a process. pid:27604
This is a process. pid:27604

```

此时要终止该后台进程，就需要使用`kill -9`指令，为了方便用户知晓`PID`，后台进程启动时会回显`pid`。因为这里的后台进程是会打印字符的，为了避免打印的字符干扰命令行输入，应该创建一个新的窗口。

```bash
[wind@starry-sky generation]$ kill -9 27604
```

在Linux中，一个终端（对于Xshell来说，可能应该叫会话窗口），一般只有一个`bash`进程与之对应，并且，一个终端只对应一个前台进程，当我们以前台方式运行自己的进程时，`bash`就变成了后台进程，只有前台进程才能获取键盘输入，所以`bash`变成后台进程之后就没反应了，也就是说，前后台进程的本质区别，就是能否获得键盘输入。当`out`成为后台进程之后，它就读不到`Ctrl C`，自然无法用`Ctrl C`终止，`bash`对`Ctrl C`做了特殊处理，毕竟`bash`要是没了怎么操作系统呢？

组合键`Ctrl C`会被进程解释为`2`号信号，该信号的作用是终止进程。

```shell
[wind@starry-sky generation]$ kill -l
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
[wind@starry-sky generation]$
```

要注意的是这里没有没有`0`号信号，但实际上`0`号信号是确实存在的，它没有实际功能，仅作为测试信号，用于验证进程能否接收到信号，信号的传送通路是否畅通，除此之外，还要注意，没有`32`和`33`号信号，`[1, 31]`号信号被称为标准信号，这些信号由操作系统预设，并且用于处理常见的操作系统任务和进程管理。`[34, 64]`号信号被称为实时信号，这些信号接收到就会立即响应，所以用于对实时性要求极高的系统，比如汽车系统，高频交易系统等，对于我们来说，用不到，所以我们后面只说`[1, 31]`号的信号，`31`和`32`这两个空缺最开始是考虑到系统扩展才空缺下来的，是为了以后可能出现的标准信号留位置的，但后来一直用不到，所以就一直空着。

在代码层面，信号以宏的形式存在，也就是说，信号是用数字表示的。

计算机或许不认识字符，但数字肯定是认得的，在系统眼里，`SIGHUP`就长成`1`的样子，`SIGINT`就长成`2`的样子....当然，在内存中是二进制形式。进程对信号的处理只包括三种情况，1.  执行默认方法； 2. 忽略； 3. 执行用户自定义行为，其中第三种处理方式一般也被叫做信号的捕捉。

信号对应的默认行为其实就是一种函数，绝大多数信号对应的行为函数可由系统接口`signal`进行重写，C++的虚函数重写说不定就是从这借鉴过去的，它们的重写原理相似，都是使用了一个中间层，具体细节在此就不多说了。

![image-20241220193322733](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241220193322838.png)

```cpp
#include <signal.h>

typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler);
```

`signum`就是信号，可以用数字，也可以用宏，`handler`是个函数指针，指向用户的自定义函数。

为什么`handler`有个`int`参数呢？这个`int`参数实际就是被捕获到的信号`signum`，这个`int`参数的意义就是让用户可以在`handler`把多个信号的行为都写进去，用条件语句分成多个独立的执行流，这样就可以把所有信号的自定义行为统统写一个`handler`函数中。

```cpp
#include<iostream>
#include<unistd.h>
#include<signal.h>

using namespace std;

// 为了方便区分 这里把参数名取为event
// 因为信号其实也映射着某种行为
// 所以可以把信号抽象成行为
void handler(int event)
{
    if(event == SIGINT)
    {
        cout<<"这是2号信号:SIGINT"<<endl;
        cout<<"它的默认行为是进程终止"<<endl;
        cout<<"现在它的行为函数被重写了"<<endl;
    }
}

int main()
{
    signal(SIGINT, handler); // 放置一个陷阱 用于捕获信号
    while(true)
    {
        cout<<"This is a process. pid:"<<getpid()<<endl;
        sleep(1);
    }
    return 0;
}
```

```shell
[wind@starry-sky generation]$ ./out
This is a process. pid:4060
This is a process. pid:4060
^C这是2号信号:SIGINT
它的默认行为是进程终止
现在它的行为函数被重写了
This is a process. pid:4060
This is a process. pid:4060
This is a process. pid:4060
This is a process. pid:4060
This is a process. pid:4060
^C这是2号信号:SIGINT
它的默认行为是进程终止
现在它的行为函数被重写了
This is a process. pid:4060
This is a process. pid:4060
This is a process. pid:4060
^C这是2号信号:SIGINT
它的默认行为是进程终止
现在它的行为函数被重写了
This is a process. pid:4060
This is a process. pid:4060
This is a process. pid:4060
Killed
[wind@starry-sky generation]$
```

因为行为函数被重写了，所以`Ctrl C`不再具有原先使进程终止的功能，而是展现用户自己写的行为。最后是在另一个窗口`kill -9`才退出的。另外说一下，陷阱只要设置一下，后面就都有效，`handler`是处理信号的时候才执行，放陷阱的时候不会执行。

我们也可以再运行一次，这次我们不使用`Ctrl C`，而是使用`kill -2`产生信号。

```shell
[wind@starry-sky generation]$ ./out
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
这是2号信号:SIGINT
它的默认行为是进程终止
现在它的行为函数被重写了
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
这是2号信号:SIGINT
它的默认行为是进程终止
现在它的行为函数被重写了
This is a process. pid:5056
This is a process. pid:5056
这是2号信号:SIGINT
它的默认行为是进程终止
现在它的行为函数被重写了
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
这是2号信号:SIGINT
它的默认行为是进程终止
现在它的行为函数被重写了
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
This is a process. pid:5056
Killed
[wind@starry-sky generation]$
```

```shell
[wind@starry-sky generation]$ kill -2 5056
[wind@starry-sky generation]$ kill -2 5056
[wind@starry-sky generation]$ kill -2 5056
[wind@starry-sky generation]$ kill -2 5056
[wind@starry-sky generation]$ kill -9 5056
[wind@starry-sky generation]$
```

--------------------

下面我们暂时中断一下信号的讲解，先说说键盘上的组合键`Ctrl C`是怎么变成`2`号信号的。

在 Linux 中，一切皆文件。对于键盘，系统为其提供了抽象文件的表示，同时在内存中有对应的内核缓冲区。当键盘输入新的数据时，系统会通过 I/O 子系统将这些数据拷贝到内核缓冲区中。但问题在于，系统是如何得知键盘有新数据的呢？轮询检查每个外设的状态吗？显然，这样的效率太低。

为了解决这一问题，就涉及到计算机中的控制总线。虽然 CPU 不直接读取外设的数据，但它可以通过控制总线对外设进行控制。当外设有更新时，会通过控制总线向 CPU 发送中断信号。本质上，中断信号就是在某个特定的物理地址上发送高低电平信号——高电平表示 `1`，低电平表示 `0`。利用这些信号的变化，形成了二进制数据。

以键盘为例，当键盘上有新数据时，键盘会向 CPU 发送它对应的中断号。CPU 随后会处理这个中断号，并执行与之对应的方法（更准确地说，是指令下发给 I/O 子系统中的相关芯片，CPU 本身不直接操作外设）。那么这些方法是从哪里来的呢？

这就依赖于系统启动时加载的中断向量表。中断向量表是一张存储在内存中的表格，本质是一个函数指针数组，每个指针都指向一个处理对应外设的具体方法。当中断信号到达时，CPU 通过中断号查找中断向量表，调用对应的函数。

因此，系统并不是主动感知外设数据的变化，而是依赖于外设主动发出的中断信号。当外设通知了 CPU，它只需要根据中断号调用中断向量表中的方法即可完成处理。

键盘里的某些按键所发送的数据大致分为两类，一是输入数据，会被拷贝到内核缓冲区中，二是控制数据，比如`Ctrl C`，控制数据用于进行控制，所以不会被拷贝到内核缓冲区中，系统会对`Ctrl C`进行解释，将它解释成`2`号信号，再把这个`2`信号传给前台进程。

系统收到中断信号后，不仅仅会进行IO处理，也可能会唤醒一些进程，比如某些进程在等待键盘输入，当键盘发送中断信号后，一方面，会进行IO处理，另一方面，IO处理这也意味着原先因为读不到键盘输入而阻塞的进程可以被唤醒了，于是系统就会唤醒这些进程。

另外，为了在视觉层面突出键盘的输入，键盘的输入数据往往也会被写到显示屏里，这种行为就被称为回显。

--------------

刚刚我们提到硬件中断，其实在软件层面，信号可以被看作一种软件中断。信号的设计思路借鉴了硬件中断，两者有许多相似之处。比如，它们都具有相同的特点：

异步性。信号的产生与进程的执行流是相互独立的，也就是说，在信号产生之前，进程可以专注于自己的任务；当信号产生时，它的产生过程与进程当前的工作没有直接关联，各自独立进行。而当信号需要被处理时，进程会暂停当前任务，执行信号对应的处理逻辑。

更具体地说，异步性可以这样理解：在信号产生之前，我正常执行自己的操作；当信号产生时，它只是一个“通知”，不会立即打断我的执行流；而当轮到信号处理时，我会跳到信号处理函数，执行对应的行为。信号处理完毕后，我会从跳过去的位置无缝返回，继续执行未完成的任务，就像什么都没发生过一样。

---------------

前面说了这么多，其实都是在铺垫，还没说到信号的产生，信号的产生有三种方案：

1. 用键盘输入控制数据

   除了`Ctrl C`之外，还有一些组合键也可以被解释为信号，比如`Ctrl /`会被解释成`3`号信号

   ```cpp
   void handler(int event)
   {
       if(event == SIGINT)
       {
           cout<<"这是2号信号:SIGINT"<<endl;
           cout<<"它的默认行为是进程终止"<<endl;
           cout<<"现在它的行为函数被重写了"<<endl;
       }
       else if(event == SIGQUIT)
       {
           cout<<"这是3号信号:SIGQUIT"<<endl;
           cout<<"它的默认行为也是终止进程"<<endl;
           cout<<"现在我们不考虑区别"<<endl;
       }
   }
   
   int main()
   {
       signal(3, handler); // 放置一个陷阱 用于捕获信号
       while(true)
       {
           cout<<"This is a process. pid:"<<getpid()<<endl;
           sleep(1);
       }
       return 0;
   }
   ```

   ```shell
   [wind@starry-sky generation]$ ./out
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   这是3号信号:SIGQUIT
   它的默认行为也是终止进程
   现在我们不考虑区别
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   这是3号信号:SIGQUIT
   它的默认行为也是终止进程
   现在我们不考虑区别
   This is a process. pid:9876
   This is a process. pid:9876
   这是3号信号:SIGQUIT
   它的默认行为也是终止进程
   现在我们不考虑区别
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   This is a process. pid:9876
   Killed
   [wind@starry-sky generation]$
   ```

   ```shell
   [wind@starry-sky generation]$ kill -3 9876
   [wind@starry-sky generation]$ kill -3 9876
   [wind@starry-sky generation]$ kill -3 9876
   [wind@starry-sky generation]$ kill -9 9876
   [wind@starry-sky generation]$
   ```

   再来一个`Ctrl Z`，对应的是`19`号信号

   ```cpp
   void handler(int event)
   {
       if (event == SIGINT)
       {
           cout << "这是2号信号:SIGINT" << endl;
           cout << "它的默认行为是进程终止" << endl;
           cout << "现在它的行为函数被重写了" << endl;
       }
       else if (event == SIGQUIT)
       {
           cout << "这是3号信号:SIGQUIT" << endl;
           cout << "它的默认行为也是终止进程" << endl;
           cout << "现在我们不考虑区别" << endl;
       }
       else if (event == 19)
       {
           cout << "这是19号信号:SIGSTOP" << endl;
           cout << "它的默认行为是暂停进程" << endl;
           cout << "它不能被重写" << endl;
           cout << "继续信号是18" << endl;
           cout << "这是无法被执行的分支" << endl;
       }
       else if (event == SIGKILL)
       {
           cout << "这是无法被执行的分支" << endl;
           cout << "它不能被重写" << endl;
           cout << "这是9号信号:SIGKILL" << endl;
           cout << "用于强制终止进程" << endl;
           cout << "上面两个终止是请求" << endl;
           cout << "不具有强制性" << endl;
       }
       else
       {
           // 对于标准信号
           // 只有9号和19号不支持重写
           // 9号是强制终止
           // 如果被重写
           // 进程就关不掉了
           // 当某些进程出现问题时
           // 由于它很重要
           // 所以不能直接强制终止
           // 所以就暂时停止
           // 以减少损失
           cout << "process get a signal:" << event << endl;
       }
   }
   
   int main()
   {
       for (int i = 0; i <= 31; i++)
       {
           signal(i, handler); // 放置一个陷阱 用于捕获信号
       }
       while (true)
       {
           cout << "This is a process. pid:" << getpid() << endl;
           sleep(1);
       }
       return 0;
   }
   ```

   ```shell
   [wind@starry-sky generation]$ ./out
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   This is a process. pid:13351
   
   [1]+  Stopped                 ./out
   [wind@starry-sky generation]$ ^C
   [1]+  Killed                  ./out
   [wind@starry-sky generation]$
   ```

   ```shell
   [wind@starry-sky generation]$ kill -19 13351
   [wind@starry-sky generation]$ kill -9 13351
   [wind@starry-sky generation]$
   ```

   我们可以看到`9`号和`19`都是能发挥功能的，因为是在暂停的状态下强制终止的，所以`[1]+  Killed                  ./out`最开始没打印出来，需要`^C`刷新一下缓冲区。总要留几个信号不支持重写，否则进程就没有软肋了，不好控制，就比如日本那边对于35岁以上没工作，没妻儿，没积蓄的人叫做“无敌的人”，不能出现无敌进程，不能小看布衣之怒：“若士必怒，伏尸二人，流血五步，天下缟素，今日是也。”，无敌进程要是把系统干坏了就不好了。

2. `kill`命令

   这个就不说了，用过很多次了。

3. 系统调用

   比如，有个系统接口叫做`kill`

   ![image-20241221094235017](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241221094235153.png)

   下面我们使用命令行参数，实现一个命令行`kill`程序。就叫`slay`(终结)吧。

   ```cpp
   #include <sys/types.h>
   #include <signal.h>
   #include<iostream>
   
   using namespace std;
   
   int main(int argc, char* argv[])
   {
       if(argc == 3)
       {
           int signal = stoi(argv[1]);
           int pid = stoi(argv[2]);
           if(kill(pid, signal))
           {
               perror("filed kill");
           }
       }
       else
       {
           cout << "-bash: saly: Operation not permitted"<<endl;
       }
       return 0;
   }
   ```

   ```shell
   #  注：为了方便观察 已经取消out的所有信号捕获
   [wind@starry-sky generation]$ ./out
   This is a process. pid:16360
   This is a process. pid:16360
   This is a process. pid:16360
   
   [1]+  Stopped                 ./out
   [wind@starry-sky generation]$ This is a process. pid:16360
   This is a process. pid:16360
   This is a process. pid:16360
   This is a process. pid:16360
   ^C
   [1]+  Killed                  ./out
   [wind@starry-sky generation]$
   ```

   ```shell
   [wind@starry-sky generation]$ ./slay 19 16360
   [wind@starry-sky generation]$ ./slay 18 16360
   [wind@starry-sky generation]$ ./slay 9 16360
   [wind@starry-sky generation]$
   ```

   有个语言接口叫`raise`

   ![image-20241221101805600](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241221101805717.png)

   它的功能是给自己发信号，实际上就是`kill(getpid(), sig);`

   ```cpp
   #include <sys/types.h>
   #include <signal.h>
   #include<unistd.h>
   #include<iostream>
   
   using namespace std;
   
   int main()
   {
       int count = 5;
       while (true)
       {
           cout << "This is a process. pid:" << getpid() << endl;
           sleep(1);
           count--;
           if(!count)
           {
               raise(19);
           }
       }
   
       return 0;
   }
   ```

   ```shell
   [wind@starry-sky generation]$ ./a
   This is a process. pid:17316
   This is a process. pid:17316
   This is a process. pid:17316
   This is a process. pid:17316
   This is a process. pid:17316
   
   [1]+  Stopped                 ./a
   [wind@starry-sky generation]$ ^C
   [wind@starry-sky generation]$ kill -9 17316
   [wind@starry-sky generation]$ ^C
   [1]+  Killed                  ./a
   [wind@starry-sky generation]$
   ```

   语言接口`abort`有些特殊，它是对自己发送`6`号信号`SIGABRT `，`SIGABRT `也是退出信号，不过是用于异常情况的退出，类似与关键步骤函数调用失败是使用的`_exit(int)`，无论`SIGABRT`的行为函数是否被重写，它都保证进程能退出，可以认为它内部又对`SIGABRT`的行为函数进行了重写，重写成了默认行为的样子，然后又对自己发送`SIGABRT`信号。

   ![image-20241221104108430](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241221104108545.png)

   ```cpp
   #include <sys/types.h>
   #include <signal.h>
   #include<unistd.h>
   #include<iostream>
   
   using namespace std;
   
   void handler(int event)
   {
           cout << "process get a signal:" << event << endl;
   }
   
   int main()
   {
       signal(SIGABRT, handler);
       int count = 5;
       while (true)
       {
           cout << "This is a process. pid:" << getpid() << endl;
           sleep(1);
           count--;
           if(!count)
           {
               abort();
           }
       }
   
       return 0;
   }
   ```

   ```shell
   [wind@starry-sky generation]$ ./a
   This is a process. pid:18422
   This is a process. pid:18422
   This is a process. pid:18422
   This is a process. pid:18422
   This is a process. pid:18422
   process get a signal:6
   Aborted
   [wind@starry-sky generation]$
   ```

   若是`abort`没有被重写，那就只展现`abort`的默认行为，如果`abort`被重写了，就先执行用户的自定义行为，在执行默认行为。

   4. 异常
   
      之前我们在说进程结果的时候, 曾经提到过异常这个词, 一般来说, 进程异常结束, 意味着其没有运行完整个代码逻辑, 跑到一半就挂掉了.  我们之前也说, 异常是因为系统对进程发送信号造成的, 接下来, 我们就把其中的细节说一下.
   
      为了方便起见, 我们先故意写一个会引发异常的程序.
   
      ```cpp
      #include<sys/types.h>
      #include<unistd.h>
      #include<iostream>
      
      using namespace std;
      
      int main()
      {
          cout<<"pid:"<<getpid()<<endl;
          cout<<"exception before"<<endl;
          sleep(1);
      
          int n = 1 / 0;
      
          sleep(1);
          cout<<"exception after"<<endl;
      
          return 0;
      }
      ```
   
      ```shell
      [wind@starry-sky generation]$ g++ exception.cc
      exception.cc: In function ‘int main()’:
      exception.cc:13:15: warning: division by zero [-Wdiv-by-zero]
         13 |     int n = 1 / 0;
            |             ~~^~~
      [wind@starry-sky generation]$ ./a.out
      pid:27753
      exception before
      Floating point exception
      [wind@starry-sky generation]$ kill -l
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
      [wind@starry-sky generation]$ 
      ```
   
      异常原因:`Floating point exception`, 也就是八号信号 `SIGFPE`, `man 7 signal`可以查看信号的默认行为
   
      ```shell
      Signal     Value     Action   Comment
      ──────────────────────────────────────────────────────────────────────
      SIGHUP        1       Term    Hangup detected on controlling terminal
      or death of controlling process
      SIGINT        2       Term    Interrupt from keyboard
      SIGQUIT       3       Core    Quit from keyboard
      SIGILL        4       Core    Illegal Instruction
      SIGABRT       6       Core    Abort signal from abort(3)
      SIGFPE        8       Core    Floating point exception
      SIGKILL       9       Term    Kill signal
      SIGSEGV      11       Core    Invalid memory reference
      SIGPIPE      13       Term    Broken pipe: write to pipe with no
      readers
      SIGALRM      14       Term    Timer signal from alarm(2)
      SIGTERM      15       Term    Termination signal
      SIGUSR1   30,10,16    Term    User-defined signal 1
      SIGUSR2   31,12,17    Term    User-defined signal 2
      SIGCHLD   20,17,18    Ign     Child stopped or terminated
      SIGCONT   19,18,25    Cont    Continue if stopped
      SIGSTOP   17,19,23    Stop    Stop process
      SIGTSTP   18,20,24    Stop    Stop typed at terminal
      SIGTTIN   21,21,26    Stop    Terminal input for background process
      SIGTTOU   22,22,27    Stop    Terminal output for background process
      ```
   
      我们看到, `SIGFPE`的默认行为是`Core`,  `core`意为核心, 或者说, 一个事物中最重要的部分, 为什么叫核心等会再说, 现在只需要知道`Core`和`Term`都是进程终止,  只不过`Trem(Terminate)`是有善后处理的终止,比如刷新缓冲区之类的,  `Core`则是没有善后直接终止.
   
      为了验证进程确实是收到八号信号而终止的, 我们可以对八号信号进行捕捉, 我个人更喜欢用重写这种词来描述, 如果重写之后, 进程行为确实发生了改变, 那就意味着进程确实是因为收到八号信号而终止的.
   
      ```cpp
      #include<sys/types.h>
      #include<signal.h>
      #include<unistd.h>
      #include<iostream>
      
      using namespace std;
      
      void handler(int event)
      {
          cout<<"接收到一个信号: "<<event<<endl;
          sleep(1);
      }
      
      int main()
      {
          signal(SIGFPE, handler);
          cout<<"pid:"<<getpid()<<endl;
          cout<<"exception before"<<endl;
          sleep(1);
      
          int n = 1 / 0;
      
          sleep(1);
          cout<<"exception after"<<endl;
      
          return 0;
      }
      ```
   
      ```shell
      [wind@starry-sky generation]$ ./a.out
      pid:29183
      exception before
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      接收到一个信号: 8
      ^C
      [wind@starry-sky generation]$
      ```
   
      我们发现了两个现象, 一是进程的行为确实依据我们写的重写函数来运行的, 二是, 尽管在重写函数里我们没有退出, 但程序的逻辑并没有继续往下走, 而是一直不停的重复执行重写逻辑. 
   
      下面我们再换一个异常, 重复上面的步骤,
   
      ```cpp
      int main()
      {
          cout<<"pid:"<<getpid()<<endl;
          cout<<"exception before"<<endl;
          sleep(1);
      
          int* p = nullptr;
          *p = 1;
      
          sleep(1);
          cout<<"exception after"<<endl;
      
          return 0;
      }
      ```
   
      ```shell
      [wind@starry-sky generation]$ ./a.out
      pid:29693
      exception before
      Segmentation fault
      [wind@starry-sky generation]$
      ```
   
      这是日常生活中最常见的错误:: 段错误,  我们知道, 内存实际上被分成一个一个段, 或者说 块,  所谓段错误其实就是内存错误, 当程序试图访问超出权限的内存空间, 或者访问未映射的进程空间就会引发段错误.
   
      段错误对应着`11`号信号 `SIGSEGV`, 下面 , 我们继续对该信号进行重写
   
      ```cpp
      #include<sys/types.h>
      #include<signal.h>
      #include<unistd.h>
      #include<iostream>
      
      using namespace std;
      
      void handler(int event)
      {
          cout<<"接收到一个信号: "<<event<<endl;
          sleep(1);
      }
      
      int main()
      {
          signal(SIGSEGV, handler);
          cout<<"pid:"<<getpid()<<endl;
          cout<<"exception before"<<endl;
          sleep(1);
      
          int* p = nullptr;
          *p = 1;
      
          sleep(1);
          cout<<"exception after"<<endl;
      
          return 0;
      }
      ```
   
      ```shell
      [wind@starry-sky generation]$ ./a.out
      pid:30394
      exception before
      接收到一个信号: 11
      接收到一个信号: 11
      接收到一个信号: 11
      接收到一个信号: 11
      接收到一个信号: 11
      接收到一个信号: 11
      接收到一个信号: 11
      接收到一个信号: 11
      ^C
      [wind@starry-sky generation]$
      ```
   
      在上述的两个异常信号捕捉中, 我们的重写函数都没有`_exit()`, 所以进程都没有退出, 但在实际中, 我们如果要对异常信号进行重写, 通常来说都是要带上`_exit()`的, 从上面的两个示例中可以看到, 即使重写函数中没有`_exit()`, 由于程序不会继续往下走, 而是会不停的执行重写行为, 所以大概率最后还是要用`Ctrl C`终止的, 与其手敲键盘, 不如重写函数直接带上`_exit()`, 上面不带`_exit()`的原因是为了方便我们看到重复执行重写逻辑的现象.  另外, 要注意的是, 也不是一定用`_exit()`, 我这里说`_exit()`的原因是因为`8  11`这两个信号的默认行为是`Core`, `Core`的退出机制更类似于`_exit()`, 即不做善后处理的退出, 而对于`Term`来说, 它的退出机制更类似于`exit()`, 即进行善后处理的退出.   
   
      这其实也体现异常的一种设计目的:  不是说, 出异常时你力挽狂澜把错误给解决, 而是做一些自定义应急方案, 比如把关键数据紧急存储一下, 尽可能地把损失降到最低,  另一方面, 再收集一些错误信息, 总结一下经验教训, 防止之后又出这种错误.
   
      刚刚我们主要从现象角度观察了信号与异常的关系, 接下来 我们从理论上面来谈一谈,.
   
      我们知道,  进程异常退出的原因是因为收到了系统发出的信号,  那系统凭什么知道现在应该给进程发信号, 而不是其它时候发信号呢?
   
      我们先以除0为例.  我们知道, 程序是依靠CPU进行运行的, 程序中的各种运算, 本质上是依靠CPU中的各种寄存器进行的, 比如, 有负责进行加法运算的加法寄存器, 有负责浮点数运算的浮点数寄存器,  有些寄存器并不直接参与程序运行, 而是维护程序的运行状态, 它们对程序的运行起到控制作用, 比如程序计数器或者`eip`负责记录下一条指令的位置, 这样就能在CPU重新调度进程时立刻衔接之前的操作, 给人营造一种进程都在同步运行的假象. 再比如,  CPU中的状态寄存器也是一种控制寄存器, 负责描述CPU的工作状态, 在状态寄存器中, 就有一个溢出标志位, 当执行除0操作时, 就会得到一个无穷, 无穷存不下, 所以就会溢出, 这样状态寄存器中的溢出标志位就会被置1.
   
      系统管理着计算机的所有软硬件资源, 当CPU状态寄存器中的溢出标志位被置一后, CPU就会通过诸如硬件中断的方式让系统获知到CPU的状态出现了异常,于是系统就会给进程发送对应信号, 进程就会执行信号的对应行为.
   
      本来, 如果出现这种异常, 进程默认行为应该是退出, 但在上面, 我们的重写逻辑中并没有退出, 不过,由于进程还使用着CPU, 所以无法处理CPU的错误状态, CPU就会一直处于溢出状态, 就会一直给系统发送硬件中断, 系统于是就会一直给进程发信号, 进程就会一直执行重写逻辑, 所以就产生了一种死循环执行重写逻辑的现象.
   
      而当要发生进程切换的时候, 系统就会把CPU上寄存器中的内容, 或者说 进程的上下文信息 拷贝一份留给进程, 这些上下文信息将被用于在进程下次被CPU调度的时候对CPU内部的寄存器进行初始化.
   
      CPU溢出了, 并不是说这个CPU就坏了, 而是说CPU因为其内部的数据而进入了一种非正常的状态, CPU异常, 是因为其中的数据有问题, 而不是说CPU的物理结构损坏了, 所以当新进程用自己的上下文数据对CPU中的寄存器进行初始化后, 寄存器中的数据就又正常了, 它的状态也就自然正常了, 因此, 前一个进程导致CPU出错, 不会导致下一个进程在运行时CPU也出错, 这是硬件层面进程独立性的实现.
   
      而当之前出错进程又被调度时, 由于它的上下文信息就是有错误的, 所以在初始化后, CPU又会进入错误状态, 系统又会收到硬件中断, 于是又向进程发信号, 进程就又进入了死循环.
   
      对于野指针来说, 我们知道, 进程空间到物理空间的映射是借助于页表这个软件结构进行地址转化的, 实际中, 为了提高地址转化效率, 页表由专门的硬件结构来实现, 被称为`MMU`(内存管理单元),被集成在CPU内部, 而不是在内存中专门创建一个页表结构, 当对野指针进行访问时, 就会引发地址转化的失败, 此时`MMU`就会报错, `MMU`自带一个状态寄存器, 因为它太关键了, 所以有自己的独立状态寄存器, 而不用CPU的整体状态寄存器, `MMU`的状态寄存器也会记录`MMU`的错误信息, 它也会给系统发送硬件中断, 所以系统就知晓了错误, 就会给进程发送信号.  `MMU`自己的寄存器也是上下文的一部分, 所以在进程切换时也会被保存.
   
      C++在语言层面也实现了异常机制, 也就是诸如`try catch`的结构, 其底层其实也是我们上述所说的异常, 不过C++的异常也不一定都是因为信号引发的, 它还有一些语言层面上的东西. C++的异常我们不说, 它的设计目的和系统层的异常其实是一样的, 而且由于`try catch`, 会破坏代码形式, 会降低代码可读性, 所以其实没人用, 甚至有些公司都不给用.
   
      异常也不是说只有硬件能产生, 比如管道就是一个纯软件结构, 由系统维护, 它没有自己专门的硬件结构, 当管道读端已经关闭, 而写端还再写, 就会产生`13`号信号`SIGPIPE`, 进程就会做出相应的行为. 不过大多数软件异常都不是通过信号, 而是通过接口返回值来实现的, 比如`malloc`如果失败, 进程并不会收到异常信号, 而是会收到`malloc`的失败返回值, 总之, 看实际情况.
   
      -----------
   
      上面我们所说的信号大多都是异常信号, 所谓异常信号, 就是进程的运行导致软硬件资源错误引发的, 除此之外, 还有条件信号, 最典型的就是闹钟信号.
   
      系统接口`alarm`可以为当前进程设定一个闹钟, 当闹钟响时, 系统就会给进程发送`14`号信号`SIGALRM`, `SIGALRM`的默认行为是`Term`
   
      ![image-20250105135610644](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250105135610878.png)
   
      `alarm`的返回值表示上次闹钟剩下的时间. 比如, 六秒钟前设定了一个闹钟, 那在三秒后又设定闹钟, 返回值就是3,  如果上次没设闹钟, 或者上次设的闹钟已经响了, 那么返回值就是0.
   
      ```cpp
      #include<sys/types.h>
      #include<unistd.h>
      #include<iostream>
      
      using namespace std;
      
      void handler(int event)
      {
          cout << "process get a signal: " << event << endl;
      }
      
      int main()
      {
          // 设定一个五秒后响的闹钟
          alarm(5);
      
          int count = 0;
          while(true)
          {
              count++;
              cout<<count<<"s"<<endl;
              sleep(1);
          }
      
          return 0;
      }
      ```
   
      ```shell
      [wind@starry-sky generation]$ ./a.out
      1s
      2s
      3s
      4s
      5s
      Alarm clock
      [wind@starry-sky generation]$
      ```
   
      条件信号和异常信号是有区别的, 条件信号只在那个条件到的时候才会发, 别的时候都不会发, 比如下面我们对`SIGALRM`信号进行捕捉, 重写函数没有`exit()`
   
      ```cpp
      int main()
      {
          signal(SIGALRM, handler);
          
          // 设定一个五秒后响的闹钟
          alarm(5);
      
          int count = 0;
          while(true)
          {
              count++;
              cout<<count<<"s"<<endl;
              sleep(1);
          }
      
          return 0;
      }
      ```
   
      ```shell
      [wind@starry-sky generation]$ ./a.out
      1s
      2s
      3s
      4s
      5s
      process get a signal: 14
      6s
      7s
      8s
      9s
      10s
      11s
      12s
      13s
      14s
      ^C
      [wind@starry-sky generation]$
      ```
   
      如果你有连续的定时任务, 可以在重写函数里再设一次闹钟, 这样就能每隔特定时间做一些别的事.
   
      下面我们用实际操作, 看一看`alarm`的返回值, 我们先设定一个50秒的闹钟, 然后在进程运行期间, 提前给自己发送一个`SIGALRM`信号, 看看效果.
   
      ```cpp
      #include<sys/types.h>
      #include<unistd.h>
      #include<signal.h>
      #include<iostream>
      
      using namespace std;
      
      void handler(int event)
      {
          cout << "接收到信号: " << event << endl;
          unsigned int n = alarm(5);
          cout << "剩余时间  : "<< n <<"s"<<endl;
      }
      
      int main()
      {
          signal(SIGALRM, handler);
      
          // 设定一个五秒后响的闹钟
          alarm(50);
      
          int count = 0;
          while(true)
          {
              cout<<count<<"s"<<endl;
              if(count == 7) raise(SIGALRM);
              count++;
              sleep(1);
          }
      
          return 0;
      }
      ```
   
      ```shell
      [wind@starry-sky generation]$ ./a.out
      0s
      1s
      2s
      3s
      4s
      5s
      6s
      7s
      接收到信号: 14
      剩余时间  : 43s
      8s
      9s
      10s
      11s
      接收到信号: 14
      剩余时间  : 0s
      12s
      13s
      14s
      15s
      16s
      接收到信号: 14
      剩余时间  : 0s
      17s
      18s
      ^C
      [wind@starry-sky generation]$
      ```
   
      闹钟, 其实就是时间戳,, 计算机的时间是根据时间戳来走的, 当调用`alarm`时, 系统会依据当前时间戳和输入的秒数算出闹钟响的那个时间戳, 同一个进程可能有多个闹钟, 随之而来就会有多个未来时间戳,  系统可能会把时间戳以小根堆, 或者说优先级队列的形式进行组织, 系统只管理堆顶的未来时间戳, 当堆顶时间戳到达后, 系统就会给进程发送信号, 并把堆顶元素移除, 如果闹钟提前响, 那就可以算一下剩余时间, 用于返回.
   
      ------------------------------
   
      现在我们回到这张表
   
      | Signal  | Value    | Action | Comment                                                      |
      | ------- | -------- | ------ | ------------------------------------------------------------ |
      | SIGHUP  | 1        | Term   | Hangup detected on controlling terminal or death of controlling process |
      | SIGINT  | 2        | Term   | Interrupt from keyboard                                      |
      | SIGQUIT | 3        | Core   | Quit from keyboard                                           |
      | SIGILL  | 4        | Core   | Illegal Instruction                                          |
      | SIGABRT | 6        | Core   | Abort signal from abort(3)                                   |
      | SIGFPE  | 8        | Core   | Floating point exception                                     |
      | SIGKILL | 9        | Term   | Kill signal                                                  |
      | SIGSEGV | 11       | Core   | Invalid memory reference                                     |
      | SIGPIPE | 13       | Term   | Broken pipe: write to pipe with no readers                   |
      | SIGALRM | 14       | Term   | Timer signal from alarm(2)                                   |
      | SIGTERM | 15       | Term   | Termination signal                                           |
      | SIGUSR1 | 30,10,16 | Term   | User-defined signal 1                                        |
      | SIGUSR2 | 31,12,17 | Term   | User-defined signal 2                                        |
      | SIGCHLD | 20,17,18 | Ign    | Child stopped or terminated                                  |
      | SIGCONT | 19,18,25 | Cont   | Continue if stopped                                          |
      | SIGSTOP | 17,19,23 | Stop   | Stop process                                                 |
      | SIGTSTP | 18,20,24 | Stop   | Stop typed at terminal                                       |
      | SIGTTIN | 21,21,26 | Stop   | Terminal input for background process                        |
      | SIGTTOU | 22,22,27 | Stop   | Terminal output for background process                       |
   
   今天我们只探讨`Term`和`Core`这两种行为, 其它的只略微说一下, `Ign`是忽略, `Cont`是继续, `Stop`是停止.
   
   之前我们说过, `Trem`和`Core`都是终止, 但有略微不同, `Trem`是有善后处理的终止, 类似于`exit()`, `Core`是无善后处理的终止, 类似于`_exit()`, 
   
   但`exit()`和`_exit()`的区别不仅仅局限在有无善后处理方面, 还包括是否生成核心转储文件, `Core`的核心指的就是核心转储文件, 核心转储机制,就是当进程出错时, 将进程的上下文信息拷贝一份, 以文件的方式存储在进程的工作路径下, 有了核心转储文件, 就可以在下次调试时, 让进程立刻初始化为出错状态, 以便于立刻定位错误. `exit()`不会生成核心转储文件, 而`_exit()`会.
   
   在学习进程结束状态的时候, 我们特意回避了一个名为`core dump `的标志位.
   
   ![image-20250105154848770](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250105154848928.png)
   
   `core dump`标志位就是记录进程终止时是否生成核心转储文件的, 如果生成 就是1, 没生成就是0.
   
   ```cpp
   #include<sys/types.h>
   #include<sys/wait.h>
   #include<unistd.h>
   #include<signal.h>
   #include<iostream>
   #include<string>
   
   using namespace std;
   
   int main(int argc, char* argv[])
   {
       pid_t id = fork();
       if(id == 0)
       {
           // 子进程执行流
           raise(stoi(argv[1] + 1));  // 给自己发送一个信号
           exit(0);
       }
       else
       {
           // 父进程执行流
           int state = 0;
           waitpid(id, &state, 0);
           if(state & 0x7f)
           {
               cout<<"进程异常退出, 错误码为"<<(state & 0x7f)<<endl;
               if((state >> 7) & 1)
               cout<<"生成核心转储文件"<<endl;
               else
               cout<<"未生成核心转储文件"<<endl;
           }
           else
           {
               cout<<"进程正常退出, 退出码为"<<((state >> 8) & 0xff)<< endl;
           }
       }
       return 0;
   }
   ```
   
   ```shell
   [wind@starry-sky generation]$ ./a.out -1
   进程异常退出, 错误码为1
   未生成核心转储文件
   [wind@starry-sky generation]$
   ```
   
   `1`号行为的默认行为是`Term`
   
   ```shell
   [wind@starry-sky generation]$ ./a.out -8
   进程异常退出, 错误码为8
   未生成核心转储文件
   [wind@starry-sky generation]$
   ```
   
   `8`号行为的默认行为是`Core`
   
   为什么`8`号信号没有生成核心转储文件呢? 因为核心转储文件的生成与否还取决于某些系统关键配置, 我们可以使用`ulimit -a`查看这些关键配置
   
   ```shell
   [wind@starry-sky generation]$ ulimit -a
   core file size          (blocks, -c) 0
   data seg size           (kbytes, -d) unlimited
   scheduling priority             (-e) 0
   file size               (blocks, -f) unlimited
   pending signals                 (-i) 6942
   max locked memory       (kbytes, -l) 64
   max memory size         (kbytes, -m) unlimited
   open files                      (-n) 65535
   pipe size            (512 bytes, -p) 8
   POSIX message queues     (bytes, -q) 819200
   real-time priority              (-r) 0
   stack size              (kbytes, -s) 8192
   cpu time               (seconds, -t) unlimited
   max user processes              (-u) 4096
   virtual memory          (kbytes, -v) unlimited
   file locks                      (-x) unlimited
   [wind@starry-sky generation]$
   ```
   
   其中有一项就是`core file size`, 它描述了生成核心转储文件的大小, 如你所见, 这里是0, 0就相当于不生成, 所以在上面的例子中, 无论是`Term`还是`Core`, 都没有生成核心转储文件.
   
   为什么我这里默认是0呢? 因为我用的是云服务器,  对于服务器系统来说, 默认不使用核心存储机制, 也最好不要用, 原因后面会说, 如果你使用的是自己安装系统的虚拟机, 大概率核心转储机制是开启的. 想要开启核心转储机制, 仅需要`ulimit -c num`即可
   
   ```shell
   [wind@starry-sky generation]$ ulimit -c 10240
   [wind@starry-sky generation]$ ulimit -a
   core file size          (blocks, -c) 10240
   data seg size           (kbytes, -d) unlimited
   scheduling priority             (-e) 0
   file size               (blocks, -f) unlimited
   pending signals                 (-i) 6942
   max locked memory       (kbytes, -l) 64
   max memory size         (kbytes, -m) unlimited
   open files                      (-n) 65535
   pipe size            (512 bytes, -p) 8
   POSIX message queues     (bytes, -q) 819200
   real-time priority              (-r) 0
   stack size              (kbytes, -s) 8192
   cpu time               (seconds, -t) unlimited
   max user processes              (-u) 4096
   virtual memory          (kbytes, -v) unlimited
   file locks                      (-x) unlimited
   [wind@starry-sky generation]$
   ```
   
   注意, 后面我们要把它关掉, 有两种关闭方法, 一是重新登录, 刚刚的修改只是内存级的修改, 二是直接改回0.
   
   现在我们重新运行一下
   
   ```shell
   [wind@starry-sky generation]$ ll
   total 60
   -rwxrwxr-x 1 wind wind 16664 Dec 21 10:46 a
   -rw-rw-r-- 1 wind wind  1331 Jan  5 16:51 clock.cc
   -rw-rw-r-- 1 wind wind   423 Jan  5 10:28 exception.cc
   -rw-rw-r-- 1 wind wind  2014 Dec 21 10:04 main.cc
   -rw-rw-r-- 1 wind wind    59 Dec 20 17:02 makefile
   -rwxrwxr-x 1 wind wind 16616 Dec 21 10:05 out
   -rw-rw-r-- 1 wind wind   456 Dec 21 10:46 slay.cc
   [wind@starry-sky generation]$ g++ -g clock.cc
   [wind@starry-sky generation]$ ./a.out -1
   进程异常退出, 错误码为1
   未生成核心转储文件
   [wind@starry-sky generation]$ ./a.out -8
   进程异常退出, 错误码为8
   生成核心转储文件
   [wind@starry-sky generation]$ ll
   total 328
   -rwxrwxr-x 1 wind wind  16664 Dec 21 10:46 a
   -rwxrwxr-x 1 wind wind  33944 Jan  5 17:11 a.out
   -rw-rw-r-- 1 wind wind   1331 Jan  5 16:51 clock.cc
   -rw------- 1 wind wind 557056 Jan  5 17:11 core.19972
   -rw-rw-r-- 1 wind wind    423 Jan  5 10:28 exception.cc
   -rw-rw-r-- 1 wind wind   2014 Dec 21 10:04 main.cc
   -rw-rw-r-- 1 wind wind     59 Dec 20 17:02 makefile
   -rwxrwxr-x 1 wind wind  16616 Dec 21 10:05 out
   -rw-rw-r-- 1 wind wind    456 Dec 21 10:46 slay.cc
   [wind@starry-sky generation]$
   ```
   
   我们发现, 进程的工作路径下生成了`core.19972`文件, 那后面的数字后缀其实就是异常退出进程的`pid`, 这核心转储文件怎么用呢? 调试的时候,直接输进去就行.
   
   为了让现象更加明显, 这里修改一下代码
   
   ```cpp
   #include<sys/types.h>
   #include<sys/wait.h>
   #include<unistd.h>
   #include<signal.h>
   #include<iostream>
   #include<string>
   
   using namespace std;
   
   int main(int argc, char* argv[])
   {
       pid_t id = fork();
       if(id == 0)
       {
           cout<<"child-pid: "<<getpid()<<endl;
           // 子进程执行流
           // raise(stoi(argv[1] + 1));  // 给自己发送一个信号
           int a = 1 / 0;
           exit(0);
       }
       else
       {
           // 父进程执行流
           int state = 0;
           waitpid(id, &state, 0);
           if(state & 0x7f)
           {
               cout<<"进程异常退出, 错误码为"<<(state & 0x7f)<<endl;
               if((state >> 7) & 1)
               cout<<"生成核心转储文件"<<endl;
               else
               cout<<"未生成核心转储文件"<<endl;
           }
           else
           {
               cout<<"进程正常退出, 退出码为"<<((state >> 8) & 0xff)<< endl;
           }
       }
       return 0;
   }
   ```
   
   ```shell
   [wind@starry-sky generation]$ rm a.out core.*
   [wind@starry-sky generation]$ ll
   total 60
   -rwxrwxr-x 1 wind wind 16664 Dec 21 10:46 a
   -rw-rw-r-- 1 wind wind  1402 Jan  5 17:28 clock.cc
   -rw-rw-r-- 1 wind wind   423 Jan  5 10:28 exception.cc
   -rw-rw-r-- 1 wind wind  2014 Dec 21 10:04 main.cc
   -rw-rw-r-- 1 wind wind    59 Dec 20 17:02 makefile
   -rwxrwxr-x 1 wind wind 16616 Dec 21 10:05 out
   -rw-rw-r-- 1 wind wind   456 Dec 21 10:46 slay.cc
   [wind@starry-sky generation]$ g++ -g clock.cc
   clock.cc: In function ‘int main(int, char**)’:
   clock.cc:18:19: warning: division by zero [-Wdiv-by-zero]
      18 |         int a = 1 / 0;
         |                 ~~^~~
   [wind@starry-sky generation]$ ./a.out
   child-pid: 20925
   进程异常退出, 错误码为8
   生成核心转储文件
   [wind@starry-sky generation]$ ll
   total 332
   -rwxrwxr-x 1 wind wind  16664 Dec 21 10:46 a
   -rwxrwxr-x 1 wind wind  33968 Jan  5 17:29 a.out
   -rw-rw-r-- 1 wind wind   1402 Jan  5 17:28 clock.cc
   -rw------- 1 wind wind 561152 Jan  5 17:29 core.20925
   -rw-rw-r-- 1 wind wind    423 Jan  5 10:28 exception.cc
   -rw-rw-r-- 1 wind wind   2014 Dec 21 10:04 main.cc
   -rw-rw-r-- 1 wind wind     59 Dec 20 17:02 makefile
   -rwxrwxr-x 1 wind wind  16616 Dec 21 10:05 out
   -rw-rw-r-- 1 wind wind    456 Dec 21 10:46 slay.cc
   [wind@starry-sky generation]$ gdb a.out
   GNU gdb (GDB) Red Hat Enterprise Linux 7.6.1-120.el7
   Copyright (C) 2013 Free Software Foundation, Inc.
   License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
   This is free software: you are free to change and redistribute it.
   There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
   and "show warranty" for details.
   This GDB was configured as "x86_64-redhat-linux-gnu".
   For bug reporting instructions, please see:
   <http://www.gnu.org/software/gdb/bugs/>...
   Reading symbols from /home/wind/projects/Signal/Concept/generation/a.out...done.
   (gdb) core-file core.20925
   [New LWP 20925]
   Core was generated by `./a.out'.
   Program terminated with signal 8, Arithmetic exception.
   #0  0x000000000040124d in main (argc=1, argv=0x7ffc79c6eaf8) at clock.cc:18
   18	        int a = 1 / 0;
   Missing separate debuginfos, use: debuginfo-install glibc-2.17-326.el7_9.3.x86_64 libgcc-4.8.5-44.el7.x86_64
   (gdb) 
   ```
   
   这样就能快速定位错误位置和原因. 不失为一种实在找不到bug的最后手段.我们把这种调试称之为"事后调试", 而我们平常的错误称之为"事前调试".
   
   听起来核心转储挺好的, 为什么服务器默认不用呢? 因为服务器上跑的是服务, 服务崩了之后, 第一时间不是找bug, 而是赶紧恢复服务, 服务中断的每一秒,都会有真金白金的损失, 怎么恢复服务呢? 一般情况下, 都是重新运行程序, 程序重新运行会有状态初始化, 只要不重新触发之前的问题, 基本上还可以勉强跑.事后再通过日志去分析问题发生的原因, 公司的服务器上会有很多程序, 程序的崩溃也可能发生在深更半夜, 总之, 不能真的手动重启崩溃程序, 所以就需要自动化运维, 我们运维部门的同事会构建一个自动化运维框架, 时刻监视服务程序的运行状态, 当程序崩溃之后, 会自动对其进行重启.  
   
   于是就可能发生这种场景: 那天晚上特别倒霉, 也不知怎么回事, 服务总是崩溃, 然后自动化运维程序又把服务程序又重启, 然后服务程序又崩溃了, 然后又重启, 如果此时启用了核心转储机制, 一方面,能正儿八经跑服务的程序一般是比较大的, 所以核心转储文件就大, 另一方面, 崩溃的次数很多, 生成的核心转储文件也就很多, 就会占用磁盘空间, 磁盘空间如果不够, 影响的是服务器系统, 服务器系统要是崩溃了, 上面运行的程序不管正不正常, 都会下线, 就会造成更大的损失, 所以服务器系统里不开启核心转储功能.
   
   另外说一下, 服务程序都是`Release`版, 不是`Debug`版, 但`Release`版生成的核心转储文件是可以直接用于`Debug`版的调试的. 关于核心转储, 我们就不深入探讨了.

### send and save

上面我们所说的内容都是信号的产生, 现在我们开始学习信号的传送和保存.

首先, 我们要知道信号, 这里指前31个普通信号, 不包括实时信号, 在进程中究竟是以什么形式存在的. 实际上, 它们是以位图形式存在的, 在进程控制块中, 有一个整型变量, 从最低位的0号位开始, 到最高位的31号位结束, 一共有32个比特位, 这32个比特位就可以表示信号的有无, 系统对进程发信号的实质, 就是把进程控制块中该整型变量的对应比特位置1, 所以与其说系统发送信号, 不如说, 系统写入信号, 以后要是客户想要了解信号, 你就这样对他说, 这种解释更加具象化.

当然, 位图这种形式也是有缺点的, 那就是它对信号的连续发送不太支持, 同一个信号发送五次, 和这个信号只发送一次, 在进程看来没什么区别, 毕竟二进制只有1和0这两种状态, 存不下其它信息了. 不过这种场景问题不大, 最后行为函数还是会被执行的.

至于其它的那些实时信号, 是用其它数据结构进行存储的, 考虑到它的实时性, 很可能会使用队列的形式, 因为队列先进先出吗, 不过呢, 我们还是不考虑是实时信号, 毕竟我们主要还是做后端的, 后端的服务主打均衡,  让大家都能得到调度,  但如果你做比如车载系统, 股票交易这种必须立刻响应, 那当我没说. 

下面我们对信号的一些关键概念进行一下了解, 以帮助我们减轻后续的叙述负担.   我们把进程执行信号对应行为的这种现象称之为"信号抵达";  信号从产生到抵达的中间过程我们称之为"信号未决";   此外, 还有名为"信号阻塞"的概念,  这个"阻塞"可不是进程的那种阻塞,  它的意思是, 不管信号来不来, 我都做我的事.  如果信号被我接收到了, 那就只是在位图里保存一下它, 然后继续干自己的事, 而当信号被取消阻塞后, 如果位图被记录了, 再执行对应的行为, 如果没有被记录, 还是继续干自己的事. 有人把"阻塞"也叫做"屏蔽".

这些概念都需要有对应的软件结构作支撑, 对于普通信号来说, 有三张非常重要的表, 其中两个是位图表, 剩下那个是方法表.

![image-20250111133409093](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250111133409186.png)

我们先看所谓的方法表, 方法表其实就是函数指针数组, 它们指向着对应信号的执行逻辑. 比如就上图来说, `SIG_DFL`是个宏, 这个宏实际上是函数指针, 或者说, 函数名, 系统有自己的类似于我们上面进行信号捕捉时的`handler`, 它内部也是以分支结构的形式写出了一个个信号的默认行为,    对于`SIG_IGN`来说, 它表示信号忽略的意思, 可以说就是指向了一个空函数, 这个函数空有接口, 但里面没代码, (实际不是真的空, 忽略或者说空是从信号行为本身的角度来说的, 但实际上这个函数里面还包括信号控制的相应代码, 信号默认都是要处理的, 其中包括一系列的内核层操作, 后面我们会细说, 忽略连这些内核层操作都不执行了, 默认是执行, 它凭什么不执行, 因为里面代码特别写了就是不要执行这些默认的内核操作, 所以不会执行, 但仅从用户层来说, 可以简单认为是空的)  所以就有忽略的意思, 信号发生时什么都不做, 除此之外, 用户还可以通过`signal`接口对信号的默认行为进行覆写, 其实就是把对应信号的方法换成用户自己写的, 这样就达到了行为覆写的这个效果.     另外, 需要说的是, 

 `pending`其实就是负责记录信号有无的比特位图, `1`表示有, `0`表示无.  `block`也是位图, 它是用来实现信号的阻塞功能的, 其中的比特位置1, 就意味着对相应信号进行了阻塞, 默认情况下, `block`全是`0`. 有人把`block`称之为"信号屏蔽字"或者"信号阻塞集" .   注意, 阻塞是一种状态, 它无关乎信号是否有无, 而忽略是一种行为, 它建立在信号有的前提上, 尽管效果类似, 看起来都是干自己事, 但细节上有一定区别, 进程收到信号时, 如果是阻塞, 那`pending`是会作相应修改的, 而忽略就是一种处理行为, 进程收到被忽略的信号后, 都不会修改`pending`, 为什么不会修改呢? 因为`SIG_IGN`那个函数就已经设置过了, 为什么别的信号行为是处理时再执行, `SIG_IGN`一开始就好像设置好了, 就像执行过了一样, 那我只能推测是被特殊处理了. 比如在方法表被初始化或修改之后, 如果发现内容是`SIG_IGN`, 那就立刻执行.

方法表如何进行修改我们之前已经用`signal`演示过了, `pending`是用来直接记录信号有无的, 太关键了, 系统不让我们改, 所以接下来我们就主要学习一下修改`block`的相应系统接口.

`block`是个位图结构, 位图的实现方案多种多样, 随之而来就有多种多样的位图操作方法, 为了统一操作, 系统自己内嵌了一个专门用于`block`操作的位图类型,  也就是`sigset_t  `, 用户要通过一些接口来对`sigset_t  `变量进行更改, 不能直接位操作, 一般将`sigset_t  `称之为信号集类型.

接下来我们来看看相应的接口

```cpp
#include <signal.h>
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset (sigset_t *set, int signo);
int sigdelset(sigset_t *set, int signo);
int sigismember（const sigset_t *set, int signo);
```

 `sigemptyset`就是把输入位图结构的比特位全部置为0, `sigfillset`就是把比特位全部置`1`, `sigaddset`就是把第`signo`的比特位置`1`,  `sigdelset`就是把第`signo`比特位置`0`,  `sigismember`查看`signo`是`1`还是`0`.   需要说明的是,  `sigset_t`类型是系统与用户传递位图信息的一种渠道, 并不是说系统真的用这个类型.

接下来我们说说`sigprocmask  `, 它是专门用于修改`block`的接口

```cpp
#include <signal.h>
int sigprocmask(int how, const sigset_t *set, sigset_t *oset);
```

第一个参数`how`, 表示要进行的具体操作, `set`是向内核输入的位图信息, `oset`是内核输出的修改前的位图信息, 如果不想要, 置为空指针即可.  `sig`无疑是信号的缩写, `proc`则是进程的缩写, 至于`mask`, 表示掩码,  `block`是对应比特位置`1`, 就对该信号进行阻塞, 有掩码的那种感觉.

`how`具体有如下操作   为方便描述, 我们把`sigset_t`中含有的位图信息都用`int`的形式来表现, 并把系统真正使用的位图信息称之为`mask`, 把用户输入的位图信息称之为`set`, 于是就有

| how         | action                                            |
| ----------- | ------------------------------------------------- |
| SIG_BLOCK   | 用于增加新的阻塞信号, 相当于 mask = mask \| set   |
| SIG_UNBLOCK | 用于清除旧的阻塞信号, 相当于 mask = mask & (~set) |
| SIG_SETMASK | 把mask变成set,              相当于 mask = set     |

除此之外, 还有`sigpending`接口, 用于将内核里面的`pending`, 也就是记录信号有无的那个位图结构信息以`sigset_t`的形式输出出来

```cpp
#include <signal.h>
int sigpending(sigset_t *set);
```

下面我们敲代码看看现象

```cpp
#include<signal.h>
#include<unistd.h>
#include<iostream>

using namespace std;

// 将sigset_t类型变量以二进制序列的形式打印出来
void print(const sigset_t& information)
{
    for(int i = 31; i >= 0; i--)
    {
        if(sigismember(&information, i) == 1)
            cout << 1;
        else
            cout << 0;
    }
    cout<<endl;
}

void handler(int event)
{
    cout<<"================================"<<endl;
    sigset_t pending;
    sigpending(&pending);
    print(pending);
    exit(0);
}

int main(int argc, char* argv[])
{
    // 解析要处理的信号
    int signaln = stoi(argv[1] + 1);

    // 定义并初始化用户层的sigset_t变量, 作为待输入位图信息的载体
    sigset_t set, oset;
    sigemptyset(&set);

    // 将位图信息写入载体
    sigaddset(&set, signaln);

    // 将位图信息由用户层传送入内核层
    sigprocmask(SIG_SETMASK, &set, &oset);   // 屏蔽信号

    // 轮询pending, 间隔1秒, 进行10次
    for(size_t i = 0; i < 10; i++)
    {
        sigset_t pending;
        sigpending(&pending);
        print(pending);

        // 让第五秒处于信号被记录状态
        if(i == 3)
            raise(signaln);

        sleep(1);
    }

    // 覆写信号的默认行为
    signal(signaln, handler);

    // 恢复原有的block
    sigprocmask(SIG_SETMASK, &oset, nullptr);

    // 信号处理

    return 0;
}
```

```shell
[wind@starry-sky send and save]$ ./out -2
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
================================
00000000000000000000000000000000
[wind@starry-sky send and save]$
```

从现象中可以看到, `2`号信号最开始确实是被屏蔽了, `pending`记录了`2`曾经来过的信息, 在`block`恢复之后, 信号也确实被处理了.    另外, 我们得到的额外信息是, `pending`比特位的重新置`0`(如果置0有意义的话), 发生在信号行为执行完前, 实际上, 信号行为执行前会有一些内核级操作, 这些操作中就包含将`pending`对应位置为`0`, 也存在一些信号, 进程接收到信号意味着错误, 所以就不会被重新置为`0`, 就会一直保持在置`1`的状态, 这就是所谓的置`0`没有意义.

需要说明的是, 有些信号是无法被屏蔽的, 比如`9`号信号`SIGKILL`和`19`号信号`SIGSTOP`, 原因和它们无法被覆写相同. 在此不作赘述.

如果你尝试了`19`号信号, 可以用下面的方式重新恢复运行

```shell
[wind@starry-sky send and save]$ ./out -19
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000

[1]+  Stopped                 ./out -19
[wind@starry-sky send and save]$ fg %1
./out -19
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
[wind@starry-sky send and save]$
```

`Stopped`前面的序号是几, 你就`fg %`几.

### handling

在说信号的检测与处理之前，我们需要对系统有更深入的了解。为此，我们需要先再谈谈进程地址空间。

![image-20241214160114166](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214160114236.png)

---

我们知道，一个应用层的进程要为用户提供某种服务，需要两方面的配合。一方面，这个进程中需要含有执行该服务的代码；另一方面，系统需要让这个进程能够运行。具体来说，系统让进程能够运行包括两个大的内容：一是管理好计算机中的各类资源，二是为进程提供使用这些资源的方法。

进程本身也是一种计算机软件资源，因此，系统需要对进程进行合理的管理，确保它们能够均衡地得到运行。而使用资源的方法，则是系统通过软件接口的形式呈现给进程的。

系统本质上也是一个进程。无论是资源管理，还是接口背后的函数实现，都需要依赖CPU的运行。因此，对于应用层的进程来说，它的地址空间由两部分组成：
- 一部分是“用户空间”，存放服务实现的具体逻辑和所需要的数据；
- 另一部分是“内核空间”，存放系统运行所需要的代码和数据。

系统作为非常特殊的进程，必须受到特别的保护。为了确保用户空间的代码无法访问系统的数据，用户空间和内核空间需要有各自独立的页表，进程和系统之间各用各的页表，彼此保持相对独立。为了区分这两种运行状态，我们称CPU运行用户空间代码的状态为“用户态”，运行内核空间代码的状态为“内核态”。

---

CPU在本质上运行的还是系统，而应用层的进程则是在系统完成自己工作的空隙中运行的。为什么说本质上运行的是系统呢？因为进程的运行是通过硬件中断来完成的。在计算机内部，有一个硬件负责定时向系统发送电信号，当系统收到这个信号后，就以中断的形式执行进程的代码。

系统在刚启动时，会完成一系列初始化操作，为进程的运行搭建好环境。等环境搭好了，系统就会进入一个死循环。在这个死循环中，系统通过响应各种硬件中断触发不同的中断方法。例如，当定时器硬件发出信号后，系统会运行进程管理方法：
1. 检查当前运行的进程是否需要进入阻塞状态；
2. 如果需要阻塞，则从运行队列中选择下一个进程调度；
3. CPU从内核态切换到用户态，运行被调度的进程代码。

当进程调用系统接口时，CPU再次切换回内核态，系统会将进程输入的参数作为依据，运行内部的具体方法。

---

CPU的运行状态由内部寄存器控制，比如名为“`ECS`”的段寄存器。其中，`ECS`的低两位用于描述CPU的当前状态：
- **00**：内核态，使用内核页表；
- **11**：用户态，使用用户页表。

在用户态时，CPU只能访问用户页表中的地址范围，因此无法直接访问内核空间的数据。

-----------------------

信号的检测与处理, 就发生在CPU由内核态切换到用户态之前,    此时CPU上运行的是系统, 系统会看即将运行进程信号的两个位图表, 如果有信号需要被处理, 那就依据具体信号先对`pending`进行修改, 然后去方法表中执行信号对应的行为, 如果对应的方法是`SIG_DFL`, 因为`SIG_DFL`就是系统自带的, 所以就直接以"内核态"的方式去执行, 如果对应的方法是用户自己写的, 就先变为"用户态", 以"用户态"的身份去执行其中的代码, 执行完后, 再返回"内核态", 因为"内核态"是系统, 进程的程序计数器记录了进程下一条指令的位置, 而程序计数器是系统的数据, 所以需要先返回"内核态", 这样才知道到底应该返回到进程的什么位置, 才能变为"用户态".  在压栈过程中, 用户自己写的信号处理方法会被自动拼接上返回内核态的代码, 所以它就返回内核态了.

![image-20250112121402753](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250112121403043.png)

我们可以用下面这张图来记忆:

![image-20250112125855745](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250112125855809.png)

图中贯穿无穷符号的横线被视为用户态和内核态的分界, 当内核态变为用户态之前, 就会进行信号检测.

由用户态转化为内核态有两个方法, 一是进程使用了系统接口, 进程若使用系统接口, 会自动进入内核态, 二是, 进程时间片用完之后, 重新再一次被CPU调度, 此时相当于是上一个进程用户态    内核态(上一个进程时间片用完, 准备调度下一个进程)   下一个进程用户态,   所以总会存在非常多的情况信号检测场景, 因此不必担心没机会检测.

### sigaction  

除了`signal`可以进行信号捕捉之外, `sigaction`也可以用于进行信号捕捉, 先说`signal`的原因是因为它的接口更加直观, 理解起来也比较简单, `sigaction`理解起来倒也不难, 但需要一些基础知识, 或者它理解起来有门槛, 所以不能一开始就直接使用`sigaction`

```cpp
#include <signal.h>
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```

`sigaction`的接口形式与`sigprocmask`类似, 第一个参数`signum`是被捕捉信号的编号, `struct sigaction`中则存储着信号两个核心表: 掩码表和方法表的信息, 第二个参数是输入型参数, 负责将用户设置的信号配置传递给内核, 第三个参数是输出型参数, 负责把内核原来的信号配置输出给用户使用.

```cpp
struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)(void);
};
```

我们只需要看`struct sigaction`中的两个成员, 一是`sa_handler`, 就是用户对信号自定义行为的函数, 二是`sa_mask`, 描述了信号自定义行为执行期间要被屏蔽的其它信号, 我们先不看`sa_mask`.

下面, 我们就拿之前的代码略微改一下

```cpp
#include<signal.h>
#include<unistd.h>
#include<iostream>

using namespace std;

// 将sigset_t类型变量以二进制序列的形式打印出来
void print(const sigset_t& information)
{
    for(int i = 31; i >= 0; i--)
    {
        if(sigismember(&information, i) == 1)
            cout << 1;
        else
            cout << 0;
    }
    cout<<endl;
}

void handler(int event)
{
    cout<<"================================"<<endl;
    sigset_t pending;
    sigpending(&pending);
    print(pending);
    exit(0);
}

void SetToNull(struct sigaction& act)
{
    sigset_t set;
    sigemptyset(&set);
    act.sa_handler = nullptr;
    act.sa_sigaction = nullptr;
    act.sa_mask = set;
    act.sa_flags = 0;
    act.sa_restorer = nullptr;
}

int main(int argc, char* argv[])
{
    // 解析要处理的信号
    int signaln = stoi(argv[1] + 1);

    // 定义并初始化用户层的sigset_t变量, 作为待输入位图信息的载体
    sigset_t set, oset;
    sigemptyset(&set);

    // 将位图信息写入载体
    sigaddset(&set, signaln);

    // 将位图信息由用户层传送入内核层
    sigprocmask(SIG_SETMASK, &set, &oset);   // 屏蔽信号

    // 轮询pending, 间隔1秒, 进行10次
    for(size_t i = 0; i < 10; i++)
    {
        sigset_t pending;
        sigpending(&pending);
        print(pending);

        // 让第五秒处于信号被记录状态
        if(i == 3)
            raise(signaln);

        sleep(1);
    }

    // 覆写信号的默认行为
    struct sigaction act;
    SetToNull(act);
    act.sa_handler = handler;
    sigaction(signaln, &act, nullptr);

    // 恢复原有的block
    sigprocmask(SIG_SETMASK, &oset, nullptr);

    // 信号处理

    return 0;
}
```

```shell
[wind@starry-sky sigaction]$ ./w -2
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
================================
00000000000000000000000000000000
[wind@starry-sky sigaction]$
```

我们知道, 当被捕捉的信号执行用户的自定义函数时, 是处于用户态的, 此时, 就有可能出现这种情况: 进程正在执行处理函数时, 又捕捉到了另一个信号, 于是就会再次进入用户的自定义函数方法集中, 也就是说, 信号的行为函数还没有执行完, 就又重新再一次执行了信号处理函数, 就拿上面的函数来说

```cpp
void handler(int event)
{
    cout<<"================================"<<endl;
    sigset_t pending;
    sigpending(&pending);
    print(pending);
    exit(0);
}
```

我们假想一下, 假如`handler`里又两个分支, 分别对应一号和二号信号的自定义行为, 在进程的一开始, 我们就对一号和二号行为进行了捕捉, 进程首先收到了一号信号, 于是进入一号信号的对应分支, 在执行一号信号的分支过程中, 进程又收到了二号信号, 于是进程放下还没执行完的一号信号分支, 又去执行二号信号分支, 二号信号的分支执行完后, 它又回到之前的一号信号分支, 把剩下的执行完.

或者我们只说一个信号, 进程一开始只对一号信号进行了捕捉, 再收到一号信号之后, 进程就开始执行`handler`, 执行到第五行的时候, 进程又收到了一号信号, 于是它再次进入`handler`, 从第三行开始执行, 执行完后, (假设没有退出), 又回到了之前的`handler`, 从第六行接着执行.

对于上面的`handler`来说, 若是出现了这种情况, 其实问题还不大, 毕竟它只是打印而已, 信息本质上是单向的, 最多输出的时候有些许错误, 但对于原子性行为来说, 就不行了, 我们之前在进程通信中说过原子性这个概念, 原子, 在古希腊中的原意是指不可分割的, 后来才有了物理学或者化学里面的那种基本粒子的意思, 一个行为是原子性的, 意思就是说, 这个行为要么不做, 要做就要一次性做完, 不存在中间状态,  

我们之前不怎么谈原子性这个概念, 是因为我们之前的都是单控制流, 但信号捕捉这里, 进程的主控制流和信号控制流是相互独立的, 于是就会牵扯到很多并发问题.  比如, 我们就拿链表节点的插入为例.

![image-20250123174417147](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250123174417357.png)

首先我们生成了一个节点, 记为`node1`, 之后, `node1`的指针指向了原先的第一个结点, 此时, 由于进程捕捉到了一个信号, 这个信号也是在该链表中头插一个节点, 于是, 就生成了`node2`, `node2`指向原先的第一个节点, 接着, `node2`变为了第一个节点, 之后回退到之前的控制流上, `node1`就变成了第一个节点, 这就造成了`node2`的丢失, 引发了内存泄漏问题.

为了降低上述问题出现的可能, 系统会在执行信号自定义行为函数之前, 将其阻塞, 而在信号处理完之后, 再将其恢复,  下面, 我们就写份代码实验一下.

```cpp
#include<signal.h>
#include<unistd.h>
#include<iostream>

using namespace std;

// 将sigset_t类型变量以二进制序列的形式打印出来
void print(const sigset_t& information)
{
    for(int i = 31; i >= 0; i--)
    {
        if(sigismember(&information, i) == 1)
            cout << 1;
        else
            cout << 0;
    }
    cout<<endl;
}

void handler(int event)
{
    cout << endl;
    static bool flag = true;
    cout <<"{"<<endl;
    sigset_t pending, block;
    sigpending(&pending);                           // 获取pending表
    sigprocmask(SIG_SETMASK, nullptr, &block);      // 获取block表   第二个参数为空时, 第一个参数无意义

    if(flag)
    {
        raise(event);                               // 再次向自己发送相同信号
        flag = false;
    }

    cout <<"    pending: ";
    print(pending);
    cout <<"    block  : ";
    print(block);
    cout<<"}"<<endl;
}

void SetToNull(struct sigaction& act)
{
    sigset_t set;
    sigemptyset(&set);
    act.sa_handler = nullptr;
    act.sa_sigaction = nullptr;
    act.sa_mask = set;
    act.sa_flags = 0;
    act.sa_restorer = nullptr;
}

int main(int argc, char* argv[])
{
    // 解析要处理的信号
    int signaln = stoi(argv[1] + 1);

    // 定义并初始化用户层的sigset_t变量, 作为待输入位图信息的载体
    sigset_t set, oset;
    sigemptyset(&set);

    // 将位图信息写入载体
    sigaddset(&set, signaln);

    // 将位图信息由用户层传送入内核层
    sigprocmask(SIG_SETMASK, &set, &oset);   // 屏蔽信号

    // 轮询pending, 间隔1秒, 进行10次
    for(size_t i = 0; i < 10; i++)
    {
        sigset_t pending;
        sigpending(&pending);
        print(pending);

        // 让第五秒处于信号被记录状态
        if(i == 3)
            raise(signaln);

        sleep(1);
    }

    // 覆写信号的默认行为
    struct sigaction act;
    SetToNull(act);
    act.sa_handler = handler;
    sigaction(signaln, &act, nullptr);

    // 恢复原有的block
    sigprocmask(SIG_SETMASK, &oset, nullptr);

    // 信号处理

    return 0;
}
```

```shell
[wind@starry-sky sigaction]$ make clean
[wind@starry-sky sigaction]$ ls
main.cc  makefile
[wind@starry-sky sigaction]$ make
[wind@starry-sky sigaction]$ ./w -2
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
00000000000000000000000000000100
{
    pending: 00000000000000000000000000000000
    block  : 00000000000000000000000000000100
}

{
    pending: 00000000000000000000000000000000
    block  : 00000000000000000000000000000100
}

[wind@starry-sky sigaction]$
```

我们对`handler`进行了略微修改, 使之可以打印当前的`pending`和`block`, 并对函数的开始和结束进行了视觉化突出, 从结果上我们可以看到, 进程确实是先把前一个二号信号的行为执行完后, 再去执行后一个二号信号的行为函数.

现在让我们再次假想一下, 假设现在我们对一号信号和二号信号都进行了捕捉, 这两个信号的行为函数是某个类里的静态函数, 这个静态函数对类中的静态链表成员进行数据的插入, 一号信号插入这类数据, 二号信号插入另一类数据, 这意味着, 在其中一个信号行为被执行的过程中, 即使进程收到了另一个信号, 它也必须先把上一个信号的行为函数执行完之后在再去处理另一个信号, 此时, 光是阻塞一个信号就不行了, 必须对它们都进行阻塞.

我们拿下面这份代码说明

```cpp
#include<signal.h>
#include<unistd.h>
#include<iostream>

using namespace std;

// 将sigset_t类型变量以二进制序列的形式打印出来
void print(const sigset_t& information)
{
    for(int i = 31; i >= 0; i--)
    {
        if(sigismember(&information, i) == 1)
            cout << 1;
        else
            cout << 0;
    }
    cout<<endl;
}

void handler(int event)
{
    static bool flag = true;
    cout <<"{"<<endl;
    sigset_t pending, block;
    sigpending(&pending);                           // 获取pending表
    sigprocmask(SIG_SETMASK, nullptr, &block);      // 获取block表   第二个参数为空时, 第一个参数无意义

    if(flag)
    {
        raise(2);                           // 向自己发送二号信号
        flag = false;
    }

    cout <<"    pending: ";
    print(pending);
    cout <<"    block  : ";
    print(block);
    cout<<"}"<<endl;
    cout << endl;
}

void SetToNull(struct sigaction& act)
{
    sigset_t set;
    sigemptyset(&set);
    act.sa_handler = nullptr;
    act.sa_sigaction = nullptr;
    act.sa_mask = set;
    act.sa_flags = 0;
    act.sa_restorer = nullptr;
}

int main()
{
    // 对一号和二号信号进行阻塞
    sigset_t set, oset;
    sigemptyset(&set);

    sigaddset(&set, 1);
    sigaddset(&set, 2);

    sigprocmask(SIG_SETMASK, &set, &oset);

    // 对自己发送一号信号
    raise(1);

    // 对一号信号和二号信号进行捕捉
    struct sigaction act;
    SetToNull(act);
    act.sa_handler = handler;
    sigaction(1, &act, nullptr);
    sigaction(2, &act, nullptr);

    // 对一号信号和二号信号取消阻塞
    sigprocmask(SIG_SETMASK, &oset, nullptr);

    // 信号处理

    return 0;
}
```

```shell
[wind@starry-sky sigaction]$ make clean ; make; ./w
{
{
    pending: 00000000000000000000000000000000
    block  : 00000000000000000000000000000110
}

{
    pending: 00000000000000000000000000000000
    block  : 00000000000000000000000000000110
}

    pending: 00000000000000000000000000000000
    block  : 00000000000000000000000000000010
}

[wind@starry-sky sigaction]$
```

首先, 我们因为收到一号信号而进入`handler`, 于是打印出了第一个`{`, 并对一号信号进行了阻塞, 然后, 在条件语句中, 第`30`行导致进程收到二号信号, 于是再一次进入`handler`, 打印出了一个`{`, 由于上次行为函数中第`31`行并未执行, 所以`flag`仍是真, 因此再次收到二号信号, 但由于二号信号已经被阻塞, 所以没有立刻执行, 而是等到前一个二号信号执行之后, 才执行后一个二号信号, 后一个二号信号执行完之后, 才回到最开始的一号信号, 

为了避免这种情况的发生, 我们在进行信号捕捉的时候, 我们可以对`struct sigaction`中的`sa_mask`进行配置, 从而告诉系统, 在执行这个信号的行为函数之前, 不光要把这个信号本身给阻塞, 如果`sa_mask`中还指示了其它信号, 那也一并阻塞, 从而避免相互干扰.

```cpp
int main()
{
    // 对一号和二号信号进行阻塞
    sigset_t set, oset;
    sigemptyset(&set);

    sigaddset(&set, 1);
    sigaddset(&set, 2);

    sigprocmask(SIG_SETMASK, &set, &oset);

    // 对自己发送一号信号
    raise(1);

    // 对一号信号和二号信号进行捕捉
    struct sigaction act;
    SetToNull(act);
    act.sa_handler = handler;
    act.sa_mask = set;
    sigaction(1, &act, nullptr);
    sigaction(2, &act, nullptr);

    // 对一号信号和二号信号取消阻塞
    sigprocmask(SIG_SETMASK, &oset, nullptr);

    // 信号处理

    return 0;
}
```

```shell
[wind@starry-sky sigaction]$ make clean ; make; ./w
{
    pending: 00000000000000000000000000000000
    block  : 00000000000000000000000000000110
}

{
    pending: 00000000000000000000000000000000
    block  : 00000000000000000000000000000110
}

[wind@starry-sky sigaction]$
```

------------

现在我们回到之前那个链表数据插入的例子, 我们把上面那种含有原子化行为的函数叫做不可重入函数, 对于不可重入函数来说, 必须要确保它要么不执行, 要执行就必须一次性执行完, 这里的重入就是重复进入, 进程在并发控制流中, 重复的进入某个函数, 称之为"函数重入", 我们目前学习的函数, 绝大多数都是不可重入函数, 重入就可能会发生问题, 

一个函数不可重入主要有两种原因, 一是这个函数本身就包含着原子化的行为, 比如链表的插入, 二是这个函数调用的其它函数中包含着原子化行为, 标准库中的许多函数, 由于其内部大都维护着某种数据结构, 所以绝大多数都是不可重入函数, 比如负责管理堆空间的`malloc`和`free`, 就是不可重入函数, 因为堆空间本质上是用全局链表进行管理的, 又比如各种`IO`函数, 它们底层都是对某些全局数据结构的管理, 这些数据结构的种种操作, 就是不可重入的, `IO`的各种函数, 则继承了这些操作的不可重入性. 

## Volatile  

编译器有时会对代码进行优化, 这种优化往往是在效率上的优化, 但优化不一定是好的, 因为某些优化的方式或者说手段会破坏程序的原本逻辑, 此时就会引发一些奇怪的事情. 比如下面的代码

````cpp
#include<iostream>
#include<signal.h>
#include<sys/types.h>
#include<unistd.h>

using namespace std;

int flag = 1;

void handler(int event)
{
    flag = 0;
    cout << endl << "flag = " << flag << endl;
}

int main()
{
    cout << "pid: "<<getpid()<<endl;
    signal(2, handler);
    while(flag);

    cout << "after" << endl;

    return 0;
}
````

`main`函数里有个循环语句, 它将全局变量`flag`作为循环条件, 循环语句中不涉及对`flag`的修改, 当进程收到了二号信号, 就对`flag`进行修改, 从而跳出循环, 也就是说, 程序本身并不清楚何时停止循环, 它是依靠外界的信息, 以信号的方式, 来让程序跳出循环.

```shell
[wind@starry-sky Volatile]$ g++ main.cc -o w
[wind@starry-sky Volatile]$ ./w
pid: 13410
^C
flag = 0
after
[wind@starry-sky Volatile]$ 
```

我们之前曾经提到过计算机的多级存储方式, 外存, 或者说外设,  内存,  CPU中的寄存器,  一个数据如果要访问, 本质上, 是把它写到CPU的寄存器中, 如果是要修改, 就是在寄存器里面修改之后再写回原来的地方.

我们还知道, 计算机本质上运行的都是机器码, 只不过CPU自带了汇编指令集, 所以编译器只要把代码翻译成汇编代码就行了, 在编译器把C/C++代码翻译成汇编代码的具体过程中, 就可以对代码进行优化, 比如, 对于上面的代码来说, 完全不优化的汇编逻辑可能是, `flag`放在内存里, 检测循环条件时, 就把`flag`从内存读到寄存器中, CPU再对寄存器中的`flag`进行逻辑判断.

但在上面的这份代码里, 编译器发现, 循环语句内部没有对`flag`进行修改, 所以它就可以进行优化, `flag`最初还是在内存中, 在第一次进行变量检测的时候, 还是把`flag`从内存写到寄存器中, 但由于循环语句中没有修改`flag`, 所以我之后再进行循环条件判断的时候, 就不需要再把`flag`从内存写到寄存器中了, 我直接把第一次逻辑检测的被写到寄存器中的那份数据保护起来, 不允许别人修改,等到之后再进行逻辑检测的时候, 直接读寄存器中的那份数据, 而不是把内存中的`flag`写到寄存器中再去进行逻辑判断, 寄存器访问速度肯定比内存快, 这样就提升了程序的运行效率, 程序就能跑的更快.

但现在的问题是, 信号自定义行为和`main`主控制流是相互独立的, 但共用同一份数据, 所以当信号被接收到时, `flag`会被写到另一个寄存器上, 在那个寄存器上修改, 修改完后再写回内存, 于是负责逻辑判断的, 在寄存器中的`flag`并没有被修改, 所以再次回到`main`中之后, 逻辑判断时用的`flag`还是之前未被修改的`flag`, 所以就会一直循环下去.

为了避免出现类似的情况, 编译器对优化等级进行了划分, 常见的是`-O0  -O1 -O2 -O3`,

![image-20250125192436353](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250125192436531.png)

 g++ 默认`-O0`, 即不优化, 随着`-O`后面的数字越来越大, 优化程度就越高, 比如, 既然`flag`都不修改, 只要第一次为真那不就恒为真, 下次都不用逻辑判断, 一直循环就行.

```shell
[wind@starry-sky Volatile]$ g++ -O0 main.cc -o w
[wind@starry-sky Volatile]$ ./w
pid: 13701
^C
flag = 0
after
[wind@starry-sky Volatile]$ g++ -O1 main.cc -o w
[wind@starry-sky Volatile]$ ./w
pid: 13718
^C
flag = 0
^C
flag = 0
^C
flag = 0
Killed
[wind@starry-sky Volatile]$
```

从`-O1`开始就已经无法退出了, 需要另一个窗口手动发送`-9`信号, 更高等级自然更无法退出.

为了避免上述情况的发生, 在编译时, 我们可以不进行任何优化,  或者也可以在`flag`的定义前面加上`volatile  `, 加上`volatile`就相当于向编译器特别强调, 不管编译时的优化选项如何, `flag`都不要优化, 每次访问, 都必须使用内存里面的`flag`.

## SIGCHLD  

第十七号信号`SIGCHLD`在普通信号中比较特殊, 它主要是在进程等待那里发挥作用.  在之前, 我们说, 子进程退出后, 父进程需要对其进行等待, 才能回收子进程资源, 等待有两种模式, 一种是阻塞等待, 子进程不退出父进程就一直在`waitpid`那里阻塞, 另一种是轮询等待, 如果子进程没退出, 就返回0, 父进程接着运行, 过一会再去等待, 直到返回`pid`, 等待结束.

实际上, 还有第三种等待模式, 就是使用信号进行等待, 子进程退出后, 会向父进程发送第十七号信号`SIGCHLD  `, 这样父进程就可以立刻知道有子进程退出了, 而不用一直等待或者间隔一段时间等待.

`SIGCHLD  `的特别之处在于它的行为, 它的默认行为, 也就是`SIG_DFL`, 是忽略, 这里的忽略和`SIG_IGN  `不是同一个概念, 它的意思是, `block`中`SIGCHLD`对应的比特位还是0, 也就是说, 它没有不阻塞, 但是它的对应行为分支是空的, 也就是说, 它完全把进程等待这件事交给用户来做, 所以行为是忽略, 而`SIGCHLD`的`SIG_IGN`行为则是对子进程进行最基础的等待, 毕竟`SIG_IGN`是系统里的,  它并不清楚子进程的具体需求, 所以它也不知道到底具体怎么回收, 所以它就进行一下最基本的等待, 确保不发生内存泄露或者说进程僵死就行了, 也就是说, `SIG_IGN`的等待做的很粗糙, 无法进行针对化设计. 

```cpp
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>
#include<iostream>

using namespace std;

int main()
{
    signal(17, SIG_IGN);
    pid_t id = fork();
    if(id > 0)
    {
        cout<<"父进程pid是: " <<getpid()<<endl;
        int count = 6;
        while(count--) sleep(1);
    }
    else
    {
        cout<<"子进程pid是: " <<getpid()<<endl;
        int count = 2;
        while(count--) sleep(1);
    }
    return 0;
}
```

```shell
[wind@starry-sky SIGCHLD]$ ./w.out
父进程pid是: 19004
子进程pid是: 19005
[wind@starry-sky SIGCHLD]$


[wind@starry-sky SIGCHLD]$ while :; do ps ajx | head -1 ; ps ajx | grep w.out | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379 19004 19004  7379 pts/0    19004 S+    1002   0:00 ./w.out
19004 19005 19004  7379 pts/0    19004 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379 19004 19004  7379 pts/0    19004 S+    1002   0:00 ./w.out
19004 19005 19004  7379 pts/0    19004 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379 19004 19004  7379 pts/0    19004 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379 19004 19004  7379 pts/0    19004 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379 19004 19004  7379 pts/0    19004 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 7379 19004 19004  7379 pts/0    19004 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
^C
[wind@starry-sky SIGCHLD]$
```

这样就自动回收了, 没有造成进程的僵死.   不过就像上面说的那样, `SIG_IGN`只是最基本的回收, 所以即使真的使用信号这种方式进行进程等待, 更多的也是使用自定义信号行为的方式.

当然, 这种方式的自由度还是不够高, 所以我们也可以完全使用自定义信号行为的方式对子进程进行回收.

```cpp
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>
#include<iostream>

using namespace std;

void handler(int event)
{
    cout<<"这里是"<<getpid()<<"↓"<<endl;
    cout << "接收到一个信号: " << event << endl;
    int status = 0;
    pid_t id = waitpid(-1, &status, 0);
    cout << "回收子进程资源: "<<id<<endl;
}

int main()
{
    signal(17, handler);
    pid_t id = fork();
    if(id > 0)
    {
        cout<<"父进程pid是: " <<getpid()<<endl;
        int count = 6;
        while(count--) sleep(1);
    }
    else
    {
        cout<<"子进程pid是: " <<getpid()<<endl;
        int count = 2;
        while(count--) sleep(1);
    }
    return 0;
}
```

```shell
[wind@starry-sky SIGCHLD]$ ./w.out
父进程pid是: 4549
子进程pid是: 4550
这里是4549↓
接收到一个信号: 17
回收子进程资源: 4550
[wind@starry-sky SIGCHLD]$

[wind@starry-sky SIGCHLD]$ while :; do ps ajx | head -1 ; ps ajx | grep w.out | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2466  4549  4549  2466 pts/0     4549 S+    1002   0:00 ./w.out
 4549  4550  4549  2466 pts/0     4549 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2466  4549  4549  2466 pts/0     4549 S+    1002   0:00 ./w.out
 4549  4550  4549  2466 pts/0     4549 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2466  4549  4549  2466 pts/0     4549 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2466  4549  4549  2466 pts/0     4549 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2466  4549  4549  2466 pts/0     4549 S+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
^C
[wind@starry-sky SIGCHLD]$
```

现在有一个问题, 如果有多个子进程同时退出怎么办, 我们知道信号的`pending`是位图结构, 这意味着, 当进程收到重复的多个信号时, 它只会执行一次信号对应行为, 那岂不是存在子进程无法回收的情况, 此时, 我们就需要采用轮询等待的方式进行进程回收了, 我们把等待内容放在一个循环语句中,依据`waitpid`的返回值判断是否继续循环

```cpp
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>
#include<iostream>
#include<vector>

using namespace std;

class processTable
{
    static vector<pid_t> _v;

    public:
    static void push(pid_t id)
    {
        _v.push_back(id);
    }

    static pid_t tail()
    {
        return _v[_v.size() - 1];
    }

    static void pop()
    {
        _v.pop_back();
    }

    static void clear()
    {
        _v.clear();
    }
};

vector<pid_t> processTable::_v;

void handler(int event)
{
    static bool flag = true;
    cout<<"这里是"<<getpid()<<"↓"<<endl;
    cout << "接收到一个信号: " << event << endl;
   while(true)
   {
        pid_t id = waitpid(-1, nullptr, WNOHANG);
        if(id <= 0) break;
        cout << "回收子进程资源: "<<id<<endl;
   }

    // 模拟信号处理过程中, 同时有多个进程退出
    if(flag)
    {
        flag = false;
        int count = 4;
        while(count--)
        {
            pid_t id = processTable::tail();
            processTable::pop();

            cout<<"子进程"<< id <<"即将退出"<<endl;
            kill(id, 9);
        }
    }

   cout<<endl;
}

int main()
{
    // 覆写信号
    signal(17, handler);

    // 屏蔽信号
    sigset_t set, oset;
    sigemptyset(&set);
    sigaddset(&set, 17);
    sigprocmask(SIG_SETMASK, &set, &oset);

    cout << "这里是父进程: "<<getpid()<<endl;
    int count = 5;
    while(count--)
    {
        pid_t id = fork();
        if(id == 0)
        {
            processTable::clear();
            cout << "这里是子进程: "<<getpid()<<endl;
            while(true);
            exit(0);
        }
        else
        {
            processTable::push(id);
        }
    }

    sleep(1);
    pid_t tail = processTable::tail();
    processTable::pop();
    cout<<"子进程"<<tail<<"即将退出"<<endl;
    kill(tail, 9);
    sleep(2);
    sigprocmask(SIG_SETMASK, &oset, nullptr);
    // 信号处理

    // 确保有足够的处理时间
    while(true);

    return 0;
}

```

```shell
[wind@starry-sky SIGCHLD]$ ./w.out
这里是父进程: 19822
这里是子进程: 19823
这里是子进程: 19826
这里是子进程: 19825
这里是子进程: 19824
这里是子进程: 19827
子进程19827即将退出
这里是19822↓
接收到一个信号: 17
回收子进程资源: 19827
子进程19826即将退出
子进程19825即将退出
子进程19824即将退出
子进程19823即将退出

这里是19822↓
接收到一个信号: 17
回收子进程资源: 19823

这里是19822↓
接收到一个信号: 17
回收子进程资源: 19825

这里是19822↓
接收到一个信号: 17
回收子进程资源: 19824

这里是19822↓
接收到一个信号: 17
回收子进程资源: 19826

^C
[wind@starry-sky SIGCHLD]$

[wind@starry-sky SIGCHLD]$ while :; do ps ajx | head -1 ; ps ajx | grep w.out | grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
19301 19822 19822 19301 pts/0    19822 S+    1002   0:00 ./w.out
19822 19823 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
19822 19824 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
19822 19825 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
19822 19826 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
19822 19827 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
19301 19822 19822 19301 pts/0    19822 S+    1002   0:00 ./w.out
19822 19823 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
19822 19824 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
19822 19825 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
19822 19826 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
19822 19827 19822 19301 pts/0    19822 Z+    1002   0:00 [w.out] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
19301 19822 19822 19301 pts/0    19822 S+    1002   0:00 ./w.out
19822 19823 19822 19301 pts/0    19822 R+    1002   0:01 ./w.out
19822 19824 19822 19301 pts/0    19822 R+    1002   0:01 ./w.out
19822 19825 19822 19301 pts/0    19822 R+    1002   0:01 ./w.out
19822 19826 19822 19301 pts/0    19822 R+    1002   0:01 ./w.out
19822 19827 19822 19301 pts/0    19822 Z+    1002   0:00 [w.out] <defunct>
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
19301 19822 19822 19301 pts/0    19822 R+    1002   0:00 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
19301 19822 19822 19301 pts/0    19822 R+    1002   0:01 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
19301 19822 19822 19301 pts/0    19822 R+    1002   0:02 ./w.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
^C
[wind@starry-sky SIGCHLD]$ 
```

需要说的是, `SIGCHLD  `, 这种进程回收方式实际上需要有一定操作, 用起来没单纯的`waitpid`直观, 特别是它不好确保子进程退出之前父进程还在运行, 作为初学者, 还是应该用之前进程等待的方法.  而且由于我们对系统的了解还不够深入, 把信号和进程回收合在一起, 容易相互干扰, 从而出现很奇怪的现象.

# end
