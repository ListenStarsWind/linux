# 进程优先级

前面我们说了Linux中进程的诸多状态，为了对这些不同状态的进程进行管理，Linux会使用一种相互嵌套的复杂数据结构将它们组织起来。

对于一般的数据结构来说，可能是节点里含有数据的索引，以链表举例：

```cpp
struct node
{
    date_t val;
    struct node* prev;
    struct node* next;
}
```

但Linux不是这样，而是：

```cpp
struct node
{
    struct node* prev;
    struct node* next;
}

struct task_struct
{
    // 各种信息
    struct node link;
    // 各种信息
}
```

![绘图1](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410191417075.png)

为了方便描述，我们把PCB中节点的起始地址（也就是prev  next指向的位置）叫做start。

那怎么找到PCB起始地址呢？首先我们要算出`link`这个结构体成员相对PCB起始地址的偏移量。试想一下，假如现在有个PCB的起始位置就位于0地址处，那`link`成员的起始地址起始就是`&((task_struct*)0->link)`。

![image-20241019143130629](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410191431742.png)

`(task_struct*)0`就是这个假想PCB的地址，用它可以访问`task_struct`中的成员`link`，得到`link`之后再取地址便能得到这个`link`的地址，然后就是计算`link`相对起始地址的偏移量，为了让地址以整数形式运算所以要强转成`char*`：

`(char*)(&((task_struct*)0->link)) - (char*)0 == (char*)(&((task_struct*)0->link))`

现在让我们把视线从这个假想的PCB上挪开，如果我们已经知道了某个PCB的`start`，想要知道这个PCB的起始地址，就可以用

`(char*)start - (char*)(&((task_struct*)0->link))`

知道了这个PCB的起始地址，我们就可以用它访问PCB中的其它成员信息了。这样做的好处是可以将不同自定义类型的数据也管理起来，也就是说，可以在一个数据结构中存储不同类型的数据。

----------------------

计算机中的CPU是较少的，而系统的进程都是比较多的，这意味着进程对CPU的访问是存在竞争性的，而优先级就规定了谁先使用CPU。优先权高的进程优先使用CPU。

一般情况下，Linux认为系统中的各种进程都是平等的，它会尽可能地公平分配CPU，让每个进程都有被CPU执行的机会。如果一个进程长时间得不到CPU执行，那我们就称这个进程被"饿死"了。外界体现就是只有这个软件卡，然后跳出一个警告：该程序长时间得不到运行，关闭还是等待。

Linux的优先级可以有限度地更改，但一般不要更改，绝大多数情况下，更改都提升不了效率甚至效率反而降低，优先级的安排系统有自己的算法，那算法再差一般也比用户随手改好。

```cpp
#include<iostream>
#include<unistd.h>

int main()
{
	while (1)
	{
		std::cout << "process->  PID:" << getpid() << std::endl;
		sleep(1);
	}
	return 0;
}
```

`ps -l`可以展示当前bash界面的进程，如果想要看所有界面的进程，则要加上选项`a`：

```shell
[wind@starry-sky ~]$ ps -l
F S   UID   PID  PPID  C PRI  NI ADDR SZ WCHAN  TTY          TIME CMD
0 R  1002  4152 16395  0  80   0 - 38324 -      pts/1    00:00:00 ps
0 S  1002 16395 16394  0  80   0 - 29151 do_wai pts/1    00:00:00 bash
[wind@starry-sky ~]$ ps -al
F S   UID   PID  PPID  C PRI  NI ADDR SZ WCHAN  TTY          TIME CMD
0 S  1002  3718 13801  0  80   0 -  3307 hrtime pts/0    00:00:00 ProcessPriority
0 R  1002  4160 16395  0  80   0 - 38332 -      pts/1    00:00:00 ps
[wind@starry-sky ~]$ ps -al | head -1 ; ps -al | grep ProcessPriority
F S   UID   PID  PPID  C PRI  NI ADDR SZ WCHAN  TTY          TIME CMD
0 S  1002  3718 13801  0  80   0 -  3307 hrtime pts/0    00:00:00 ProcessPriority
[wind@starry-sky ~]$
```

其中和优先级有关的属性是`PRI  NI`。`PRI`源于"Priority"，`NI`则是"nice"。`PRI`就是进程的当前优先级，这里的`80`是优先级的基值，`NI`则是优先级的修正值，共有40个档位：-20到19，只能通过修改`NI`来修改优先级，`PRI`越大优先级越高；且修正值`NI`都是基于基值来说的，防止左脚踩右脚上天，就像某些游戏角色增益效果只能用于别人不能用于自身。

`nice`和`renice`指令可以修改`NI`，不过这里我们用`top`指令。注意使用`top`指令需要root权限。

```shell
[wind@starry-sky ~]$ sudo top
[sudo] password for wind:
```

![image-20241019165402468](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410191654611.png)

按下`r`：

![image-20241019165436499](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410191654624.png)

输入调整进程PID，回车：

![image-20241019165527831](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410191655955.png)

输入新的`NI`值，回车，如果输入的`NI`值超出范围，会按照最近的极值计算，比如输入-30会按照-20修改：

![image-20241019165803418](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410191658532.png)

按`q`退出top界面

```shell
[wind@starry-sky ~]$ ps -al
F S   UID   PID  PPID  C PRI  NI ADDR SZ WCHAN  TTY          TIME CMD
0 S  1002  3718 13801  0  60 -20 -  3307 hrtime pts/0    00:00:00 ProcessPriority
0 R  1002  6604 16395  0  80   0 - 38332 -      pts/1    00:00:00 ps
[wind@starry-sky ~]$
```

我们再改一下`NI`,输入`10`，之后的优先级是`90`，`NI`的修改都是以基值作为参考的。

```shell
[wind@starry-sky ~]$ ps -al
F S   UID   PID  PPID  C PRI  NI ADDR SZ WCHAN  TTY          TIME CMD
0 S  1002  3718 13801  0  90  10 -  3307 hrtime pts/0    00:00:00 ProcessPriority
0 R  1002  6791 16395  0  80   0 - 38332 -      pts/1    00:00:00 ps
[wind@starry-sky ~]$
```

----------------------------

优先级好了，接下来就要说说操作系统是怎么用这个优先级的。

Linux对优先级的使用用到了一种名叫"位图"的数据结构。位图其实就是一串二进制序列，它通过这些二进制序列的状态传达某种信息。比如可以通过下面的结构构造一个800个位的位图：

```cpp
struct bitsmap
{
    char bits[100];
}
```

如果想修改第423位的值，就先用423整除成员的比特数，char是八个比特位，423/8等于52，这就意味着要改的比特位处于下标为52的成员里，然后再拿423%成员比特数，得到是7，所以423就在52下标成员的第七位上，然后通过某些位操作修改第七位的值。

在运行队列里，Linux会定义两个指针数组：

```cpp
struct runqueue
{
    // others
    task_struct** run;
    task_struct** wait;
    task_struct* running[140];
    task_struct* waiting[140];
    // others
}
```

`NI`有40个档位，或者说`PRI`的范围是60到99，它会采取140的后40个下标，也就是100到139这40个下标，去和这40个档位一一对应。

比如先来了一个`PRI`为80的，就把它PCB指针放在120下标那里，然后又来一个`PRI`80的，就以链表的形式把后来的进程PCB指针连在前一个指针的后面，然后来了一个`PRI`是81的，就放在121下标那里，以此类推。

上面的过程发生在`waiting`数组里，它被`wait`二级指针指着，于此同时`run`二级指针指向`running`，等到`running`里的进程都运行过一轮了，就让`run`和`wait`交换指针内容，使得`run`指向`waiting`而`wait`指向`running`；并让`running`插入新的指针。就这样来回切换。

位图用与判断进程有没有都运行过一轮，由于指针数组里指针太多了，所以不能用遍历判断是否为空。Linux会定义一个40个位的位图，每个位与数组的100到139这40个下标一一对应，如果某个下标的链表进程都遍历完了，就把位图对应位的数字置为0，然后根据位图找下一个不为空的下标，当这40个位全为0了，就说明已经都运行过一轮，就会切换到另一个数组。

---------------------------

我们上面说的都是一般进程，还有一种进程讲究实时响应，所以有些进程是要放在0到99下标的，比如汽车的刹车进程。实时进程我们现在不探讨。

# 完