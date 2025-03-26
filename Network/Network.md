# 网络

## 网络基础 Ⅰ

所谓网络通信, 其实就是把对方需要的数据放在一个特定形式的容器里, 然后依据一些规定, 在网络之间拷贝过来拷贝过去, 最终对方收到对象后, 再把数据取出来进行处理. 在这个过程中, 容器的形式是怎么样的, 具体是怎么传的, 收到之后又怎么解析, 都不是乱来的, 而是遵循着大家都承认的一个约定, 我们把这种约定, 称之为"协议".

如果把网络极简抽象, 每个主机或者说网络节点就是一个个文件, 这些文件一方面可以存储数据, 另一方面, 可以把自己身上的数据写到其它文件中, 在写来写去的过程中, 数据就完成了传送,   这些文件集群没有明显的中心, 或者说每个文件都是中心, 每个文件看上去似乎都是相同的, 它们都需要面对一些问题, 比如我该如何解释从别的文件里写过来的数据, 如果我要继续往下传, 我该传给谁, 知道传给谁后, 如果传送失败该怎么办.... 协议都规定了这些问题的解决方式, 比如, 数据链路层协议告诉你这个数据下一次该传给谁, 文件之间如何区分定位, 则是ip协议负责的内容, 数据传到中间丢了怎么办, 这是 tcp 协议发挥作用的地方, 接收到的数据该如何解释, 则是fttp, fttps, http... 它们的事.

为什么有这么多问题? 因为文件太多了, 情况太多了, 链路太长了.

我们可以把协议看做抽象类, 它是一系列声明, 告诉你, 如果要传输数据, 你应该要干什么, 而具体的协议实现则是继承这个抽象类, 提供了接口的实现方式, 而每个数据包, 就是通过该类实例化的对象, 存储数据.

### 协议的分层

网络是个很复杂的系统, 对于这种系统来说, 我们往往采用分层的概念, 其实在之前的线程学习阶段中, 我们就无意中写出了分层的代码.

让我们回顾一下.

我们在写生产消费者模型的时候, 为了让程序的运行效果更加明显, 我们总喜欢往里面加些信息的打印.   最开始, 我们把打印放到锁外, 但由于所有的线程用的都是同一个屏幕, 所以经常会出现打印混乱的情况, 甚至, 有些打印虽然不混乱, 但内容具有误导性, 在实际运行中, 实际上是生产者先运行, 然后消费者再运行, 但在打印的时候, 消费者却率先运行执行了打印指令, 先打印了消费数据的信息, 然后再是生产者打印生产数据的信息.         为了解决类似问题, 同时考虑到多线程程序设计时的原则:  临界资源应该尽可能放到一块进行访问,   所以我们设计了一个数据守卫层,   把一个数据放入容器必须要经过数据守卫层的插入接口, 而将数据取出容器也必须经过数据守卫层的取出接口, 生产者和消费者在经过守卫层的插入取出接口时, 将会执行由用户提供的回调函数: 打印相关的消息.  单从最外层的线程行为函数上来看, 完全看不到数据守卫层的痕迹, 数据以什么样的形式进入, 它就会以什么样的形式被取出, 在逻辑上就像是直接相连一样. 稍后我们会在网络中也见到相似的场景, 只不过网络层数更多了.

![image-20250311121752401](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311121752492.png)

什么是协议, 协议就是一种约定, 在线程行为层, 我们用的都是同一个类型, 生产者生产一个任务, 消费者就能得到一个任务, 它们在逻辑上是相连的, 我们可以完全不考虑其它层的情况, 在容器层, 容器也不在乎生产者生产的是什么任务, 甚至线程行为层换成别的`task`类, 它也无所谓, 同样的, 它也不需要考虑守卫层内部的具体情况, 它们在逻辑上也是相连的, 我往下传了什么对象, 对面的就会收到什么对象, 在逻辑上也是相连的. 对于数据守卫层来说, 它也不在乎上层具体是怎么实现的, 这个容器可以用队列, 也可以用数组, 还可以用其它接口, 我不在乎, 我只需要在收到对象的时候, 顺手执行一下主线程提供的信息打印函数, 给容器层数据的时候再顺手执行一下对应的打印函数即可.

分层的好处, 用行话来说, 就是高内聚低耦合,  高内聚就是各层只负责各自的事, 行为层, 你只需要生产消费数据就行了, 你不需要操心, 数据是怎么传的, 会不会发生数据不一致问题.  容器层也不用管上层用的到底是什么类型, 你只需要在合适的地方上锁, 保证线程安全就行了, 守卫层只负则在数据进入拿出时取出数据执行回调函数就行, 不用担心线程安全, 因为锁是从容器层那里开始上的, 也只能从容器层那里解. 低耦合就是各层之间没有很强的联系, 行为层换任务类了, 无所谓, 下层不需要改代码, 容器层换容器了, 无所谓, 上层该插入就插入, 该取出就取出.  守卫层增加了功能, 对于其它层来说也没有影响.  各层改变的影响只会在层内传播, 不会跨层传播.  这样就便于维护和优化.

 下面我们回到网络.

对于网络来说, 更是要分层.  两个用户, 张三和李四, 你们在进行网络通信时, 你也不需要考虑什么网络问题, 你们要关心的, 就是张三说的话李四能听懂, 李四说的话张三也能听懂, 这就够了, 我们在用户层要遵循人类语言协议, 要说大家都能听懂的话, 在感觉上, 底层的技术细节被隐藏了, 张三在电脑上输入的是什么话, 李四收到的就是什么, 反之亦然, 张三和李四就像是在直接沟通, 他们在逻辑上是连为一体的.

![image-20250311130324127](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311130324201.png)

我们在语言层用的是汉语, 要遵循汉语的协议,  设备层用的是座机.也要准许相应的座机协议.

现在我们换成无线电, 那该说什么话, 就说什么话, 不需要在乎设备层的更换.

![image-20250311130624544](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311130624598.png)

如果改成说英语, 那设备层也是该怎么样就怎么样, 不需要因为语言的改变而做一些特殊操作

![image-20250311130748218](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311130748269.png)

 网络要分层有内因也有外因.   从外因上说, 是因为分层天然能产生高内聚低耦合, 这是一种好的程序设计思路. 从内因来说, 网络通信中额问题天然是分层的, 首先要保证每个主机能相互连接起来, 这样一个主机到另一个主机才有通信的可能, 打个比方, 张三遭遇了海难, 到了一个孤岛上面, 那个孤岛旁边根本没有航线, 李四想要给张三一些东西根本无从谈起.  现在各个主机相连了, 这就确保, 最起码是存在一条路径, 能把主机`A`和主机`B`连起来, 这才有通信的可能.   确保最起码有这样一条路之后, 我们要保证数据在传输过程中, 不会迷路, 每遇到一个路口, 它都知道, 我应该往那条路走, 而不是往那条路走.   如果还是用之前的例子, 那就是要确保运输物资的那条船不要偏离航线.   接着, 张三收到了李四传来的物资, 接下来要解决的问题, 是张三要知道李四发的是什么东西, 知道这个东西该如何解释, 这就是应用层的事, 李四给张三发来一个火箭, 可以张三根本不会用, 那也是白搭.

那具体该如何分层呢?

国际标准化组织提出了一种被称为"开放式系统互联"`Open Systems Interconnection`, 简称为`OSI`的概念模型., 它把网络从逻辑上分为了7层. 每一层都有相关、相对应的物理设备，比如路由器，交换机;  这个模型在理论上设计的非常好, 它的最大优点是将服务、接口和协议这三个概念明确地区分开来，概念非常清楚, 因此经常出现在教科书上,  已经成为了一种经典模型了.

![image-20250311143430783](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311143431121.png)

但是它分的实在太细了, 所以在日常工程实践中, 我们用的往往另一种简化的模型, 称之为"TCP/IP五层(或四层)模型 "

### TCP/IP模型

TCP/IP通讯协议采用了5层的层级结构，每一层都呼叫它的下一层所提供的网络来完成自己的需求, 同时为更高层提供对应的服务.

- 物理层: 负责光/电信号的传递方式. 比如现在以太网通用的网线(双绞 线)、早期以太网采用的的同轴电缆(现在主要用于有线电视)、光纤, 现在的wifi无线网使用电磁波等都属于物理层的概念。物理层的能力决定了最大传输速率、传输距离、抗干扰性等. 集线器(Hub)工作在物理层.
- 数据链路层: 负责设备之间的数据帧的传送和识别. 例如网卡设备的驱动、帧同步(就是说从网线上检测到什么信号算作新帧的开始)、冲突检测(如果检测到冲突就自动重发)、数据差错校验等工作. 有以太网、令牌环网, 无线LAN等标准. 交换机(Switch)工作在数据链路层.
- 网络层: 负责地址管理和路由选择. 例如在IP协议中, 通过IP地址来标识一台主机, 并通过路由表的方式规划出两台主机之间的数据传输的线路(路由). 路由器(Router)工作在网路层.
- 传输层: 负责两台主机之间的数据传输. 如传输控制协议 (TCP), 能够确保数据可靠的从源主机发送到目标主机.
- 应用层: 负责应用程序间沟通，如简单电子邮件传输（SMTP）、文件传输协议（FTP）、网络远程访问协议（Telnet）等. 我们的网络编程主要就是针对应用层.

![image-20250311144200162](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311144200582.png)

我们一般不操心, 实际上操心也没用, 物理层的事, 所以也有把TCP/IP说成是四层的.表示层和会话层是被归纳为应用层的.

其中, 网络层和传输层被内嵌到系统之中, 它们也是五层里面最重要的两个层, 其中的代表协议就是TCP, IP, 所以这个模型被叫做"TCP/IP模型 ". 有时我们也把这个模型叫做"TCP/IP"协议栈.

物理层负责提供光电信号的基本载体,  链路层让各种设备可以互相连接, 网络层负责为数据流确定传输方向, 传输层负责数据在传输过程的稳定, 应用层负责数据的使用.

在继续谈网络之前, 先让我们把系统和网络放在一块说说.

![绘图1](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410111425950.png)

如图, 这是计算机的分层结构.

网络的各种层都被内嵌的计算机的对应层中. 比如, 物理层就被内嵌进网卡中, 数据链路层则是网卡驱动的一部分, 图中的系统调用和操作系统可以合称为系统内核, 其中就包含着网络层和传输层的实现, 而剩下的用户部分, 则对应着应用层.

每一次网络通信, 都必须要贯穿整个计算机, 都必须贯穿整个协议栈, 区别只是在于方向不同, 对于发送者, 是由上而下贯穿, 对于接受者, 则是由下而上贯穿, 网络被计算机内化为自身的一部分, 网络通信的本质就是贯穿协议栈的过程.

### 网络传输基本流程

我们先来看张图

![image-20250311163337436](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311163337511.png)

这里要强调的是, 网络的各层都是依托在计算机上的, 不要光看到各种各样的网络层, 更要意识到, 它们背后是计算机的分层结构. 每个竖列都相当于上面我们那张计算机分层图的化简.

我们知道, 想要让任意两台主机通信, 首先要做的就是让相近的主机连接起来, 形成一个局域网. 同一个局域网下的设备, 可以进行直接通信. 比如, 在日常生活中, 我们可能在手机上看到了一部好看的电影, 你想和家人一起看, 但电视上找不到免费的资源, 所以就可以直接把手机的播放内容投屏到电视上, 这样做的前提是, 电视和手机必须连在同一个局域网下.

保障局域网正常运行的协议就是局域网协议, 局域网协议有很多种, 比如以太网, 令牌环网, 无线LAN... 上面图中用的就是以太网, 也是用得最多, 最广泛的局域网协议.

我们知道, 网络中的每一层都有自己的协议, 而在实际代码中, 我们通过对高层传来的数据上添加额外数据的方式, 落实协议, 这些额外数据, 就被称之为"报头", 在代码中, 它以结构体或者字符串集合的方式呈现, 是对应网络层通信双方都能理解的内容. (我们之前说过, 通信双方的相同网络层在逻辑上都可以认为是直接沟通的), 我们会把一些需要的信息填充到结构体的各个成员或者各个字符串上, 为了方便起见, 我们先姑且认为, 报头都是结构体.

现在客户想要对服务器发送一份数据, 应用层中就会首先在这份数据上加上应用层协议的报头. 报头和数据就构成了一份报文, 报头中有很多属性 描述了应用层协议的各种信息,  比如, 在下图中, 我们就在报头中加入了应用协议的版本号. 将来, 服务器的应用层会获得同样的一份报文, 会将其进行解包, 获得同样的数据.如果版本号是最新的, 那服务端就按照最新的方式来解释数据, 如果是老版本, 就要用对应的解释方式, 否则就会发生混乱, 造成信息的错误.

![image-20250311173146538](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311173146617.png)

我们说过, 通信必须贯穿协议栈, 所以这份报文将会被传到传输层, 传输层也要加上自己的协议报头. 传输层要保证数据在传输过程中不发生错乱, 所以需要给数据带上序号, 保证数据在传输过程中的有序性.

![image-20250311173840509](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311173840571.png)

网络层也是如此, 要加上对应的报头, 它要为数据的传输指引方向, 所以, 报头内容中含有描述数据传输过程中起始和终点位置的信息.

![image-20250311174410581](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311174410646.png)

接着这份报文会被传到链路层, 链路层亦会在其上加上自己的协议报头.

![image-20250311174628581](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311174628650.png)

我们把左边的过程称之为"数据自顶向下的交付", 这其实是一个不断封装的过程.

在每一层中, 去掉本层协议报头的报文被称之为有效载荷, 比如, 对于应用层, 去掉应用层报头后剩下了`date`, 所以`date`就是对应的有效载荷, 而在传输层里, 去掉传输层报头, 剩下的是应用层报头和`date`, 所以它们两个就是传输层的有效载荷...

之后, 报文就会在以太网中传播, 于是对面服务器的网卡就捕捉到了这条报文. 然后交给了以太网驱动程序, 因为用的是同样的链路层协议, 所以服务器的以太网驱动程序就可以识别出这条报文中哪些是链路层协议, 哪些是有效载荷, 于是它就把报头与有效载荷分离, 把有效载荷传到上一层, 上一层的"IP"协议就也能识别出自己层的报头, 就会让报头和有效载荷分离, 并把有效载荷传到上一层.... 最终, 用户就拿到了这份数据.   将报文中的有效载荷和报头分离就被称之为"解包"

![image-20250311190636381](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311190636443.png)

同层协议的报文都是相同的, 所以在逻辑上就相当于直接相连.

另外需要注意的是, 同一层协议里有很多中种类, 比如传输层, 除了"TCP"外, 还有"UDP"等其它种类的协议, 所以下一层在往上传有效载荷前还要看准了, 要传到对应的上一层具体协议上.

网络通信的内容就是在不断进行封装和解包的过程.

显而易见, 每个报文在生成之后, 总有要被解包的时候, 所以几乎对于任何一种协议, 都必须有将报头和有效载荷分离的能力,  并且, 为了让之后解包的同层协议知道该把解包后的有效载荷传给上一层中的哪个, 几乎所有的协议, 都要在报头中提供, 决定将自己的有效载荷交付给上一层哪一个协议的能力, 我们把这种能力称为"分用". 这两条是大部分协议的共性, 在将来我们学习具体协议的过程中, 我们就将这两条作为学习的线索, 它们的地位和系中的"先描述, 再组织"相同.

#### 以太网通信

在一个局域网中, 为了大家能够相互区分, 每台主机在局域网中, 都要有自己的唯一一个标识.

我们可以把一个局域网看做一个教室, 局域网中的每个主机都相当于教室中的同学以及老师. 当老师要求张三回答某个问题时, 所有同学都会听到, 但只有张三会进行响应. 这是因为, 大家会对老师说的话进行信息提取, 当发现老师叫的不是自己时, 大家就会把这条消息丢弃, 不进行响应, 而对于张三来说, 他经过信息提取后发现叫的是自己, 所以就会做出响应. 张三回答这个问题, 大家也都能听到, 老师让张三再回答一个问题, 大家都能听到, 但只有张三会再次回答.   这就类似与局域网中的情况, 其中的任何一个主机发出数据, 大家都能收到.

局域网中每个主机的网卡, 都有一串序列, 这就是每台主机在局域网中相互区分的唯一标识符,  称之为"Mac"地址, 当操作系统启动时, 就会读取网卡的"Mac"地址, "Mac"地址在理论上全球唯一, 但在实际上, 只需要同一个局域网下唯一即可.

![image-20250311195311448](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311195311538.png)

我们假设上图代表的就是一个局域网,. 中间的那条线就是以太网, 图中的方框就是局域网中的主机, 每个主机都有自己的"Mac"地址, 当其中的一台主机发送数时, 链路层协议就会把报头拼接到自己的有效载荷上, 链路层报头就描述了数据的原始Mac地址和目标Mac地址. 之后当它把报文发出去后, 局域网中所有主机的网卡都会捕捉到这条报文

![image-20250311200802566](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311200802619.png)

如图H1向局域网中发出了一条报文, 目标是H6.

局域网中的所有主机网卡都会捕捉到这条报文, 之后这些主机就会对接收到的报文进行解包, 解包下来的报头会被分析, 其它主机链路层发现, 这条报文不是发给自己的, 于是它们就会把有效负载舍弃, 而不传给更高层. 所以对于其它主机的上层来说, 它就像从来没收到过这个报文. 而对于H6来说, 它的链路层发现这就是给自己的报文, 而且是从H1那里来的, 所以它就会把解包后的有效载荷传到更高层.

当局域网中的多台主机同时发送报文, 就类似于教室里的同学在大声吵闹, 数据就会相互干扰, 从而丢失了原有的信息, 我们把这种情况, 称之为, "数据碰撞".而在实际生活中, 常见的体现就是上午10点多, 你请假了, 在宿舍里, 网络情况就会非常好, 因为没几个设备在连着局域网, 而等到晚上, 网络就会变得差一点, 因为局域网中的主机多了, 发生数据碰撞的可能也增加了.

为了应对数据碰撞问题, 链路层就有对应的碰撞避免算法: 其实现其实很简单, 碰撞了, 没关系, 我等个随机数的时长, 再把报文发一遍.  如何知道自己发出的报文发生碰撞了? 这也很简单, 链路层可以尝试捕捉自己发出的报文, 捉不到, 那就相当于发生碰撞, 这和我们上面的比喻也很相似. 教室里很嘈杂, 你是李四, 朝张三说话, 结果连你自己都听不清自己在说什么, 那张三自然更听不清.

由于局域网中的主机网卡都能捕捉到局域网中的所有报文, 只不过是解包后发现不是自己的, 在链路层丢弃了, 但实际上, 我们也可以通过系统配置链路层, 让其进入所谓的"混杂模式", 混杂模式会将所有捕捉到的报文都进行解包, 即使不是发给自己的, 也把有效载荷发给上一层, 所以, 这就有一定的数据安全问题, 但也不用太过担心, 一般来说, 数据在应用层会被加密, 所以当别人把包解完后, 拿到的仍然是被加密的数据, 所以也不用过于担心.

在局域网中, 任何时候都只能有一定数目的主机发报文, 当超过该数目时, 就会发生数据碰撞问题,  为了应对这个问题, 一方面, 我们有碰撞避免算法, 另一方面, 我们可以在局域网中添加交换机, 交换机的作用就相当于把局域网分成两个小部分, 就类似与把一个50人的大班拆成25人的两个小班, 分别在两个教室里上课, 这样就能减少数据碰撞的可能.

![image-20250311204645010](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311204645083.png)

在上图中, 整个局域网被交换机分成了左右两个子部分. 当左边没有发生数据碰撞时, 交换机就会让左边的报文进入右边, 而当左边出现数据碰撞时, 交换机就会拦下错误报文, 不让它的影响进一步扩大.

我们把网络中所有可能发生数据碰撞的区域称之为碰撞域, 在交换机被接入前, 上图有一个碰撞域, 而当交换机接入后, 就变为了两个相对独立的小碰撞域, 所以交换机的功能就是划分 碰撞域.

虽然局域网中可能经常发生数据碰撞问题, 但还是不要太担心, 你要相信光电的速度, 它不会多低效.

局域网就类似于所有主机的共享资源, 主机之间应该对局域网进行互斥访问, 它的互斥不是靠锁事前预防, 而是随便访问, 出问题再使用防碰撞算法, 主打事后解决.  不过, 有些局域网是有互斥锁这种东西的, 这种局域网我们称之为"令牌环".

#### 跨局域网通信

下面我们说数据怎么跨局域网中通信, 如图, 这是两个由路由器连接起来的两个局域网, 这里为了更简洁一些, 我们只画了起始主机和目标主机, 你可以脑补一下, 这两边其实还有很多主机,  但在这张图中被省略了.

![image-20250313164558869](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250313164558971.png)

为了方便起见, 我们把左边使用以太网的局域网记为`E`(取自英文"Ethernet"以太的首字母), 右边使用令牌环网的局域网记为`T`(取自英文"Token"令牌的首字母), 客户主机和服务器主机是分别位于`E, T`中的两台主机, 它们将要进行网络通信.

在正式说通信过程前, 我们先来说说令牌环网, 令牌环网对于数据碰撞问题采用的是事前阻止策略, 它会向自己局域网中主机派发令牌, 只有获得令牌的主机才可以将数据包发到局域网中, 其它主机都不能发, 所以对于令牌环网来说, 就不会发生数据碰撞, 它的解决思路就非常像互斥锁, 网络对于各主机来说是共享资源, 也就是临界资源, 只有获得令牌的才可以进行访问. 在绝大多数教材中, 令牌环网都讲的较少, 我们这里也不会细说, 这里两个局域网使用不同的链路协议主要是为了之后突出跨局域网传播的特点.

除此之外, 我们还要再简单了解一下IP地址. IP地址是一台主机在全网的唯一标识符, 当然Mac地址理论上也可以, 但Mac地址一般只用在局域网中, 跨局域网通信是要依靠IP地址的. IP地址可以分为公网IP和私网IP, 比如我们使用Xshell连接远端主机的那个IP地址就是公网IP, 为了方便起见, 我们先不考虑私网IP的情况.

关于IP地址, 我们先来讨论三个话题: IP地址是什么, IP地址为什么要有, IP地址和Mac地址有什么区别.    为了弄清这些话题, 我们需要先假想一个生活场景.

![bigmap](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250313171031567.jpg)

这是一张中华人民共和国的行政地图.

假如你在沈阳, 想去昆明那里看看. 那该怎么走呢? 首先我们能很容易的认识到昆明它不在辽宁省, 它在云南省, 所以我们首先会知道, 想要去昆明, 首先要出辽宁省, 而且你也很明显知道云南它是在南边的, 你应该往南走, 而不是往北走, 所以你首先认识到, 下一站应该去河北, 于是你就找到了从辽宁到河北的路线, 来到了河北.

来到了河北之后, 你又粗略看了一下, 我现在在河北, 云南在河北的西南方向, 所以你把下一站定为陕西, 同样的, 来到了陕西之后,  你认为下一站应该去四川, 于是你就在陕西规划了路线, 顺着路线来到了四川, 你也知道云南就在四川南边, 于是你规划了路线, 来到了云南, 进入云南之后, 你又规划了路线, 终于来到了昆明.

在这个过程中, 为了实现从沈阳到昆明这个大目标, 你划分了一系列小目标, 通过不断地完成小目标, 你最终实现了大目标.   小目标是在不断发生变化的, 但大目标一直都是不变的.

网络甚至是人生它都是这样, 它被划分成了一系列小过程, 这些小过程之间是有序的, 对于一般人来说, 需要一步一步自己走, 不能跳着走, 也不能省略走, 因为跳着走和省略走是有代价, 而你支付不起.

对于跨局域网通信来说, 也是如此, Mac地址在其中指明的是短期方向(在局域网中的方向), IP地址指明的是整体方向.

目前国际主流的IP地址标准是IPV4, 是个四字节的整数, 例如, 192. 168. 1. 1, 使用`.`划分各个字节.   对于国内来说, 没有明确的主流标准, 处于一种IPV4向IPV6过度的阶段. 但大公司一般都必须支持IPV6, 这是政策支持的.

现在我们可以开始说跨局域网通信的过程了.

首先仍是老样子, 客户在应用层发出了一个请求, 请求被发到传输层, 传输层的TCP协议为负载添加了报头, 并继续向下发送, 网络层的IP协议也为负载加上了自己的报头, 报头中含有起始目标IP地址和目标IP地址, 然后继续向下交付, 数据链路层的以太网协议也增加了自己的报头, 为了让数据到达另一个局域网, 数据需要先达到两个局域网中的公有部分, 也就是路由器上, 于是以太网协议在报头中标记的起始Mac地址是客户主机的Mac地址, 而终点Mac地址是路由器的Mac地址.

![image-20250313184349275](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250313184349411.png)

于是, 这份报文被发到了局域网中, 对于局域网中的其它主机来说, 它把数据链路层的报头解出分析之后, 发现不是自己的, 所以就在链路层直接舍弃了, 而对于路由器来说, 它分析了链路层的报头, 发现, 这就是给自己的, 于是把负载又传到了网络层, 网络层的IP协议分析了收到报文的报头, 发现, 目标IP不是自己, 目标IP在另一头的局域网里面, 而另一边的局域网用的是令牌环协议, 于是它把这份报文又给了数据链路层的令牌环协议那里, 令牌环协议就给它封装了报头, 标记起始Mac地址是路由器, 目标Mac地址是服务器.

![image-20250313185551401](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250313185551513.png)

后面的过程我们就不说了, 路由器往令牌环局域网中发了报文, 服务器的网卡捕捉到了这个报文, 这个报文的链路层用的也是令牌环协议, 所以服务器的链路层令牌环协议成功解包, , 并向上分用, 最后, 服务器就拿到了客户发出的请求.

![image-20250313190028954](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250313190029055.png)

正是由于网络层的IP协议实现了全球所有主机在网络上的统一, 大家用的都是IP报文, 所以尽管数据链路层的情况各异, 但所有报文在网络层都能被统一描述, 不管什么设备大家都能连上网. 网络在这一层是统一的, 所以它叫网络层.

而在实际中, 可能报文在传递过程中经过了多个路由器, 尽管链路层的协议可能一直在换, 但没关系, IP层向上的都是不变的.

上面我们说的都只是大致框架, 之后我们会把细节慢慢填补的.

还有一件事, 之前我们都是说哪一层的报文, 但实际上, 它们都是有特定称呼的, 传输层的报文一般叫数据段, 数据包, 网络层一般叫数据报, 链路层一般叫数据帧.以后我们就用这些称呼了.

在我们的远程主机上可以输入`ifconfig`, 查询自己的网络信息. 我们现在只看`eth0`开头的, `lo`以后会讲. `eth0`就是网卡接口号, 如果是`eth33`,可能是因为用的是虚拟机, 没有实体网卡. `inet`后面就是IP地址, 不过这里应该是内网IP,  `ether`我们前面说过, 是以太的意思, 说明这台主机用的是以太网, 后面的那串序列`00:16:3e:1c:d7:8d`就是Mac地址, 六个字节, 字节之间用`:`分割.

```shell
[wind@starry-sky ~]$ ifconfig
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.31.235.81  netmask 255.255.240.0  broadcast 172.31.239.255
        inet6 fe80::216:3eff:fe1c:d78d  prefixlen 64  scopeid 0x20<link>
        ether 00:16:3e:1c:d7:8d  txqueuelen 1000  (Ethernet)
        RX packets 17962134  bytes 4505382980 (4.1 GiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 14773354  bytes 4528326181 (4.2 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 12050015  bytes 2043764675 (1.9 GiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 12050015  bytes 2043764675 (1.9 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

[wind@starry-sky ~]$
```

windows也有对应的指令`ipconfig`

```shell
C:\Users\21066>ipconfig

Windows IP 配置


无线局域网适配器 本地连接* 1:

   媒体状态  . . . . . . . . . . . . : 媒体已断开连接
   连接特定的 DNS 后缀 . . . . . . . :

无线局域网适配器 本地连接* 2:

   媒体状态  . . . . . . . . . . . . : 媒体已断开连接
   连接特定的 DNS 后缀 . . . . . . . :

以太网适配器 VMware Network Adapter VMnet1:

   连接特定的 DNS 后缀 . . . . . . . :
   本地链接 IPv6 地址. . . . . . . . : fe80::ec28:1ea0:562b:6597%13
   IPv4 地址 . . . . . . . . . . . . : 192.168.248.1
   子网掩码  . . . . . . . . . . . . : 255.255.255.0
   默认网关. . . . . . . . . . . . . :

以太网适配器 以太网 3:

   媒体状态  . . . . . . . . . . . . : 媒体已断开连接
   连接特定的 DNS 后缀 . . . . . . . :

以太网适配器 VMware Network Adapter VMnet8:

   连接特定的 DNS 后缀 . . . . . . . :
   本地链接 IPv6 地址. . . . . . . . : fe80::d14:b013:cc76:c841%20
   IPv4 地址 . . . . . . . . . . . . : 192.168.150.1
   子网掩码  . . . . . . . . . . . . : 255.255.255.0
   默认网关. . . . . . . . . . . . . :

无线局域网适配器 WLAN:

   连接特定的 DNS 后缀 . . . . . . . :
   本地链接 IPv6 地址. . . . . . . . : fe80::d4a3:9643:d5f8:adcc%4
   IPv4 地址 . . . . . . . . . . . . : 172.20.203.47
   子网掩码  . . . . . . . . . . . . : 255.255.254.0
   默认网关. . . . . . . . . . . . . : 172.20.203.254

以太网适配器 以太网:

   媒体状态  . . . . . . . . . . . . : 媒体已断开连接
   连接特定的 DNS 后缀 . . . . . . . :

C:\Users\21066>
```

## 网络套接字

### 套接字预备

在说套接字之前, 我们需要先看几个小知识点.

### 端口号

之前, 我们一般是说A主机到B主机要通信, 但实际上, 如果说的更细致一些, 应该是A主机中的某个进程和B主机上的某个进程进行通信, 网络之所以存在, 到底还是为了应用层服务, 是进程需要系统提供网络服务, 网络其实是进程间的一种通信方式, 可以进行数据的远程交流以满足用户的需求, 网络之所以重要, 是因为进程需要它. 上网不是单纯为了上网, 而是为了让本地的客户端进程和远处的服务端进程建立逻辑上的信道, 借助于这个信道, 服务端进程就能把你所需要的数据从远处发过来, 你也能把请求发过去. 网络就是这两个进程的临界资源.

应用层有那么多的进程在等着网络发来的数据, 作为传输层, 解包出来的负载到底该给那个进程呢? 端口号就是为此存在的. IP确定网络中的一个特定主机, 而端口号则确定这台主机上的一个特定进程.

对于本地的一个进程来说, 它在启动之后会先向系统申请一个端口号(实际上就是调用传输层接口, 所以端口号实际上是在传输层的, 但应用层也要使用), 比如它申请到了一个`1234`的端口号, 然后本地进程查了查自己的配置文件, 发现远端进程的端口号是`700`, 于是本地进程就对传输层说, 我这里有个请求, 起始端口号为`1234`, 目标端口号为`700`, 去把它传输到远端, 之后, 在远端的传输层就收到了这样一份请求, 并将负载交给了端口号为`700`的进程, 这个远端进程就知道客户端进程用的端口号是`1234`, 等到服务进程把数据准备好了, 就把数据交给自己的传输层, 告诉他, 起始端口`700`, 目标端口`1234`, 发回去, 于是本地就收到了这份数据.                   注:   对于服务进程的端口来说, 一般有特殊要求,  特定服务应该对应特定端口, 并且服务一般起来就不会关了, 所以端口号也不会改变,     至于用户进程端口, 没什么要求, 在之后的网络编程, 因为我们就一个人, 服务端和本地端都要写, 所以自己和自己约定一下, 服务端用什么端口, 本地端用什么端口.

所以两对IP, 两对端口, 就能让两处的进程相互通信, 我们把这种通信方式称之为"socket", 直译为插座, 就是说, 只要插头查到插座上, 就能通电, 传输数据了.

除了端口号之外, 进程ID也可以确定一台主机上的特定进程, 为什么网络不用进程ID呢? 主要是为了让系统和网络相对独立, 降低耦合度, 进程ID就专门用来进行进程管理, 端口号就专门用来进行网络通信, 如果都用进程ID, 系统变了容易引发网络问题, 网络变了容易引发系统问题, 牵一发而动全身, 太烦人了, 所以要各用各的, 尽量不要相互影响.    另外有些进程也不使用网络资源, 可以没有端口号, 但还是要有进程ID.     还有, 一个进程可能有多个端口号, 可能服务的范围比较宽泛, 多用几个端口细化功能也是可以的,  但一个端口号只能对应一个进程, 端口号要保证唯一性.

端口号在传输层的具体实现, (或者说在系统内核里面, 毕竟传输层是内嵌到内核里面的, ), 可以认为是一个数组, 这个数组还是老样子, 里面存的是`task_struct*`, 进程请求端口号, 相当于在这个数组里面找个空位置, 把该进程的`task_struct*`填进去, 所以之后传输层收到报文后, 就能快速找到这个进程, 另外, 由于Linux一切皆文件, 所以传输层会把解包得到的负载放到文件缓冲区里供进程来读.

### 初识TCP和UDP

TCP和UDP都是传输层的协议, 我们先简单谈一下它俩.

首先是TCP, TCP是面向字节流的, 面向字节流就是一字节一字节传, 它的常见形式是把数据转化成字符串来进行操作, 因为字符串本身就是一个字节一个字节, 另外原因是字符串非常自由, 可读性高, 所以之后我们会遇到序列化和反序列化, 就是数据和字符串转来转去;                    另外, TCP也是有连接的, 它相当于有个通信预处理, 会在建立稳定信道的前提下再进行数据传输, 把一个字节一个字节传过去 它有连接的特点也导致其是具有可靠性的,  可靠性的意思是说, 我会查看数据是否发送成功, 没有发送成功TCP会自己重新补发, 总之出各种各样的问题, TCP都会努力的尝试解决, 保证数据能成功发到对面, 所以应用层不用担心数据发送失败,     但TCP的这些优势也导致它的开销比较大, 之后我们会细说.

至于UDP, 特点就是高效简单. 它没有连接, 所以传输不可靠, 数据发出去就发出去了, 我不管有没有发送成功, 我只是把负载加个报头, 送到下一层就不管了, 另外我这数据也是一次性作为一个整体发出去的, 对面要么整个收到, 要么什么都收不到.   因为它机制简单, 所以非常高效, 也适合给初学者当入门协议使用.

至于到底用TCP还是UDP, 还是要结合具体情况分析, 比如游戏里用UDP传位置数据，因为UDP快, 打游戏就是要快, 延迟那么高怎么玩, 丢点数据没什么事, 大不了让玩家多敲几次键盘, TCP, 常见的就是交易信息, 支付宝肯定要用TCP, 不能说钱打过去对面收不到.

### 网络字节序

我们知道，计算机领域一直存在大小端之争。无论是内存、文件，还是网络传输，都离不开这个话题。所谓小端，就是数据的低位存储在高位地址，而大端则是数据的高位存储在低位地址。

网络数据流的发送和接收通常按从低地址到高地址的顺序进行。对于发送端主机来说，会先发送缓冲区的低地址部分，再发送高地址部分；而对于接收端主机，则是将先收到的数据存入低地址，后收到的数据存入高地址。

为了统一标准，TCP/IP协议规定网络数据流采用大端字节序。无论主机本身是大端还是小端，都必须遵循这一网络字节序来发送和接收数据。如果发送主机是小端机，就需要先将数据转换为大端格式；如果是大端机，则无需转换，直接发送即可。

最初，在设计网络数据传输时，人们并未直接硬性规定使用大端字节序，而是考虑在数据包的报头中添加一个标志位，用来标识数据是大端格式还是小端格式。然而，这个方案最终被放弃了。原因在于，解析这个标志位本身就需要先确定字节序——是按大端读取还是小端读取？但字节序的确定又依赖于标志位的含义，这就陷入了一个“鸡生蛋、蛋生鸡”的循环依赖问题。最终，为了避免这种复杂性和潜在的混乱，TCP/IP协议决定直接统一采用大端字节序（Big-Endian），也就是所谓的“网络字节序”，从而彻底消除了歧义。

至于大小端怎么互相转换, 也不需要担心, C语言有相应的库函数. 直接调用即可.

```cpp
#include <arpa/inet.h>

// h表示host,n表示network,l表示32位长整数,s表示16位短整数
// 例如htonl表示将32位的长整数从主机字节序转换为网络字节序
// 如果主机是小端字节序,这些函数将参数做相应的大小端转换然后返回
// 如果主机是大端字节序,这些 函数不做转换,将参数原封不动地返回

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

```

### socket编程接口

 ```cpp
 // 创建 socket 文件描述符 (TCP/UDP, 客户端 + 服务器)
 int socket(int domain, int type, int protocol);
 // 绑定端口号 (TCP/UDP, 服务器)
 int bind(int socket, const struct sockaddr *address,
          socklen_t address_len);
 // 开始监听socket (TCP, 服务器)
 int listen(int socket, int backlog);
 // 接收请求 (TCP, 服务器)
 int accept(int socket, struct sockaddr* address,
            socklen_t* address_len);
 // 建立连接 (TCP, 客户端)
 int connect(int sockfd, const struct sockaddr *addr,
             socklen_t addrlen);
 ```

除了第一个接口之外, 其它接口都有一个`int socket`的参数, 这其实就是第一个接口返回的文件描述符. 除此之外, 我们还看到有一个`struct sockaddr`在反复出现, 我们下面要来谈谈它.

套接字编程并非只有一种形式，通常可以分为以下三类：

- 域间套接字编程：用于主机内部的进程间通信（IPC）。它通过文件路径标识一个资源，并以套接字的形式实现进程之间的数据交换。
- 原始套接字编程：面向底层的网络通信，可以绕过传输层，直接操作网络层甚至链路层的接口，常用于开发网络工具（如抓包或协议分析软件）。
- 网络套接字编程：基于传输层协议（如TCP/UDP），用于用户之间的网络通信。

尽管这三种套接字编程各有用途，但它们都依赖相同的核心接口（如socket、bind等）。为了在接口调用上实现抽象统一，系统设计了struct sockaddr作为通用类型，用于表示不同套接字的地址结构。例如：

- struct sockaddr_in：专为网络套接字设计，适用于IPv4网络通信。
- struct sockaddr_un：用于域间套接字，支持本地进程通信。
- 原始套接字则更底层，这里暂不展开讨论。

![image-20250314095326053](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250314095326170.png)

对于网络套接字和域间套接字，可以通过地址结构的前两个字节（即sa_family字段）来区分类型：

- 如果是AF_INET，则表示网络套接字（IPv4）。
- 如果是AF_UNIX，则表示域间套接字（Unix域）。

在使用这些接口时，我们通常将struct sockaddr_in或struct sockaddr_un对象的地址强制转换为struct sockaddr*类型传进去。接口内部会自动解析前两个字节，判断具体的套接字类型。从设计上看，struct sockaddr就像一个基类，而struct sockaddr_in和struct sockaddr_un则是它的派生类，这种抽象方式确保了接口的灵活性和兼容性。

在C语言中，实现多态的另一种常见方式是使用void\*指针。这种方式无需显式强制转换，非常灵活。然而，套接字的设计诞生较早，当时的C语言尚未引入泛型指针（如void*）的概念，因此采用了struct sockaddr这样的方式来实现类型抽象。

## UDP网络通信

下面, 我们来写一份使用UDP协议的网络通信代码.

我们先搭建一下具体结构

```cpp
// udpserver.hpp
#pragma once

class udpserver
{
    public:
    void init(){}
    void run(){}
};

// main.cc
#include"udpserver.hpp"

int main()
{
    udpserver svr;
    svr.init();
    svr.run();
    return 0;
}
```

```makefile
udpserver:main.cc
	@g++ $^ -o $@ -std=c++11
.PHONY:clean
clean:
	@rm -f udpserver
```

对于网络通信来说, 首先要做的是创建一个套接字, 使用`man socket`可查询相关接口

```cpp
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

第一个参数`domain`表示套接字的具体类型, 具体选项以宏的方式呈现

```
Name                Purpose                          Man page
AF_UNIX, AF_LOCAL   Local communication              unix(7)
AF_INET             IPv4 Internet protocols          ip(7)
AF_INET6            IPv6 Internet protocols          ipv6(7)
AF_IPX              IPX - Novell protocols
AF_NETLINK          Kernel user interface device     netlink(7)
AF_X25              ITU-T X.25 / ISO-8208 protocol   x25(7)
AF_AX25             Amateur radio AX.25 protocol
AF_ATMPVC           Access to raw ATM PVCs
AF_APPLETALK        Appletalk                        ddp(7)
AF_PACKET           Low level packet interface       packet(7)
```

我们是网络套接字编程, 所以其实上只会使用`AF_INET`或`AF_INET6`, 作为初学者我们当然选择表示`IPv4`的`AF_INET`.

第二个参数`type`表示协议的种类, 同样以宏的方式呈现

```
SOCK_STREAM     Provides sequenced, reliable, two-way, connection-based byte streams.  An out-of-band data transmission mechanism may be supported.

SOCK_DGRAM      Supports datagrams (connectionless, unreliable messages of a fixed maximum length).

SOCK_SEQPACKET  Provides a sequenced, reliable, two-way connection-based data transmission path for datagrams of fixed maximum length; a consumer is
required to read an entire packet with each input system call.

SOCK_RAW        Provides raw network protocol access.

SOCK_RDM        Provides a reliable datagram layer that does not guarantee ordering.

SOCK_PACKET     Obsolete and should not be used in new programs; see packet(7).

```

第一个是面向数据流的, 第二个是面向数据包的, 对于UDP协议来说, 应该选择第二个`SOCK_DGRAM`

第三个参数`protocol`表示具体协议, 如果设置为零, 则由系统根据第二个参数的协议大类选择一个默认的具体协议, 对于`SOCK_DGRAM`来说, 就是`UDP`

当成功时,`socket`的返回值是新创建套接字虚拟文件的描述符,  该文件将充当应用层和传输层交互的接口.   失败时, 返回-1, 并设置`errno`.

好的, 下面我们来写`init`, 首先应该定义一个表示套接字句柄(描述符的另一种说法)的成员变量, 然后去接收`socket`的返回值.    这里把我们之前的`log.hpp`拿过来, 用来做差错处理, 尽管以我现在的眼光来看, 以前写的`log.hpp`并不好, 但是我们先将就用.

```cpp
#pragma once

 #include <sys/types.h>
 #include <sys/socket.h>
 #include"log.hpp"

 // 我觉得用单例模式更适合些, 但这里主题是网络, 就用全局变量凑活用
 // 多加两个下划线防止标识符冲突
 wind::Log log__;

 namespace wind
 {
    enum
    {
        SOCKET_ERR = 0
    };

     class udpserver
     {
     public:
         void init()
         {
             _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sockfd < 0)
            {
                log__(Fatal, "socket create error: %s", strerror(errno));
                exit(SOCKET_ERR);
            }
            log__(Info, "socket create success, sockfd: %d", _sockfd);
         }

         void run() {}

     private:
         int _sockfd; // 套接字句柄
     };
 }
```

```shell
[wind@starry-sky UDP]$ make clean ; make
[wind@starry-sky UDP]$ ./udpserver
[Info][2025-3-15 16:20:16]::socket create success, sockfd: 4
```

我们看到文件描述符是`4`, 而不是`3`, 这没关系, 因为`log`里面复制了一份文件, 把`3`给占上了.

套接字创建成功后，接下来需要进行绑定操作。绑定是将套接字和端口号进行关联。尽管套接字和端口号都具有唯一性，并能标识具体的进程，但正如我们之前对进程ID与端口号的比较，文件系统和网络系统需要解耦。套接字作为虚拟文件不能替代端口号的功能.

```cpp
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr,
         socklen_t addrlen);
```

第一个参数就是我们刚刚创建的套接字描述符, 第二个参数则是套接字的通用类型指针, 我们之前说过, 它其实可以看做基类指针, 用来实现多态调用, 第三个参数是用来描述第二个参数的大小. 成功返回0, 失败返回-1, 并设置错误码

我们用的是网络套接字编程, 所以下面要初始化一个`struct sockaddr_in`的对象, 使用该类型需要包含`<netinet/in.h>`头文件

```cpp
struct sockaddr_in
{
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;			/* Port number.  */
    struct in_addr sin_addr;		/* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof (struct sockaddr) -
                           __SOCKADDR_COMMON_SIZE -
                           sizeof (in_port_t) -
                           sizeof (struct in_addr)];
};

#define	__SOCKADDR_COMMON(sa_prefix) \
  sa_family_t sa_prefix##family
```

`sin_port`就是端口号, `sin_addr`则是IP地址, `sin_zero`是八字节的填充符, 不需要初始化,  `in_port_t`是无符号16位整数, `in_addr`则是又一个结构体, 里面是一个无符号32位的整数.

```cpp
typedef uint16_t in_port_t;

typedef uint32_t in_addr_t;
struct in_addr
{
    in_addr_t s_addr;
};
```

为此, 我们需要在成员变量中再引入表示地址和端口号的对象, 需要注意的是, 一般来说, 我们看IP地址的时候都是`172.31.235.81`这种形式, 这种形式被称为"点分十进制"表示法, 这种IP地址是一个字符串, 所以表示IP地址的成员变量也是字符串, 这意味着着我们之后要把字符串转成32位无符号整数.

接下来我们正式开始实例化一个`struct sockaddr_in`对象, 首先由于它本身是C风格的, 所以在定义后我们要把它给置空. 置空的方法有很多种, 我们使用的是`bzero`, 其头文件是`<string.h>`

语法提示告诉我们`struct sockaddr_in`里面有四个成员, `sin_addr, sin_family, sin_port, sin_zero`, 那个`sin_family`其实是靠宏定义, 把`sin_`和`family`通过`##`连起来的, 这里是想让变量名更加具有适配性.

```cpp
wind::Log log__;

 #define DEFAULT_ADDRESS  "0.0.0.0"

 namespace wind
 {
    enum
    {
        SOCKET_ERR = 0
    };

     class udpserver
     {
     public:
     udpserver(const uint16_t& port, const char* id = DEFAULT_ADDRESS) :_sockfd(0), _port(port), _id(id){}

         void init()
         {
             _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sockfd < 0)
            {
                log__(Fatal, "socket create error: %s", strerror(errno));
                exit(SOCKET_ERR);
            }
            log__(Info, "socket create success, sockfd: %d", _sockfd);

            struct sockaddr_in local;  // 服务端本地的套接字
            bzero(&local, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = _port;
            local.sin_addr = ;
         }

         void run() {}

     private:
         int _sockfd;       // 套接字句柄
         in_port_t _port;   // 端口号
         std::string _id;   // 点分十进制IP

     };
 }
```

接下来我们要做两件事, 一是要把点分十进制IP转换成32位无符号整型, 二是, 要确保端口号和IP地址都是网络字节序列, 第一件事很好理解, 我们对于第二件事要简谈一下, 我们通过套接字虚拟文件把`sockaddr_in`传输层, 传输层就会对`sockaddr_in`进行解析, 它会把其中的端口号加到传输层报头中, 并把IP传到网络层, 网络层也会把IP地址放到报头里, 所以`sockaddr_in`里的端口和IP地址是要真正参与网络传输的, 所以它们也必须使用网络字节序.

不过网络字节序的转换其实也不难, 我们之前已经说过, 它是存在对应接口的, 端口号是16位的, 我们直接调用`htons`即可.  接下来我们要想想怎么把点分十进制转换成整型(其实这个也有对应接口, 也可以直接调用, 不过我们需要了解一下该接口的大致原理)

我们可以先定义一个结构体, 该结构体有32位, 其中有四个8位的成员, 都是无符号的.

```cpp
struct ip
{
    uint8_t part1;
    uint8_t part2;
    uint8_t part3;
    uint8_t part4;
};
```

将来我们收到一个已经转化为本机字节序的, 客户端发来的IP地址, 就可以直接把地址强转, 然后就可以很轻松的解析出这四个字节, 从而转换为点分十进制.

```c
struct ip* p = (struct ip*)src_ip;
to_string(p->part1) + "." +
to_string(p->part2) + "." +
to_string(p->part3) + "." +
to_string(p->part4);
```

把点分十进制转整数, 可以先把它解析成四份, 然后直接转整数就可以了

```c
"192.168.50.100"
uint32_t ip__;
struct ip* x = (struct ip*)&ip__;
x->part1 = stoi("192");
x->part2 = stoi("168");
x->part3 = stoi("50");
x->part4 = stoi("100");
```

别忘了还要把本机字节序转换成网络字节序.                 不过正如上文的括号所言, 字符串和整数相互转, 以及转完再转字节序, 这些事都不需要我们直接做, 它们都是网络通信的基本操作, 所以都有相应的接口, 直接调用即可.

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int inet_aton(const char *cp, struct in_addr *inp);

in_addr_t inet_addr(const char *cp);

in_addr_t inet_network(const char *cp);

char *inet_ntoa(struct in_addr in);

struct in_addr inet_makeaddr(int net, int host);

in_addr_t inet_lnaof(struct in_addr in);

in_addr_t inet_netof(struct in_addr in);

```

我们直接用`inet_addr`就行了, 它不仅会转成整数, 也会把本机字节转成网络字节序

```cpp
wind::Log log__;

#define DEFAULT_ADDRESS  "0.0.0.0"
#define DEFAULT_PORT     8080

namespace wind
{
    enum
    {
        SOCKET_ERR = 0,
        BIND_ERR= 1
    };

    class udpserver
    {
        public:
        udpserver(const uint16_t& port = DEFAULT_PORT, const char* id = DEFAULT_ADDRESS) :_sockfd(0), _port(port), _id(id){}

        void init()
        {
            _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sockfd < 0)
            {
                log__(Fatal, "socket create error: %s", strerror(errno));
                exit(SOCKET_ERR);
            }
            log__(Info, "socket create success, sockfd: %d", _sockfd);

            struct sockaddr_in local;  // 服务端本地的套接字
            bzero(&local, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            local.sin_addr.s_addr = inet_addr(_id.c_str());
            if(bind(_sockfd, reinterpret_cast<const struct sockaddr *>(&local), static_cast<socklen_t>(sizeof(local))) != 0)
            {
                log__(Fatal, "bind error: %s", strerror(errno));
                exit(BIND_ERR);
            }
            log__(Info, "bind success");
        }

        void run() {}

        private:
        int _sockfd;       // 套接字句柄
        in_port_t _port;   // 端口号
        std::string _id;   // 点分十进制IP

    };
}
```

`UDP`的初始化就这些.

```shell
[wind@starry-sky UDP]$ ./udpserver
[Info][2025-3-15 21:0:40]::socket create success, sockfd: 4
[Info][2025-3-15 21:0:40]::bind success
```

接下来我们要`run`一下, 对于`run`来说, 主要就干两件事, 一是接收客户端发来的请求, 二是把用户需要的数据传回去, 而且由于服务器时刻要保持服务, 所以要写成死循环.

尽管套接字是以虚拟文件的形式呈现的, 但不能使用`read, write`来对套接字进行读写, 因为`read, write`的特点是面向字节流, 面向数据包的UDP用不了, 所以UDP有自己的接收请求, 发送数据接口

```cpp
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recv(int sockfd, void *buf, size_t len, int flags);

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
```

`from`就是从套接字里面拿数据, 调用之前, 要先准备好承接负载的缓冲区, 和承接客户端端口IP信息的`struct sockaddr_in`套接字,  `buf, len`分别是负载缓冲区, 和缓冲区的最大容量, `flags`是个标志位, 我们现在先用零, 表示读不到就阻塞, 对于`src_addr`来说, 它用的还是通用类型, 但我们这里是网络通信, 所以还是把`struct sockaddr_in`的地址传进去, `addrlen`则是`struct sockaddr_in`的大小.            如果成功, 返回的是读到的字节, 失败时, 返回-1, 设置`errno`.

在这里我们把接收到的数据视为字符串, 由于字符串的基本元素字符只有一个字节, 所以大端和小端其实都是一样的, 不将网络字节序转换为本机字节序也行. 因为是字符串, 所以别忘了把字符串末尾加上`0`, 之后我们再对字符串进行简单的加工, 把它发回去.

由于我们接收到的`struct sockaddr_in`里面的成员本身就是从网络中发过来的, 是网络字节序, 所以也可以直接用.   对于DUP协议来说, 给别人发数据包用的是`sendto`接口, `sendto`的接口形式和`recvfrom`相同. , 成功返回发送的字节数, 失败返回-1, `errno`被设置

```cpp
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
```

```cpp
void run()
{
    _isrunning = true;
    char buffer[BUFFER_SIZE];   // 约定通信内容为字符串
    while(true)
    {
        struct sockaddr_in remote;
        socklen_t size = sizeof(remote);
        ssize_t len = recvfrom(_sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&remote), reinterpret_cast<socklen_t*>(&size));
        if(len < 0)
        {
            log__(Warning, " recvfrom error: %s", strerror(errno));
            continue;
        }

        // 在用户层接收数据前, 数据的字节序已经被协议自动转换了, 所以这里不需要转
        buffer[len] = '\0';

        // 字符串加工,证明确实来过服务端
        std::string echo("server: ");
        echo += static_cast<char*>(buffer);

        if(sendto(_sockfd, echo.c_str(), echo.size(), 0, reinterpret_cast<const sockaddr*>(&remote), static_cast<socklen_t>(sizeof(remote))) == -1)
        {
            log__(Warning, "sendto error: %s", strerror(errno));
        }
        log__(Info, "sendto success ");
    }
}
```

接下来我们运行一下程序看看效果, `netstat`指令可以查看当前的网络状态, 带上选项`-naup`, `n`的意思是尽可能把信息都以数字的形式呈现, `p`是显示PID信息., `u`是只看UDP协议, `a`表示all.

```shell
[wind@starry-sky UDP]$ netstat -naup
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
udp        0      0 127.0.0.1:323           0.0.0.0:*                           -
udp        0      0 0.0.0.0:713             0.0.0.0:*                           -
udp        0      0 0.0.0.0:8080            0.0.0.0:*                           16960/./udpserver
udp        0      0 0.0.0.0:68              0.0.0.0:*                           -
udp        0      0 0.0.0.0:111             0.0.0.0:*                           -
udp6       0      0 ::1:323                 :::*                                -
udp6       0      0 :::713                  :::*                                -
udp6       0      0 :::111                  :::*                                -
[wind@starry-sky UDP]$ netstat -aup
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
udp        0      0 localhost:323           0.0.0.0:*                           -
udp        0      0 0.0.0.0:iris-xpc        0.0.0.0:*                           -
udp        0      0 0.0.0.0:webcache        0.0.0.0:*                           16960/./udpserver
udp        0      0 0.0.0.0:bootpc          0.0.0.0:*                           -
udp        0      0 0.0.0.0:sunrpc          0.0.0.0:*                           -
udp6       0      0 localhost:323           [::]:*                              -
udp6       0      0 [::]:iris-xpc           [::]:*                              -
udp6       0      0 [::]:sunrpc             [::]:*                              -
[wind@starry-sky UDP]$
```

`Recv-Q, Send-Q`分别代表收发的数据包可数, `Local Address`就是本地地址, 以IP+端口号的形式呈现, 我们的`udpserver`使用的是缺省参数, 所以IP全零, 端口8080. `Foreign Address`表示是否与某些特定IP和端口建立关联(完成数据包的收发), 这里的`0.0.0.0:*`表示还未与任何IP端口建立联系, 但只要有IP端口给我发送数据包, 我都会接收.

现在我们把进程终止, 换个IP看看, 端口号还是使用8080, IP就用`Xshell`连接的公网IP

```cpp
#include"udpserver.hpp"

int main()
{
    wind::udpserver svr(8080, "120.55.90.240");
    svr.init();
    svr.run();
    return 0;
}
```

重新编译, 运行, 我们发现绑定失败

 ```shell
 [wind@starry-sky UDP]$ make clean ; make
 [wind@starry-sky UDP]$ ./udpserver
 [Info][2025-3-16 9:47:51]:: socket create success, sockfd: 4
 [Fatal][2025-3-16 9:47:51]:: bind error: Cannot assign requested address
 ```

这是因为, 我们使用的是云服务器, IP也是一种资源, 云服务器厂商也会对IP进行统一管理(管理方式就是建立一个虚拟层), `Xshell`连接的那个IP实际上是被虚拟化的, 并不是主机真正使用的IP.主机没有这个IP, 自然也就分配不到.

所以对于绑定来说, 云服务器不应该由用户手动绑定IP, 主要有两个原因, 一是云服务器上可能存在多个网卡, 所以也会占有多个IP, 如果显式绑定一个, 那客户端就只能使用显式绑定的IP来访问服务器, 二是, 即使云服务器就一个网卡, 但是, 你也不知道自己的IP地址, 所以就绑不了.                  因此, 对于云服务器来说, 我们建议绑定的IP地址为`0`, 此时系统会根据实际情况为我们动态绑定IP, 一方面系统自己知道自己用的是什么IP, 它不会去绑自己没有的IP, 另一方面, 这种绑定是动态的, 如果主机有多个IP, 客户端使用其中的任意一个都能与服务端通信.

在给`sockaddr_in`IP地址初始化时, 我们也可以使用宏`INADDR_ANY`, 其实就是零.

```cpp
struct sockaddr_in local;  // 服务端本地的套接字
bzero(&local, sizeof(local));
local.sin_family = AF_INET;
local.sin_port = htons(_port);
// local.sin_addr.s_addr = inet_addr(_id.c_str());
local.sin_addr.s_addr = inet_addr(INADDR_ANY);  // 因为是零, 所以其实转不转都一样
if(bind(_sockfd, reinterpret_cast<const struct sockaddr *>(&local), static_cast<socklen_t>(sizeof(local))) != 0)
{
    log__(Fatal, " bind error: %s", strerror(errno));
    exit(BIND_ERR);
}
log__(Info, " bind success");
```

 IP聊完了, 接下来聊一聊端口.

端口都在我主机上, 这回可以随便定了吧. 也不行, 有两个原因, 一是某些端口是被系统内定的, 作为用户不能去使用, 另一方面, 某些特定的端口, 一般用来做特定的服务, 你也不能随便改.

这回我们把端口改成`80`

```cpp
#include"udpserver.hpp"

int main()
{
    wind::udpserver svr(80);
    svr.init();
    svr.run();
    return 0;
}
```

运行, 发现没有权限, 这是因为`0 - 1023`的端口是系统内定的, 要用`sudo`才绑的动.

```shell
[wind@starry-sky UDP]$ make clean ; make
[wind@starry-sky UDP]$ ./udpserver
[Info][2025-3-16 10:39:25]:: socket create success, sockfd: 4
[Fatal][2025-3-16 10:39:25]:: bind error: Permission denied
```

```shell
[wind@starry-sky UDP]$ sudo ./udpserver
[sudo] password for wind:
[Info][2025-3-16 10:42:42]:: socket create success, sockfd: 4
[Info][2025-3-16 10:42:42]:: bind success
```

此时相当于用`root`的身份运行程序, 所以`netstat`也要用`sudo`

```shell
[wind@starry-sky UDP]$ sudo netstat -naup
[sudo] password for wind:
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
udp        0      0 127.0.0.1:323           0.0.0.0:*                           557/chronyd
udp        0      0 0.0.0.0:713             0.0.0.0:*                           547/rpcbind
udp        0      0 0.0.0.0:68              0.0.0.0:*                           813/dhclient
udp        0      0 0.0.0.0:80              0.0.0.0:*                           21598/./udpserver
udp        0      0 0.0.0.0:111             0.0.0.0:*                           547/rpcbind
udp6       0      0 ::1:323                 :::*                                557/chronyd
udp6       0      0 :::713                  :::*                                547/rpcbind
udp6       0      0 :::111                  :::*                                547/rpcbind
```

下面, 为了方便换端口, 我们用命令行方式传参

```cpp
#include"udpserver.hpp"
#include<iostream>
#include<string>

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    wind::udpserver svr(std::stoi(*(argv + 1) + 1));
    svr.init();
    svr.run();
    return 0;
}
```

```shell
[wind@starry-sky UDP]$ ./udpserver -26
[Info][2025-3-16 10:56:32]:: socket create success, sockfd: 4
[Fatal][2025-3-16 10:56:32]:: bind error: Permission denied
[wind@starry-sky UDP]$ sudo ./udpserver -26
[sudo] password for wind:
[Info][2025-3-16 10:57:17]:: socket create success, sockfd: 4
[Info][2025-3-16 10:57:17]:: bind success
```

```shell
[wind@starry-sky UDP]$ sudo netstat -naup
[sudo] password for wind:
Sorry, try again.
[sudo] password for wind:
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
udp        0      0 127.0.0.1:323           0.0.0.0:*                           557/chronyd
udp        0      0 0.0.0.0:713             0.0.0.0:*                           547/rpcbind
udp        0      0 0.0.0.0:26              0.0.0.0:*                           23059/./udpserver
udp        0      0 0.0.0.0:68              0.0.0.0:*                           813/dhclient
udp        0      0 0.0.0.0:111             0.0.0.0:*                           547/rpcbind
udp6       0      0 ::1:323                 :::*                                557/chronyd
udp6       0      0 :::713                  :::*                                547/rpcbind
udp6       0      0 :::111                  :::*                                547/rpcbind
```

上面的都是属于系统内定的端口号, 作为用户来说, 不建议和系统去抢端口, 除此之外, MySQL的端口号是3306, 像这种一般也是默认的.  端口号无符号16位, `0 - 65535`, 你可以选其他的.

-----------

下面我们去写客户端

大致流程差不多, 只不过要注意的是, 对于客户端来说, 理论上需要被绑定, 实际上也会被绑定, 但我们不需要显式地写. 我们不写, 操作系统在客户端向外首次发送数据的时候, 会自动绑定一个随机端口. 客户端和服务端是不一样的, 对于服务器来说, 一般来说就运行某个特定的服务, 但对于客户的主机来说, 它可能会同时运行多个客户端, 可能客户机上有几个社交软件, 有几个购物软件, 有几个外卖软件, 有几个短视频软件....,  它们可能都需要同时运行,  为了绑定未被使用的端口号, 不能要求大家聚到一起, 规定, 这家公司用这个端口, 那家公司用那个端口... 所以对于客户端来说, 我们一般都是让系统自己找个端口绑定, 而不是自己显式的写.    当然这可能会导致客户端的端口号老是在变, 但没关系, 服务端端口号是确定的, 只要客户端和服务端一通信, 服务端也就知道了客户端的端口号.

客户端是有服务端的IP端口号的,我们这里就不写个配置文件读了, 就直接用命令行参数了.

```cpp
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <string>

using namespace std;

#define SOCKET_ERR       1
#define BUFFER_SIZE      1024


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    string remote_ip(argv[1]);
    in_port_t remote_port = stoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        cout << "socket create error: "<< strerror(errno)<<endl;
        exit(SOCKET_ERR);
    }
    cout << "socket create success, sockfd: "<<sockfd<<endl;

    // 交由系统自动绑定

    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons(remote_port);
    remote.sin_addr.s_addr = inet_addr(remote_ip.c_str());

    string message;
    char buffer[BUFFER_SIZE];
    while(true)
    {
        cout << "Pleace Enter@ ";
        getline(cin, message);

        if(sendto(sockfd, message.c_str(), message.size(), 0, reinterpret_cast<const sockaddr*>(&remote), static_cast<socklen_t>(sizeof(remote))) == -1)
        {
            cout << " sendto error: " << strerror(errno);
        }

        struct sockaddr_in temp;
        socklen_t size = sizeof(temp);
        ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&temp), reinterpret_cast<socklen_t *>(&size));
        if (len < 0)
        {
            cout << " recvfrom error: " << strerror(errno) << endl;
            continue;
        }
        cout << buffer<<endl;
    }


    return 0;
}
```

接下来我们来运行一下

```shell
[wind@starry-sky UDP]$ make clean ; make
[wind@starry-sky UDP]$ ./udpserver -8888
[Info][2025-3-16 14:28:50]:: socket create success, sockfd: 4
[Info][2025-3-16 14:28:50]:: bind success
```

```shell
[wind@starry-sky UDP]$ ./udpclient 120.55.90.240 8888
socket create success, sockfd: 3
Pleace Enter@ 你好
server: 你好
Pleace Enter@ swqhsas
server: swqhsas
Pleace Enter@ ^C
[wind@starry-sky UDP]$ ./udpclient 120.55.90.240 8888
socket create success, sockfd: 3
Pleace Enter@ 这里是阿里云
server: 这里是安阿里云
Pleace Enter@ jdscbhsba
server: jdscbhsba
Pleace Enter@
```

```shell
[Info][2025-3-16 14:29:16]::sendto success
[Info][2025-3-16 14:29:17]::sendto success
[Info][2025-3-16 14:29:19]::sendto success
[Info][2025-3-16 14:29:23]::sendto success
[Info][2025-3-16 14:29:25]::sendto success
[Info][2025-3-16 14:29:28]::sendto success
```

对于云服务器来说, 可能出现跑不动的情况, 这可能是由于两种原因, 一是代码本身有问题, 二是云服务器是有端口限制的,  如果是第二种情况, 请去自己主机的控制台的安全组里, 添加对应的端口.

当然, 现在客户端和服务端都是跑在同一个主机的, 上面的测试属于自娱自乐型的网络通信, 下面我们再启动一个云服务器, 来进行真正的跨网络通信.

```shell
[whisper@VM-12-6-centos network_test]$ ./udpclient 120.55.90.240 8888
socket create success, sockfd: 3
Pleace Enter@ hello
server: hello
Pleace Enter@ 你好
server: 你好
Pleace Enter@ asxsah
server: asxsah
Pleace Enter@ tin
server: tin
Pleace Enter@ 听风若依
server: 听风若依
Pleace Enter@
```

接下来, 我们打磨一下代码, 我们仍然把数据识别为字符串, 但对于字符串的处理细节将以外部接口的形式开放给用户. 用户只需要在外部定义一个可调用对象, 参数类型是`const string&`, 返回类型是`string`就行了. 这种行为实际上是在分层, 将接收到的数据处理方式独立出来.

```cpp
#include"udpserver.hpp"
#include<iostream>
#include<string>

using namespace std;


string func(const string& str)
{
    string result("server: ");
    cout << "Get a message: "<<str<<endl;
    result += str;
    return result;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    wind::udpserver svr(std::stoi(*(argv + 1) + 1));
    svr.init();
    svr.run(func);
    return 0;
}
```

```cpp
void run(const function__type& func)
{
    _isrunning = true;
    char buffer[BUFFER_SIZE];   // 约定通信内容为字符串
    while(true)
    {
        struct sockaddr_in remote;
        socklen_t size = sizeof(remote);
        ssize_t len = recvfrom(_sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&remote), reinterpret_cast<socklen_t*>(&size));
        if(len < 0)
        {
            log__(Warning, " recvfrom error: %s", strerror(errno));
            continue;
        }

        // 在用户层接收数据前, 数据的字节序已经被协议自动转换了, 所以这里不需要转
        buffer[len] = '\0';
        std::string temp(buffer);
        // 字符串加工,证明确实来过服务端
        std::string result = func(temp);

        if(sendto(_sockfd, result.c_str(), result.size(), 0, reinterpret_cast<const sockaddr*>(&remote), static_cast<socklen_t>(sizeof(remote))) == -1)
        {
            log__(Warning, "sendto error: %s", strerror(errno));
        }
        log__(Info, "sendto success ");
    }
}
```

接下来我们可以把用户外部定义的`func`, 这回我们不把收到的数据视为纯粹的消息, 而是视为一个命令行指令, 我们将通过创建子进程, 进程替换的方式执行该命令.

考虑到时间原因, 我们这里就不手动写个`fork`再`exec`了, 而是调用C语言系统接口`popen`

```c
#include <stdio.h>

FILE *popen(const char *command, const char *type);

int pclose(FILE *stream);

```

`popen` 在底层类似于我们之前用管道实现的进程池。它会先在内部创建一个管道，然后通过 `fork` 生成一个新进程，并使用 `exec` 将该进程替换为执行字符串形式的 `command` 指令。`popen` 返回的是它内部创建的管道文件描述符，同时，子进程的标准输出会被重定向到这个管道。因此，我们可以通过管道读取子进程的标准输出内容。需要注意的是，最后的 `pclose` 具有进程资源回收的功能，必须调用它以避免出现僵尸进程，从而防止内存泄漏。

```cpp
string ExcuteCommand(const string& str)
{
    if(SecurityCheck(str))
    {
        return "Permission denie";
    }

    FILE* fp = popen(str.c_str(), "r");
    if(fp == nullptr)
    {
        string err(strerror(errno));
        cout<<"popen error: "<<err<<endl;
        return err;
    }

    string result;
    char buffer[4096];
    while(true)
    {
        char* str = fgets(buffer, sizeof(buffer), fp);
        if(str == nullptr) break;
        result += str;
    }

    // 就不考虑返回状态了
    int status = fclose(fp);

    return result;

}
```

我们把这个可调用模块传给`run`, 这样, 就可以把客户端输入的字符串视为命令, 并把执行结果返回出去.   为了防范某些危险指令, 在正式执行之前, 我们有一道安全检查, 拒绝危险的命令执行

```cpp
bool SecurityCheck(const string& str)
{
    static const vector<string> blacklists =
    {
        "rm", "sudo", "yum", "install", "uninstall",
        "mv", "cp",  "kill", "unlink"
    };

    for(const auto& e : blacklists)
    {
        if(string::npos != str.find(e))
            return true;
    }

    return false;
}
```

```cpp
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    wind::udpserver svr(std::stoi(*(argv + 1) + 1));
    svr.init();
    svr.run(ExcuteCommand);
    return 0;
}
```

接下来我们就测试一下

```shell
[wind@starry-sky UDP]$ make clean ; make
[wind@starry-sky UDP]$ ./udpserver -8888
[Info][2025-3-18 11:32:50]:: socket create success, sockfd: 4
[Info][2025-3-18 11:32:50]:: bind success
```

```shell
[wind@starry-sky UDP]$ ./udpclient 120.55.90.240 8888
socket create success, sockfd: 3
Pleace Enter@ ls
log.hpp
main.cc
makefile
test
udpclient
udpclient.cc
udpserver
udpserver.hpp

Pleace Enter@ rm test
Permission denie
Pleace Enter@ pwd
/home/wind/projects/Network/UDP

Pleace Enter@ whoami
wind

Pleace Enter@ ^C
[wind@starry-sky UDP]$
```

在这里我们说一个特殊的IP:`127.0.0.1`, 该IP可以在任一服务器上绑定, 它被称为本主机的本地环回地址, 它的特点是, 若一个进程绑定了该地址, 它就只能进行本地进程通信, 只不过通信数据会先到协议栈的最底层, 然后链路层不把数据帧传到网络中, 而是直接解包再往上传, 常用于客户端和服务端的本地网络测试.   下面我们就来用一用, 需要注意的是, 客户端和服务端都需要使用本地环回地址

```cpp
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    wind::udpserver svr(std::stoi(*(argv + 1) + 1), "127.0.0.1");
    svr.init();
    svr.run(ExcuteCommand);
    return 0;
}
```

```shell
[wind@starry-sky UDP]$ make clean ; make
[wind@starry-sky UDP]$ ./udpserver -8888
[Info][2025-3-18 11:51:56]:: socket create success, sockfd: 4
[Info][2025-3-18 11:51:56]:: bind success
```

```shell
[wind@starry-sky UDP]$ ./udpclient 127.0.0.1 8888
socket create success, sockfd: 3
Pleace Enter@ ls
log.hpp
main.cc
makefile
test
udpclient
udpclient.cc
udpserver
udpserver.hpp

Pleace Enter@ pwd
/home/wind/projects/Network/UDP

Pleace Enter@ whoami
wind

Pleace Enter@ sudo
Permission denie
Pleace Enter@ hello

Pleace Enter@
```

因为之前我们是当字符串传的, 所以客户端有个换行, 所以这里会多一个换行.

下面, 我们开始跨平台网络通信, 我们之前说过, 尽管线程实现, 进程管理, 文件系统什么的, 在各个系统上的实现方式不同, 但是, 无论是什么平台, 都必须遵守网络协议. 否则, 它就连不上网, 所以尽管系统不同, 网络的相应接口也要类似, 下面, 我们就把Linux上的客户端代码转成Windows风格, 让Windows与Linux通信起来.

我们方向不是Windows的, Windows的封装也向来复杂, 所以我们可以直接把Linux源代码扔给AI, 让它转成Windows风格.

```cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

using namespace std;

#define SOCKET_ERR       1
#define BUFFER_SIZE      1024
#define REMOTE_IP        "127.0.0.1"  // 默认IP地址（本地回环）
#define REMOTE_PORT      12345        // 默认端口号

// Link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed: " << iResult << endl;
        return 1;
    }

    // 使用宏定义的IP和端口
    string remote_ip = REMOTE_IP;
    unsigned short remote_port = REMOTE_PORT;

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET)
    {
        cout << "socket create error: " << WSAGetLastError() << endl;
        WSACleanup();
        exit(SOCKET_ERR);
    }
    cout << "socket create success, sockfd: " << sockfd << endl;

    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons(remote_port);
    if (inet_pton(AF_INET, remote_ip.c_str(), &remote.sin_addr) <= 0) {
        cout << "inet_pton error: " << WSAGetLastError() << endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    string message;
    char buffer[BUFFER_SIZE];
    while (true)
    {
        cout << "Please Enter@ ";
        getline(cin, message);

        int sendResult = sendto(sockfd, message.c_str(), message.size(), 0,
            (struct sockaddr*)&remote, sizeof(remote));
        if (sendResult == SOCKET_ERROR)
        {
            cout << "sendto error: " << WSAGetLastError() << endl;
        }

        struct sockaddr_in temp;
        int size = sizeof(temp);
        int len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
            (struct sockaddr*)&temp, &size);
        if (len == SOCKET_ERROR)
        {
            cout << "recvfrom error: " << WSAGetLastError() << endl;
            continue;
        }
        buffer[len] = '\0';
        cout << buffer;
    }

    // Cleanup
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
```

由于VS的命令行参数不太好用, 所以我们使用宏的方式来手动传参.  `#pragma comment(lib, "Ws2_32.lib")`是一个编译选项, 其实就是告诉编译器把`Ws2_32`链接一下, 然后之后我们就不管了.

在把宏作相应修改之后, 我们看到, 连接是正常的.

![image-20250318124402585](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250318124403040.png)

再在`Permission denie`后面加个换行, 效果会更好

![image-20250318124641978](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250318124642062.png)

```shell
[wind@starry-sky UDP]$ ./udpserver -8888
[Info][2025-3-18 12:46:0]:: socket create success, sockfd: 4
[Info][2025-3-18 12:46:0]:: bind success
[Info][2025-3-18 12:46:14]::sendto success
[Info][2025-3-18 12:46:16]::sendto success
[Info][2025-3-18 12:46:20]::sendto success
[Info][2025-3-18 12:46:23]::sendto success
sh: hello: command not found
[Info][2025-3-18 12:46:27]::sendto success
[Info][2025-3-18 12:46:36]::sendto success
```

还要注意一个问题, Windows和Linux的编码方式一般是不同的, Linux喜欢用UTF-8, Windows喜欢用GB, 所以不要输汉字.

------------

下面我们再把上面的代码改改, 改成一个命令行版的群聊. 由于我们是命令行, 所以我们就把客户端的IP作为账号, 创建一个哈希表, 用来记录所有与服务端通信过的客户端IP, 该行为就被视为进入群聊, 之后, 如果服务端收到任何信息, 都会把信息再传给群聊中的所有成员

```cpp
void run(const function__type& func)
{
    _isrunning = true;
    char buffer[BUFFER_SIZE];   // 约定通信内容为字符串
    while(true)
    {
        struct sockaddr_in remote;
        socklen_t size = sizeof(remote);
        ssize_t len = recvfrom(_sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&remote), reinterpret_cast<socklen_t*>(&size));
        if(len < 0)
        {
            log__(Warning, " recvfrom error: %s", strerror(errno));
            continue;
        }

        // 解析远端IP和端口, 交由外层处理
        uint16_t port = ntohs(remote.sin_port);
        std::string ip = inet_ntoa(remote.sin_addr);

         // 在用户层接收数据前, 数据的字节序已经被协议自动转换了, 所以这里不需要转
        buffer[len] = '\0';
        std::string temp(buffer);
        // 字符串加工,证明确实来过服务端
        std::string result = func(temp, ip, port);

        if(sendto(_sockfd, result.c_str(), result.size(), 0, reinterpret_cast<const sockaddr*>(&remote), static_cast<socklen_t>(sizeof(remote))) == -1)
        {
            log__(Warning, "sendto error: %s", strerror(errno));
        }
        // 避免干扰
        // log__(Info, "sendto success ");
    }
}
```

```cpp
string func(const string& str, const string& ip, const uint16_t& port)
{
    cout << "["<<ip<<":"<<port<<"]$ "<<str<<endl;;


    string result("server: ");
    result += str;
    result += "\n";
    return result;
}


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    wind::udpserver svr(std::stoi(*(argv + 1) + 1));
    svr.init();
    svr.run(func);
    return 0;
}
```

让我们看看打印效果如何

```shell
[wind@starry-sky UDP]$ make clean ; make
[wind@starry-sky UDP]$ ./udpserver -8888
[Info][2025-3-18 14:9:57]:: socket create success, sockfd: 4
[Info][2025-3-18 14:9:57]:: bind success
```

```shell
[whisper@VM-12-6-centos network_test]$ ./udpclient 120.55.90.240 8888
socket create success, sockfd: 3
Pleace Enter@
```

```shell
[wind@starry-sky UDP]$ ./udpclient 120.55.90.240 8888
socket create success, sockfd: 3
Pleace Enter@
```

```shell
socket create success, sockfd: 268
Please Enter@
```

```shell
[wind@starry-sky UDP]$ ./udpserver -8888
[Info][2025-3-18 14:9:57]:: socket create success, sockfd: 4
[Info][2025-3-18 14:9:57]:: bind success
[112.26.31.132:46837]$ This is Windows
[120.55.90.240:57664]$ This is Alibaba Cloud
[175.24.175.224:41302]$ This is Tencent Cloud
```

我们看到, 效果还是可以的, 这个测试成本比较高, 我这里就一个人, 只能一个人在三个平台上发.  需要注意的是, 这上面的IP可能不是我们的真实IP, 因为它们在局域网中, 所以上面的IP可能是局域网运营商的IP, 比如, 对于上面的`Windows`平台来说, 使用`ipconfig`查询到的实际IP为`192.168.248.1`, 而不是`112.26.31.132`.

下面, 我们就不用外层了, 因为把消息全部转发给群聊中的成员很明显要改一下底层, 都要发一遍, 我们就不分层了.

我们的思路很简单, 那就是服务端收到一个消息, 就看看是不是新上线的用户发的, 顺便确定一下该用户的账号信息, 然后向在群聊中的用户进行广播.

```cpp
namespace wind
{
    enum
    {
        SOCKET_ERR = 0,
        BIND_ERR= 1
    };

    class udpserver
    {
        typedef std::function<std::string(const std::string&, const std::string&, const uint16_t&)> function__type;
        typedef std::unordered_map<std::string, struct sockaddr_in> user_list_type;

        public:
        udpserver(const uint16_t& port = DEFAULT_PORT, const char* id = DEFAULT_ADDRESS) :_sockfd(0), _port(port), _id(id), _isrunning(false) {}

        void init()
        {
            _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sockfd < 0)
            {
                log__(Fatal, " socket create error: %s", strerror(errno));
                exit(SOCKET_ERR);
            }
            log__(Info, " socket create success, sockfd: %d", _sockfd);

            struct sockaddr_in local;  // 服务端本地的套接字
            bzero(&local, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            local.sin_addr.s_addr = inet_addr(_id.c_str());
            // local.sin_addr.s_addr = inet_addr(INADDR_ANY);  // 因为是零, 所以其实转不转都一样
            if(bind(_sockfd, reinterpret_cast<const struct sockaddr *>(&local), static_cast<socklen_t>(sizeof(local))) != 0)
            {
                log__(Fatal, " bind error: %s", strerror(errno));
                exit(BIND_ERR);
            }
            log__(Info, " bind success");
        }

        void run()
        {
            _isrunning = true;
            char buffer[BUFFER_SIZE];   // 约定通信内容为字符串
            while(true)
            {
                struct sockaddr_in remote;
                socklen_t size = sizeof(remote);
                ssize_t len = recvfrom(_sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&remote), reinterpret_cast<socklen_t*>(&size));
                if(len < 0)
                {
                    log__(Warning, " recvfrom error: %s", strerror(errno));
                    continue;
                }

                std::string who = RegisterNewUser(remote);

                buffer[len] = 0;
                _message = buffer;

                Broadcast(who);

            }
        }

        private:
        std::string RegisterNewUser(const struct sockaddr_in &user)
        {
            std::string ip = inet_ntoa(user.sin_addr);
            if (_online_user.find(ip) == _online_user.end())
            {
                _message = ip;
                _message += " joins group chat";
                log__(Info, _message.c_str());
                _online_user.emplace(ip, user);
                // Broadcast();
            }
            return std::move(ip);
        }

        void Broadcast(std::string& who)
        {
            _message += "\n";

            std::stringstream oss;
            oss << "["<<who<<":"<<ntohs(_online_user[who].sin_port)<<"]$ "<<_message;
            _message = oss.str();
            log__(Info, _message.c_str());

            // ip充当账号信息
            // [ip, user]是C++17语法
            for(const auto& [ip, user] : _online_user)
            {
                log__(Info, "正在向%s发送消息", ip.c_str());
                if(sendto(_sockfd, _message.c_str(), _message.size(), 0, reinterpret_cast<const sockaddr*>(&user), static_cast<socklen_t>(sizeof(user))) == -1)
                {
                    log__(Warning, "sendto error: %s", strerror(errno));
                }
            }
        }

        private:
        int _sockfd;       // 套接字句柄
        in_port_t _port;   // 端口号
        std::string _id;   // 点分十进制IP
        bool _isrunning;
        std::string _message;
        user_list_type _online_user;
    };
}
```

让我们看看效果:

```shell
[wind@starry-sky UDP]$ make clean ; make
[wind@starry-sky UDP]$ ./udpserver -8888
[Info][2025-3-18 16:9:19]:: socket create success, sockfd: 4
[Info][2025-3-18 16:9:19]:: bind success
[Info][2025-3-18 16:9:26]::112.26.31.132 joins group chat
[Info][2025-3-18 16:9:26]::[112.26.31.132:46857]$ haha

[Info][2025-3-18 16:9:26]::正在向112.26.31.132发送消息
[Info][2025-3-18 16:9:41]::120.55.90.240 joins group chat
[Info][2025-3-18 16:9:41]::[120.55.90.240:44486]$ lsls

[Info][2025-3-18 16:9:41]::正在向120.55.90.240发送消息
[Info][2025-3-18 16:9:41]::正在向112.26.31.132发送消息
[Info][2025-3-18 16:9:47]::175.24.175.224 joins group chat
[Info][2025-3-18 16:9:47]::[175.24.175.224:53114]$ nihao

[Info][2025-3-18 16:9:47]::正在向175.24.175.224发送消息
[Info][2025-3-18 16:9:47]::正在向120.55.90.240发送消息
[Info][2025-3-18 16:9:47]::正在向112.26.31.132发送消息
[Info][2025-3-18 16:14:53]::[112.26.31.132:46857]$ zheli

[Info][2025-3-18 16:14:53]::正在向175.24.175.224发送消息
[Info][2025-3-18 16:14:53]::正在向120.55.90.240发送消息
[Info][2025-3-18 16:14:53]::正在向112.26.31.132发送消息
```

我们看到服务端的效果其实还行, 但是对于客户端来说, 就不怎么好了. 这是因为客户端是一个执行流, 它必须先输入, 才能再接收, 所以就会出现这种情况: 客户端一直阻塞, 等待用户输入信息, 所以它收不到自服务端发来的消息, 很明显, 我们要对客户端进行大改了, 要改成多线程, 一个负责等待用户输入, 另一个负责接收消息. 需要说明的是, 单就读写来说, 套接字是线程安全的, 可以一边读, 一边写.

具体的代码还是很简单的, 直接把原先内容拆成两份即可.

```cpp
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <string>

using namespace std;

#define SOCKET_ERR       1
#define BUFFER_SIZE      1024

struct push_args
{
    int _fd;
    struct sockaddr_in _server;
};

void* push(void* args)
{
    push_args* p_ = reinterpret_cast<push_args*>(args);
    int& sockfd = p_->_fd;
    struct sockaddr_in& remote = p_->_server;
    string message;

    while(true)
    {
        cout << "Pleace Enter@ ";
        getline(cin, message);

        if(sendto(sockfd, message.c_str(), message.size(), 0, reinterpret_cast<const sockaddr*>(&remote), static_cast<socklen_t>(sizeof(remote))) == -1)
        {
            cout << " sendto error: " << strerror(errno);
        }
    }
    return nullptr;
}

void* pull(void* args)
{
    int& sockfd = *(reinterpret_cast<int*>(args));
    char buffer[BUFFER_SIZE];
    while(true)
    {
        struct sockaddr_in temp;
        socklen_t size = sizeof(temp);
        ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&temp), reinterpret_cast<socklen_t *>(&size));
        if (len < 0)
        {
            cout << " recvfrom error: " << strerror(errno) << endl;
            continue;
        }
        buffer[len] = '\0';
        cerr << buffer;
    }
    return nullptr;
}


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    string remote_ip(argv[1]);
    in_port_t remote_port = stoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        cout << "socket create error: "<< strerror(errno)<<endl;
        exit(SOCKET_ERR);
    }
    cout << "socket create success, sockfd: "<<sockfd<<endl;

    // 交由系统自动绑定

    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons(remote_port);
    remote.sin_addr.s_addr = inet_addr(remote_ip.c_str());

    struct push_args i;
    i._fd = sockfd;
    i._server = remote;

    pthread_t in, out;
    pthread_create(&out, nullptr, push, &i);
    pthread_create(&in, nullptr, pull, &sockfd);

    pthread_join(in, nullptr);
    pthread_join(out, nullptr);


    close(sockfd);


    return 0;
}
```

服务端代码又稍微改了一下

```cpp
void Broadcast(std::string& who)
{
    _message += "\n";

    std::stringstream oss;
    oss << "["<<who<<":"<<ntohs(_online_user[who].sin_port)<<"]$ "<<_message;
    _message = oss.str();
    // log__(Info, _message.c_str());

    // ip充当账号信息
    // [ip, user]是C++17语法
    for(const auto& [ip, user] : _online_user)
    {
        if(who == ip) continue;
        log__(Info, "正在向%s发送消息", ip.c_str());
        if(sendto(_sockfd, _message.c_str(), _message.size(), 0, reinterpret_cast<const sockaddr*>(&user), static_cast<socklen_t>(sizeof(user))) == -1)
        {
            log__(Warning, "sendto error: %s", strerror(errno));
        }
    }
}
```

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250318182115199.mp4"></video>

不过这里有个问题, 那就是输入输出混在一起, 如果有图形化界面, 那我们可以把界面分成两个区域, 一是发送区, 二是接收区, 这样看起来就会更好, 但是, 我们这里开不了图形化界面, 所以我们可以使用文件重定向的方式把打印信息输出到另一个终端上.

Centos系统可以通过`ls -l /dev/pts/`查看终端文件

```shell
[wind@starry-sky UDP]$ ls -l /dev/pts/
total 0
crw--w---- 1 wind tty  136,  0 Mar 18 08:37 0
crw--w---- 1 wind tty  136, 10 Mar 18 16:32 10
crw--w---- 1 wind tty  136, 11 Mar 18 19:00 11
crw--w---- 1 wind tty  136, 12 Mar 18 18:58 12
crw--w---- 1 wind tty  136, 13 Mar 18 18:59 13
crw--w---- 1 wind tty  136, 14 Mar 18 18:58 14
crw--w---- 1 wind tty  136,  5 Mar 18 18:30 5
crw--w---- 1 wind tty  136,  7 Mar 18 08:37 7
crw--w---- 1 wind tty  136,  8 Mar 18 18:29 8
crw--w---- 1 wind tty  136,  9 Mar 18 16:32 9
c--------- 1 root root   5,  2 Sep 12  2024 ptmx
[wind@starry-sky UDP]$
```

```shell
[whisper@VM-12-6-centos network_test]$ ls -l /dev/pts/
total 0
crw--w---- 1 whisper tty  136, 0 Mar 18 19:00 0
crw--w---- 1 whisper tty  136, 1 Mar 18 18:29 1
crw--w---- 1 whisper tty  136, 2 Mar 18 18:30 2
crw--w---- 1 whisper tty  136, 3 Mar 18 19:00 3
c--------- 1 root    root   5, 2 Mar 16 11:14 ptmx
[whisper@VM-12-6-centos network_test]$
```

我们可以通过`tty`来获取终端的编号

关于重定向, 我们有两种实现方式, 一是调动系统接口手动实现重定向, 而是直接运行程序时使用重定向符, 重定向的原理我们之前也说过, 在此不作赘述, 你或许可以看到我是用`cerr`作输出的, 所以接下来我们就要把标准错误流重定向到终端文件上.

效果如下:

![image-20250318194409197](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250318194409482.png)

服务端是左下角终端启动的, 但别人发送的内容都会来到左上角的终端, 左下角只发挥着读取作用.

但其实上, 我们也犯不着用代码实现重定向, 只要启动进程的时候用输出重定向就行了

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250318195847188.mp4"></video>

------------

最后, 我们再来说些其它东西

点分十进制转`uint32_t`的接口有

```c
#include <arpa/inet.h>
int inet_aton(const char *cp, struct in_addr *inp);
in_addr_t inet_addr(const char *cp);
int inet_pton(int af, const char *src, void *dst);
```

`uint32_t`转点分十进制有

```c
char *inet_ntoa(struct in_addr in);
const char *inet_ntop(int af, const void *src,
                      char *dst, socklen_t size);
```

需要注意的是`inet_ntoa`返回的是`char*`, 这个`char*`空间在静态区, 所以不用担心内存泄露, 不过, 该接口据说是线程不安全的, 并发条件下可能会出现问题. `inet_ntop`理论上更安全, 缓冲区由用户决定.

## TCP网络通信

我们先把套路写好

```cpp
#pragma once

#include "log.hpp"
#include <string>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_PORT     8888
#define DEFAULT_ADDRESS  "0.0.0.0"

namespace wind
{
    enum{
        PARAMERROR = 1,
        SOCKERROR = 2,
        BINDERROR = 3,
        LISTENERROR = 4
    };

    class tcpserver
    {
        typedef std::string string;
        public:
        tcpserver(const int16_t& port = DEFAULT_PORT, const char* ip = DEFAULT_ADDRESS) :_port(port), _ip(ip){}
        ~tcpserver() {close(_sockfd);}

        void init()
        {
            socket_init();

            struct sockaddr_in local;
            sockaddr_in__init(local);

            bind_port(local);
        }

        void run()
        {

        }

        private:
        inline void socket_init()
        {
            _sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(_sockfd < 0)
            {
                _log(Fatal, "socket create error: %s", strerror(errno));
                exit(SOCKERROR);
            }
            _log(Info, "socket create success, sockfd: %d", _sockfd);
        }

        inline void sockaddr_in__init(sockaddr_in& local)
        {
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            inet_aton(_ip.c_str(), &(local.sin_addr));
            _log(Info, "socket init success");
        }

        inline void bind_port(const sockaddr_in& local)
        {
            if(bind(_sockfd, reinterpret_cast<const sockaddr*>(&local), static_cast<socklen_t>(sizeof(local))) != 0)
            {
                _log(Fatal, "bind error: %s", strerror(errno));
                exit(BINDERROR);
            }
            _log(Info, "bind success");
        }

        private:
        int _sockfd;
        string _ip;
        int16_t _port;
        static Log _log;
    };
}

wind::Log wind::tcpserver::_log;
```

接下来就有些不同了, TCP是面向连接的, 在未与客户端建立连接前, 服务端要一直处于一种等待状态, 这种等待状态的具体表现是要将套接字设置为监听状态

```c
#include <sys/types.h>
#include <sys/socket.h>

int listen(int sockfd, int backlog);
```

 `listen`可以将一个套接字设置为监听状态, 当一个套接字进入监听状态后, 就可以通过它获取来自客户端的新连接.

第一个参数`sockfd`就是我们所创建的套接字, 第二个参数和TCP原理有关, 我们暂且不讨论 姑且默认为5, 成功时返回0, 失败返回-1, 错误码被设置, 至此, init就写完了

```cpp
void init()
{
    socket_init();

    struct sockaddr_in local;
    sockaddr_in__init(local);

    bind_port(local);

    start_listening();
}


inline void start_listening()
{
    if(listen(_sockfd, DEFAULT_BACKLOG) == -1)
    {
        _log(Fatal, "listen error: %s", strerror(errno));
        exit(LISTENERROR);
    }
    _log(Info, "start listening");
}
```

对于run, 我们先不写实际内容, 而是单纯打印一下, 然后运行一下程序, 看看init有没有潜在的问题.

```cpp
void run()
{
    while(true)
    {
        _log(Info, "tcpserver is running....");
        sleep(1);
    }
}
```

```cpp
#include"tcpserver.hpp"
#include<iostream>

using namespace std;

void print_help() {
    cout << "Usage: ./tcpserver -<port_number>" << endl;
    cout << "Options:" << endl;
    cout << "  -<port_number>    Specify the port number (1-65535)" << endl;
    cout << "Example:" << endl;
    cout << "  ./tcpserver -8888" << endl;
}

// ! 存在平台差异, Windows不适用
int16_t get_port(char **argv)
{
    int i = 0;
    for (; argv[i]; ++i);

    if (i != 2)
    {
        print_help();
        exit(wind::PARAMERROR);
    }

    int temp = stoi(argv[1] + 1);
    if (temp < 1 || temp > 65535)
    {
        print_help();
        exit(wind::PARAMERROR);
    }

    return static_cast<int16_t>(temp);
}

int main(int argc, char* argv[])
{
    wind::tcpserver tcp_svr(get_port(argv));
    tcp_svr.init();
    tcp_svr.run();
    return 0;
}
```

编译运行一下

```shell
[wind@starry-sky TCP]$ make clean ; make
[wind@starry-sky TCP]$ ./tcpserver
Usage: ./tcpserver -<port_number>
Options:
  -<port_number>    Specify the port number (1-65535)
Example:
  ./tcpserver -8888
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-20 11:23:47]::socket create success, sockfd: 4
[Info][2025-3-20 11:23:47]::socket init success
[Info][2025-3-20 11:23:47]::bind success
[Info][2025-3-20 11:23:47]::start listening
[Info][2025-3-20 11:23:47]::tcpserver is running....
[Info][2025-3-20 11:23:48]::tcpserver is running....
[Info][2025-3-20 11:23:49]::tcpserver is running....
```

```shell
[wind@starry-sky TCP]$ netstat -nltp
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
tcp        0      0 127.0.0.1:39593         0.0.0.0:*               LISTEN      12245/code-ddc367ed
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      18431/./tcpserver
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -
tcp6       0      0 :::111                  :::*                    LISTEN      -
tcp6       0      0 :::22                   :::*                    LISTEN      -
tcp6       0      0 ::1:25                  :::*                    LISTEN      -
[wind@starry-sky TCP]$
```

好的, 看来效果还不错.

下面我们开始写`run`.

接下来我们要了解一下`accept`接口, 它的核心作用就是从监听套接字的已连接队列中接受一个客户端连接，并返回一个新的套接字描述符，用于与该客户端通信。通过这个套接字，可以使用读写操作（如 read 和 write）与客户端交互。

监听套接字就是之前我们设置为监听状态的套接字, "从已连接队列中"涉及到原理, 本小节面向应用, 我们暂且不谈.

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

第一个参数就是我们的监听套接字, 剩下两个就是客户端的`sockaddr_in`, 和之前的UDP类似, 成功时, 就会返回一个客户端连接, 以文件描述符的形式.为了分辨监听套接字文件描述符和使用`accept`接收到的文件描述符, 我们需要把之前的`_sockfd`改成`_listeningSockfd`

为了理解监听套接字和accept返回的新套接字, 我们可以说个故事. 有些商家可能会在商店门口安排专人拉客, 为了方便表述, 我们把这位负责拉客的员工叫做张三, 张三只负责拉客, 拉到客就往店里送, 送进去就继续拉客, 路人拒绝, 他就换一个人拉, 他只负责拉客, 不负责顾客进店之后的具体服务,   顾客进店后的具体服务由其他人负责, 比如, 我们就叫这个其他人叫做李四, 在这个故事中, 张三扮演的就是监听套接字的功能, 而李四就是accept返回的套接字.

```cpp
inline int accept_connection(int& sockfd, struct sockaddr_in& client)
{
    socklen_t len = static_cast<socklen_t>(sizeof(client));
    sockfd = accept(_listeningSockfd, reinterpret_cast<struct sockaddr*>(&client), &len);
    if(sockfd < 0)
    {
        _log(Warning, "accert error: %s", strerror(errno));
        return -1;
    }
    _log(Info, "accept a new connection, sockfd: %d", sockfd);
    return 0;
}
```

拉不到客并不是致命问题, 所以Warning, continue下一个就行

```cpp
void run()
{
    _log(Info, "tcpserver is running....");
    while (true)
    {
        int sockfd = 0;
        struct sockaddr_in client;
        if (accept_connection(sockfd, client) == -1)
            continue;


    }
}
```

接下来我们再测试一下, 不过我们并没有写客户端, 这里我们引入一个Linux自带的网络测试工具, telnet, 它默认使用TCP, 可以临时充当客户端检验服务端是否可以被正常连接.

```shell
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-20 12:48:25]::socket create success, sockfd: 4
[Info][2025-3-20 12:48:25]::socket init success
[Info][2025-3-20 12:48:25]::bind success
[Info][2025-3-20 12:48:25]::start listening
[Info][2025-3-20 12:48:25]::tcpserver is running....
```

```shell
[wind@starry-sky TCP]$ telnet 127.0.0.1 8888  # 远程登录  本地环回  8888
Trying 127.0.0.1...
Connected to 127.0.0.1.                       # 连接成功
Escape character is '^]'.
^]                                            # 按下组合键 `Ctrl` + `]`  进入命令模式
telnet> quit                                  # 退出
Connection closed.
[wind@starry-sky TCP]$
```

```shell
[Info][2025-3-20 12:48:32]::accept a new connection, sockfd: 5
^C
[wind@starry-sky TCP]$
```

接下来我们把新获得的套接字内容解析一下.  用于数据显示时的身份区分.

```cpp
void run()
{
    _log(Info, "tcpserver is running....");
    while (true)
    {
        int sockfd = 0;
        struct sockaddr_in client;
        if (accept_connection(sockfd, client) == -1)
            continue;

        uint16_t port = ntohs(client.sin_port);
        char ipbuff[32];
        inet_ntop(AF_INET, &client.sin_addr, ipbuff, sizeof(ipbuff));

        service(sockfd, ipbuff, port);

    }
}

inline void service(int sockfd, const char* ip, uint16_t port)
{
    char buff[BUFFER_SIZE];
    while(true)
    {
        ssize_t len = read(sockfd, buff, sizeof(buff) - 1);
        if(len > 0)
        {
            buff[len] = 0;
            std::cout<<"client say# "<<buff<<std::endl;

            std::string echo("tcpserver echo#");
            echo += buff;

            write(sockfd, echo.c_str(), echo.size());
        }
    }
}
```

现在服务端已经有了基本的交互功能, 我们运行测试一下.

```shell
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-20 15:58:48]::socket create success, sockfd: 4
[Info][2025-3-20 15:58:48]::socket init success
[Info][2025-3-20 15:58:48]::bind success
[Info][2025-3-20 15:58:48]::start listening
[Info][2025-3-20 15:58:48]::tcpserver is running....
[Info][2025-3-20 15:59:7]::accept a new connection, sockfd: 5
client say# jc

client say# aaaa

client say# dasc

client say# wvcws

client say# vrvr

client say# vreve

client say# vrevbe

client say# vrevb

client say# vbrevb

client say# vberbv

client say# bvebv

^C
[wind@starry-sky TCP]$
```

```shell
[wind@starry-sky TCP]$ telnet 127.0.0.1 8888
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
^]
telnet>                  # 回车, 进入普通模式进行通信
jc
tcpserver echo#jc
aaaa
tcpserver echo#aaaa
dasc
tcpserver echo#dasc
wvcws
tcpserver echo#wvcws
vrvr
tcpserver echo#vrvr
vreve
tcpserver echo#vreve
vrevbe
tcpserver echo#vrevbe
vrevb
tcpserver echo#vrevb
vbrevb
tcpserver echo#vbrevb
vberbv
tcpserver echo#vberbv
bvebv
tcpserver echo#bvebv
^]
telnet> quit
Connection closed.
[wind@starry-sky TCP]$
```

接下来写客户端,

首先创建套接字, 差错处理, 不需要亲自绑定

```cpp
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        cout << "socket error: "<<strerror(errno)<<endl;
        exit(1);
    }

    // 客户端不由我们亲自绑定



    close(sockfd);

    return 0;
}
```

接下来我们来了解一下`connect`, 它用来与服务端建立连接, 成功返回零, 失败返回-1, 自动绑定就发生在`connect`里.

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr,
            socklen_t addrlen);
```

连接成功之后, 就可以直接通过套接字描述符与服务端通信.

```cpp
#include <string>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

struct sockaddr_in args_parsing(char** argv)
{
    int i = 0;
    for(; argv[i]; ++i);
    if(i != 3)
    {
        cout << "parsing error"<<endl;
        exit(1);
    }
    sockaddr_in result;
    memset(&result, 0, sizeof(result));
    result.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &result.sin_addr);
    result.sin_port = htons(stoi(argv[2]));
    return result;
}

int main(int argc, char* argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        cout << "socket error: "<<strerror(errno)<<endl;
        exit(1);
    }

    // 客户端不由我们亲自绑定

    auto server = args_parsing(argv);

    // 与服务端建立连接
    if(connect(sockfd, reinterpret_cast<const struct sockaddr*>(&server), static_cast<socklen_t>(sizeof(server))) != 0)
        exit(1);

    cout<<"connect success"<<endl;

    string message;
    while(true)
    {
        cout << "Please Enter# ";
        getline(cin, message);

        write(sockfd, message.c_str(), message.size());



        char buff[4096];
        ssize_t len = read(sockfd, buff, sizeof(buff) - 1);
        if(len > 0)
        {
            buff[len] = 0;
            cout<<buff<<endl;
        }

    }

    close(sockfd);

    return 0;
}
```

运行一下

```shell
[wind@starry-sky TCP]$ make clean ; make
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-20 17:17:15]::socket create success, sockfd: 4
[Info][2025-3-20 17:17:15]::socket init success
[Info][2025-3-20 17:17:15]::bind success
[Info][2025-3-20 17:17:15]::start listening
[Info][2025-3-20 17:17:15]::tcpserver is running....
[Info][2025-3-20 17:17:46]::accept a new connection, sockfd: 5
client say# hello
client say# aaaa
client say# sss
client say# ddd
client say# fff
client say# dasdc
client say# cac

```

```shell
[wind@starry-sky TCP]$ ./tcpclient 127.0.0.1 8888
connect success
Please Enter# hello
tcpserver echo#hello
Please Enter# aaaa
tcpserver echo#aaaa
Please Enter# sss
tcpserver echo#sss
Please Enter# ddd
tcpserver echo#ddd
Please Enter# fff
tcpserver echo#fff
Please Enter# dasdc
tcpserver echo#dasdc
Please Enter# cac
tcpserver echo#cac
Please Enter#
```

下面, 我们就要对双方代码进行优化了.

对于客户端来说, 当网络信号不佳时, 连接可能会中断, 此时, 一般来说要进行重连, 代码上就是反复重复执行`connect`, 对于服务端来说, 连接中断而导致套接字描述符失效也是需要考虑的差错处理. 服务端对于这种情况的处理也很简单, 不就是描述符失效了吗, 不就是读不到东西了吗, 那就关闭这个文件, 跳出循环, 去接收下一个连接.

```cpp
{
    char buff[BUFFER_SIZE];
    while(true)
    {
        ssize_t len = read(sockfd, buff, sizeof(buff) - 1);
        if(len > 0)
        {
            buff[len] = 0;
            std::cout<<"client say# "<<buff<<std::endl;

            std::string echo("tcpserver echo# ");
            echo += buff;

            write(sockfd, echo.c_str(), echo.size());
        }
        else if(len == 0)
        {
            _log(Info, "user quit, close sockfd: %d", sockfd);
            close(sockfd);
            break;
        }
        else
        {
            _log(Warning, "read error: %s", strerror(errno));
            close(sockfd);
            break;
        }
    }
}
```

重新运行

```shell
[wind@starry-sky TCP]$ make clean ; make
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-20 17:43:40]::socket create success, sockfd: 4
[Info][2025-3-20 17:43:40]::socket init success
[Info][2025-3-20 17:43:40]::bind success
[Info][2025-3-20 17:43:40]::start listening
[Info][2025-3-20 17:43:40]::tcpserver is running....
[Info][2025-3-20 17:43:44]::accept a new connection, sockfd: 5
client say# lihao
client say# 你好
client say# this
client say# ax
[Info][2025-3-20 17:44:14]::user quit, close sockfd: 5
[Info][2025-3-20 17:44:28]::accept a new connection, sockfd: 5
[Info][2025-3-20 17:44:37]::user quit, close sockfd: 5
[Info][2025-3-20 17:44:47]::accept a new connection, sockfd: 5
client say# kmkc

client say# kackds

client say# klsadmc

[Info][2025-3-20 17:45:3]::user quit, close sockfd: 5
^C
[wind@starry-sky TCP]$
```

```shell
[wind@starry-sky TCP]$ ./tcpclient 127.0.0.1 8888
connect success
Please Enter# ^C
[wind@starry-sky TCP]$ telnet 127.0.0.1 8888
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
^]
telnet>
kmkc
tcpserver echo# kmkc
kackds
tcpserver echo# kackds
klsadmc
tcpserver echo# klsadmc
^]
telnet> quit
Connection closed.
[wind@starry-sky TCP]$
```

我们看到, 可以正常的结束连接并接收新连接.    但服务端的代码很明显有一些问题, 那就是它只支持单客户端串行访问, 很明显, 这样的服务端是几乎不能使用的, 服务器很明显要对客户请求及时处理的, 所以, 下面我们的优化方向, 就是使用各种各样的方式实现客户端的多执行流运行, 从而使得它可以支持客户端并发访问.

我们先用多进程.

```cpp
void run()
{
    _log(Info, "tcpserver is running....");
    while (true)
    {
        int sockfd = 0;
        struct sockaddr_in client;
        if (accept_connection(sockfd, client) == -1)
            continue;

        uint16_t port = ntohs(client.sin_port);
        char ipbuff[32];
        inet_ntop(AF_INET, &client.sin_addr, ipbuff, sizeof(ipbuff));

        // service(sockfd, ipbuff, port);    // 单进程版

        service2(sockfd, ipbuff, port);      // 多进程版

    }
}

inline void service2(int sockfd, const char* ip, uint16_t port)
{
    pid_t id = fork();
    if(id == 0)
    {
        close(_listeningSockfd);
        service(sockfd, ip, port);
        exit(0);
    }
    else if(id > 0)
    {
        close(sockfd);
        waitpid(id, nullptr, 0);
    }
    else
    {
        _log(Fatal, "fork error: %s", strerror(errno));
        exit(FROKERROR);
    }
}

```

对于这份代码来说, 理论上是说的通的, 子进程会继承父进程的文件列表, 而子进程本身只负责与客户端交互, 所以它也用不着`_listeningSockfd`, 可以直接关掉, 而对于父进程来说, `sockfd`已经被子进程继承下去了, 所以自己也用不着了, 关闭也没有问题, 不关父进程打开的文件会越来越多, 所以是必须关的,  为了防止子进程僵死, 需要对子进程进行等待, 从而回收子进程资源.

但是, 很明显, 如果父进程采用阻塞等待的方式回收进程资源, 它自己就会阻塞住, 所以到头来, 服务端的效果还是串行访问的, 所以我们需要再找找法子, 想办法让子进程不使用阻塞的方式来进行回收,    有什么方法呢? 很明显, 信号等待很不错, 但我们这里就不用信号等待了, 换一个新的方法, 这种方法算是一种代码技巧, 主进程还是采用阻塞等待的方式回收子进程, 但是, 子进程在执行之后, 立刻再创建一个新的子进程, 然后自己退出, 这样, 子进程就能立刻被父进程回收, 就不会有阻塞, 而新创建的孙子进程, 由于它的父进程没了, 所以会变为孤儿进程从而被系统自动收养, 它退出了, 系统就会自动回收资源.

```cpp
inline void service2(int sockfd, const char* ip, uint16_t port)
{
    pid_t id = fork();
    if(id == 0)
    {
        close(_listeningSockfd);
        if(fork() > 0) exit(0);
        service(sockfd, ip, port);
        exit(0);
    }
    else if(id > 0)
    {
        close(sockfd);
        waitpid(id, nullptr, 0);
    }
    else
    {
        _log(Fatal, "fork error: %s", strerror(errno));
        exit(FROKERROR);
    }
}
```

来测试一下

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250320201619811.mp4"></video>

好的, 但多进程的版本还是有很大的缺点, 占用资源太多了,  而且来请求之后再创建进程也太慢了, 因此, 很明显, 下一版本我们要使用多线程了.

```cpp
void run()
{
    _log(Info, "tcpserver is running....");
    while (true)
    {
        int sockfd = 0;
        struct sockaddr_in client;
        if (accept_connection(sockfd, client) == -1)
            continue;

        uint16_t port = ntohs(client.sin_port);
        char ipbuff[32];
        inet_ntop(AF_INET, &client.sin_addr, ipbuff, sizeof(ipbuff));

        // service(sockfd, ipbuff, port);    // 单进程版

        // service2(sockfd, ipbuff, port);   // 多进程版

        service3(sockfd, ipbuff, port);   // 多线程基础版

    }
}

```

```cpp
struct Args
{
    int fd_;
    string ip_;
    uint16_t port_;
    tcpserver* obj_;
};

inline void service3(const int& sockfd, const char* ip, const uint16_t& port)
{
    auto args_ = new Args;
    args_->fd_ = sockfd;
    args_->ip_ = ip;
    args_->port_ = port;
    args_->obj_ = this;

    pthread_t t;
    pthread_create(&t, nullptr, interaction, args_);
}

static void* interaction(void* args_)
{
    pthread_detach(pthread_self());
    std::shared_ptr<Args> args(reinterpret_cast<Args*>(args_));
    args->obj_->service(args->fd_, args->ip_.c_str(), args->port_);
    return nullptr;
}
```

```shell
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-21 9:12:23]::socket create success, sockfd: 4
[Info][2025-3-21 9:12:23]::socket init success
[Info][2025-3-21 9:12:23]::bind success
[Info][2025-3-21 9:12:23]::start listening
[Info][2025-3-21 9:12:23]::tcpserver is running....
[Info][2025-3-21 9:13:26]::accept a new connection, sockfd: 5
[Info][2025-3-21 9:13:35]::accept a new connection, sockfd: 6
client say# cec
client say# cec
client say# cecd
client say# ceqc
client say# creqc
client say# cerc
[Info][2025-3-21 9:16:11]::accept a new connection, sockfd: 7
client say# r
client say# b
client say# v
client say# t
client say# e
client say# y
[Info][2025-3-21 9:16:53]::user quit, close sockfd: 7
[Info][2025-3-21 9:17:1]::user quit, close sockfd: 5
[Info][2025-3-21 9:17:3]::user quit, close sockfd: 6
^C
[wind@starry-sky TCP]$
```

我们看到, 效果还是可以的, 可以支持并行访问.

但这份代码还有很大的改进空间.  比如, 线程是用户请求的时候才创建的, 线程的创建数目是不可控的, 当面临大量用户请求时, 一方面, 大量的线程创建耗费时间, 另一方面, 线程数目过多对设备也是一种负担.        另外还有一点就是, 一般来说, 所谓的网络服务都是短服务, 所谓短服务就是它只把服务运行一次, 然后就退出, 而不会像之前我们的代码那样来个死循环,   我们之前写死循环的原因还是因为默认这是一个人与人之间通信的程序, 但实际上, 更多的情况应该是客户端发来请求, 服务端进行计算, 然后把结果发回客户端, 所以算一遍就行了

因此, 我们下一步的优化方案是使用线程池, 线程池有着固定的线程数目, 可以应对短期的大量请求, 而且线程只运行一遍服务, 可以很快跑完这个客户的服务, 然后立刻跑下一个用户的服务或者是阻塞.

```cpp
void init()
{
    // 线程池已经改为单例模式
    threadPool::getInstance().start();

    socket_init();

    struct sockaddr_in local;
    sockaddr_in__init(local);

    bind_port(local);

    start_listening();
}

void run()
{
    _log(Info, "tcpserver is running....");
    while (true)
    {
        int sockfd = 0;
        struct sockaddr_in client;
        if (accept_connection(sockfd, client) == -1)
            continue;

        uint16_t port = ntohs(client.sin_port);
        char ipbuff[32];
        inet_ntop(AF_INET, &client.sin_addr, ipbuff, sizeof(ipbuff));

        // service(sockfd, ipbuff, port);    // 单进程版

        // service2(sockfd, ipbuff, port);   // 多进程版

        // service3(sockfd, ipbuff, port);   // 多线程基础版

        service4(sockfd, ipbuff, port);   // 线程池

    }
}

inline void service4(const int& sockfd, const char* ip, const uint16_t& port)
{
    threadPool::getInstance().push(sockfd, ip, port);
}
```

 ```cpp
 #pragma once

 #include "log.hpp"
 #include <string>
 #include <unistd.h>
 #include <iostream>

 #define BUFFER_SIZE      1024


 namespace wind
 {
     class task
     {
         typedef std::string string;

         public:
         task() {}
         task(const int &sockfd, const char *ip, const uint16_t &port) : _sockfd(sockfd), _ip(ip), _port(port) {}
         void operator()()
         {
             char buff[BUFFER_SIZE];
             ssize_t len = read(_sockfd, buff, sizeof(buff) - 1);
             if (len > 0)
             {
                 buff[len] = 0;
                 std::cout << "client say# " << buff << std::endl;

                 std::string echo("tcpserver echo# ");
                 echo += buff;

                 write(_sockfd, echo.c_str(), echo.size());
             }

             _log(Info, "user quit, close sockfd: %d", _sockfd);
             close(_sockfd);
         }

         private:
         int _sockfd;
         string _ip;
         uint16_t _port;
         static Log _log;   // Log之前不是单例, 现在也不好改,改了前面不兼容 所以就用这种方式了
     };

 }

 wind::Log wind::task::_log;

 ```

```cpp
#pragma once

#include <pthread.h>
#include <string>
#include <vector>
#include <queue>

struct thread
{
    pthread_t tid;
    // 线程的其它属性
    std::string name;
};

template <class task_>
    class thread_pool
    {
        typedef thread_pool<task_> self;

        public:
        static self& getInstance()
        {
            static self inst;
            return inst;
        }

        private:
        thread_pool(int num = 5) : _threads(num)
        {
            pthread_mutex_init(&_mutex, nullptr);
            pthread_cond_init(&_cond, nullptr);
        }

        ~thread_pool()
        {
            pthread_mutex_destroy(&_mutex);
            pthread_cond_destroy(&_cond);
        }

        public:
        void start()
        {
            for (int i = 0; i < _threads.size(); ++i)
            {
                _threads[i].name = "thread-" + std::to_string(i + 1);
                pthread_create(&(_threads[i].tid), nullptr, behavior, this);
            }
        }

        template <class... Args>
            void push(Args &&...args)
        {
            pthread_mutex_lock(&_mutex);
            _tasks.emplace(std::forward<Args>(args)...);
            pthread_cond_signal(&_cond);
            pthread_mutex_unlock(&_mutex);
        }

        private:
        static void *behavior(void *p_)
        {
            thread_pool<task_> *p = static_cast<thread_pool<task_> *>(p_);
            while (true)
            {
                pthread_mutex_lock(&p->_mutex);
                while (p->_tasks.empty())
                {
                    pthread_cond_wait(&p->_cond, &p->_mutex);
                }
                auto t = p->_tasks.front();
                p->_tasks.pop();
                pthread_mutex_unlock(&p->_mutex);

                t();
            }
        }

        private:
        std::vector<thread> _threads;
        std::queue<task_> _tasks;
        pthread_mutex_t _mutex;
        pthread_cond_t _cond;
    };
```

我们看到效果还是可以的

```shell
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-21 10:42:14]::socket create success, sockfd: 5
[Info][2025-3-21 10:42:14]::socket init success
[Info][2025-3-21 10:42:14]::bind success
[Info][2025-3-21 10:42:14]::start listening
[Info][2025-3-21 10:42:14]::tcpserver is running....
[Info][2025-3-21 10:42:17]::accept a new connection, sockfd: 6
[Info][2025-3-21 10:42:21]::accept a new connection, sockfd: 7
client say# ces
[Info][2025-3-21 10:42:22]::user quit, close sockfd: 7
client say# cdac
[Info][2025-3-21 10:42:24]::user quit, close sockfd: 6
[Info][2025-3-21 10:42:43]::accept a new connection, sockfd: 8
client say# cwd

[Info][2025-3-21 10:42:47]::user quit, close sockfd: 8
^C
[wind@starry-sky TCP]$
```

当然, 这种代码它也只能应对短服务, 长服务还是应对不了.

下面, 我们写一个简单的翻译程序, 中文对照表将以文件形式存在硬盘中, 程序运行时, 将其中的KV值加载进内存中, 服务为用户输入一个英文单词, 回显一个中文词汇.

```txt
apple 苹果
banana 香蕉
```

```cpp
#include <string>
#include <fstream>
#include <unordered_map>

#include <iostream>

#define DICTPIFE "dict.txt"

namespace wind
{
    class dict
    {
        typedef std::string string;
        typedef std::fstream fstream;
        typedef std::unordered_map<string, string> unordered_map;

        public:
        static dict &getInstance()
        {
            static dict inst;
            return inst;
        }

        const string &translate(const string &s)
        {
            if (hash.find(s) != hash.end())
                return hash[s];
            else
                return _s;
        }

        private:
        dict()
        {
            fstream fs(DICTPIFE);
            while (fs)
            {
                string kv;
                getline(fs, kv);
                size_t space1 = kv.find(' ');
                size_t space2 = kv.find('\n', space1 + 1);
                string k = kv.substr(0, space1);
                string v = kv.substr(space1 + 1, space2 - (space1 + 1));
                hash[k] = v;
            }
        }

        void test()
        {
            for (const auto &[m, n] : hash)
            {
                std::cout << m << ":" << n << std::endl;
            }
        }

        private:
        unordered_map hash;
        string _s = "unknow";
    };

}
```

```cpp
class task
{
    typedef std::string string;

    public:
    task() {}
    task(const int &sockfd, const char *ip, const uint16_t &port) : _sockfd(sockfd), _ip(ip), _port(port) {}
    void operator()()
    {
        char buff[BUFFER_SIZE];
        // dict::getInstance()
        ssize_t len = read(_sockfd, buff, sizeof(buff) - 1);
        if (len > 0)
        {
            buff[len] = 0;
            string k(buff);

            const string& v = dict::getInstance().translate(k);

            write(_sockfd, v.c_str(), v.size());
        }

        _log(Info, "user quit, close sockfd: %d", _sockfd);
        close(_sockfd);
    }

    private:
    int _sockfd;
    string _ip;
    uint16_t _port;
    static Log _log;   // Log之前不是单例, 现在也不好改,改了前面不兼容 所以就用这种方式了
};

```

我们可以看到, 效果也是可以的

```shell
[wind@starry-sky TCP]$ make clean
[wind@starry-sky TCP]$ make
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-21 11:44:13]::socket create success, sockfd: 5
[Info][2025-3-21 11:44:13]::socket init success
[Info][2025-3-21 11:44:13]::bind success
[Info][2025-3-21 11:44:13]::start listening
[Info][2025-3-21 11:44:13]::tcpserver is running....
[Info][2025-3-21 11:44:17]::accept a new connection, sockfd: 6
[Info][2025-3-21 11:44:28]::user quit, close sockfd: 6
[Info][2025-3-21 11:44:58]::accept a new connection, sockfd: 7
[Info][2025-3-21 11:45:1]::user quit, close sockfd: 7
[Info][2025-3-21 11:45:35]::accept a new connection, sockfd: 6
[Info][2025-3-21 11:45:39]::user quit, close sockfd: 6
^C
[wind@starry-sky TCP]$
```

```shell
[wind@starry-sky TCP]$ ./tcpclient 120.55.90.240 8888
connect success
Please Enter# apple
苹果
Please Enter# ^C
[wind@starry-sky TCP]$
```

```shell
[whisper@VM-12-6-centos TCP]$ ./tcpclient 120.55.90.240 8888
connect success
Please Enter# banana
香蕉
Please Enter# ^C
[whisper@VM-12-6-centos TCP]$
```

需要注意的是`telnet`可能对输入的字符串加了私货, 所以大概率匹配不成功.

-------------

接下来我们就不改TCP服务端的大框架了, 而是在此框架基础上对细节进行优化.

首先我们要解决的是对服务端的写进行差错处理. 读我们是处理过的, 但写我们并没有进行额外判断, 直接就写了, 此时就可能会产生一种情况:  客户端已经把连接关闭了, 但服务端还继续往里面写, 此时会出现什么问题呢?

当然, 这种情况发生的概率很小, 毕竟我们刚刚才尝试读过, 而且还读成功了, 所以要断开连接必须是在读成功和开始写的中间, 这个时间是很短的, 所以概率也很小, 不过, 这种概率我们仍然不能忽略, 下面我们就故意找个麻烦, 想办法让客户端在读完之后立刻断开连接.

我们首先认识一下`pidof`指令, 它可以用来查询某个进程的PID, 接下来, 我们想做的是, 让客户端和服务端本地环回, 服务端在读成功之后, 立刻使用`popen`, (我们在UDP谈过这个接口)执行`pidof`把客户端的PID查出来, 然后给客户端发信号, 让客户端主动断开连接.   至于客户端那边, 我们则需要捕捉一下信号.(不能直接用kill -9 , 那样没有进行完整的关闭流程, 服务端仍会认为连接正常)

```cpp
// 服务端
void operator()()
{
    char buff[BUFFER_SIZE];
    // dict::getInstance()
    ssize_t len = read(_sockfd, buff, sizeof(buff) - 1);
    if (len > 0)
    {
        buff[len] = 0;
        string k(buff);

        const string& v = dict::getInstance().translate(k);

        trouble();

        len = write(_sockfd, v.c_str(), v.size());
        if(len < 0)
        {
            _log(Warning, "write error: %s", strerror(errno));
        }
    }

    _log(Info, "user quit, close sockfd: %d", _sockfd);
    close(_sockfd);
}

void trouble() {
    FILE* fp = popen("pidof -s tcpclient", "r");
    char buff[16];

    // 如果 fgets 返回 NULL，说明无输出，直接退出
    if (fgets(buff, sizeof(buff), fp) == NULL) {
        pclose(fp);
        return;
    }

    // 去掉末尾 \n
    buff[strlen(buff) - 1] = '\0';

    pid_t id = atoi(buff);
    kill(id, SIGUSR1);

    pclose(fp);
}

// 客户端
int fd = 0;

void handler(int event)
{
    close(fd);
    sleep(10);
}

int main(int argc, char* argv[])
{
    signal(SIGUSR1, handler);
    // .........
    fd = sockfd;

   // ......
    return 0;
}
```

结果如下

```shell
[wind@starry-sky TCP]$ ./tcpserver -8888
[Info][2025-3-22 17:17:44]::socket create success, sockfd: 5
[Info][2025-3-22 17:17:44]::socket init success
[Info][2025-3-22 17:17:44]::bind success
[Info][2025-3-22 17:17:44]::start listening
[Info][2025-3-22 17:17:44]::tcpserver is running....
[Info][2025-3-22 17:17:56]::accept a new connection, sockfd: 6
[Info][2025-3-22 17:17:59]::user quit, close sockfd: 6
^C
[wind@starry-sky TCP]$
```

我们看到, `write`并没有失败, 但需要说明的是, 不同的平台对于写已经关闭的套接字处理是不同的, 对于某些平台来说, 这种情况可能会导致某些信号, 比如`SIGPIPE `被触发, 从而引发服务端的退出. 所以我们要对相关的信号进行忽略.

```cpp
void init()
{
    signal(SIGPIPE, SIG_IGN);

    // 线程池已经改为单例模式
    threadPool::getInstance().start();

    socket_init();

    struct sockaddr_in local;
    sockaddr_in__init(local);

    bind_port(local);

    start_listening();
}
```

接下来我们就不继续优化服务端了, 而是把重心放到客户端上.

我们先把客户端写的更模块化一点, 把差错处理放模块内部, `main`里面只要能体现我们的主逻辑就行了.

```cpp
#include <string>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

#define BUFFSIZE 4096

enum{
    SOCKETERRO = 1,
    CONNECTERRO = 2,
};

struct sockaddr_in args_parsing(char** argv)
{
    int i = 0;
    for(; argv[i]; ++i);
    if(i != 3)
    {
        cout << "parsing error"<<endl;
        exit(1);
    }
    sockaddr_in result;
    memset(&result, 0, sizeof(result));
    result.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &result.sin_addr);
    result.sin_port = htons(stoi(argv[2]));
    return result;
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        cerr << "create socket error: "<< strerror(errno)<<endl;
        exit(SOCKETERRO);
    }
    return sockfd;
}

void link_to(const int& sockfd, const struct sockaddr_in& server)
{
    if(connect(sockfd, reinterpret_cast<const struct sockaddr*>(&server), static_cast<socklen_t>(sizeof(server))) != 0)
    {
        cerr << "connect error: "<< strerror(errno)<<endl;
        exit(CONNECTERRO);
    }
}

void get_message(string& message)
{
    cout << "Please Enter# ";
    getline(cin, message);
}

void push(const int& sockfd, const string& message)
{
    ssize_t len = write(sockfd, message.c_str(), message.size());
    if(len < 0)
    {
        cerr << "push error: "<< strerror(errno)<<endl;
    }
}

string pull(const int& sockfd)
{
    char buffer[BUFFSIZE];
    ssize_t len = read(sockfd, buffer, sizeof(buffer) - 1);
    if(len < 0)
    {
        cerr << "pull error: "<< strerror(errno)<<endl;
        return "";
    }
    buffer[len] = 0;
    string result(buffer);
    return result;
}

int main(int argc, char* argv[])
{
    int sockfd = create_socket();

    // 客户端不由我们亲自绑定

    auto server = args_parsing(argv);

    // 与服务端建立连接
    link_to(sockfd, server);

    string message;
    while(true)
    {
        get_message(message);

        push(sockfd, message);

        auto response = pull(sockfd);
        cout << response<<endl;
    }

    close(sockfd);

    return 0;
}
```

我们在这份代码的基础上修改.

首先我们先把循环改一下, 因为服务端现在已经是短服务了, 所以如果客户端想要接受长服务, 它自己就要不断进行重连.   我们姑且这样写.

```cpp
int main(int argc, char *argv[])
{
    int sockfd = create_socket();

    // 客户端不由我们亲自绑定

    auto server = args_parsing(argv);

    string message;
    while (true)
    {
        // 与服务端建立连接
        link_to(sockfd, server);

        get_message(message);

        push(sockfd, message);

        auto response = pull(sockfd);
        cout << response << endl;
    }

    close(sockfd);

    return 0;
}
```

编译运行之后我们发现, 第二次连不上

```shell
[wind@starry-sky TCP]$ ./tcpclient 120.55.90.240 8888
Please Enter# apple
苹果
connect error: Transport endpoint is already connected
[wind@starry-sky TCP]$
```

为什么呢? 这是因为,  这里并没有关闭`sockfd`, 尽管服务端那里已经断连了, 客户端这里仍然认为该套接字已经建立连接了, 所以不允许你使用同一个套接字继续连接, 为此, 我们重连就需要关闭`sockfd`, 然后创建一个新的`sockfd`.

```cpp
int main(int argc, char *argv[])
{
    // 客户端不由我们亲自绑定

    auto server = args_parsing(argv);

    string message;
    while (true)
    {
        int sockfd = create_socket();

        // 与服务端建立连接
        link_to(sockfd, server);

        get_message(message);

        push(sockfd, message);

        auto response = pull(sockfd);
        cout << response << endl;

        close(sockfd);

    }
    return 0;
}

```

此时就可以在客户端维持长服务了

```shell
[wind@starry-sky TCP]$ ./tcpclient 120.55.90.240 8888
Please Enter# apple
苹果
Please Enter# apple
苹果
Please Enter# apple
苹果
Please Enter# ^C
[wind@starry-sky TCP]$
```

不过, 在上面的代码中, 对于错误的处理方式太暴力了, 我们可以换一种温柔的方式, 当连接不成功时, 就自动尝试重连, 重连若干次数时再退出进程, 对于推送和拉取失败, 也不要直接退出进程, 而是跳过后面的步骤, 重新进行连接.

由于我们的代码形式不太好用错误码, 所以我们就用一下C++的异常机制.

```cpp

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        throw system_error(errno, system_category(), "create socket error");
    }
    return sockfd;
}

void link_to(const int& sockfd, const struct sockaddr_in& server)
{
    if(connect(sockfd, reinterpret_cast<const struct sockaddr*>(&server), static_cast<socklen_t>(sizeof(server))) != 0)
    {
        throw system_error(errno, system_category(), "connect error");
    }
}

void get_message(string& message)
{
    cout << "Please Enter# ";
    getline(cin, message);
}

void push(const int& sockfd, const string& message)
{
    ssize_t len = write(sockfd, message.c_str(), message.size());
    if(len < 0)
    {
        throw system_error(errno, system_category(), "push error");
    }
}

string pull(const int& sockfd)
{
    char buffer[BUFFSIZE];
    ssize_t len = read(sockfd, buffer, sizeof(buffer) - 1);
    if(len < 0)
    {
        throw system_error(errno, system_category(), "pull error");
    }
    buffer[len] = 0;
    string result(buffer);
    return result;
}

int comm_prep(const struct sockaddr_in &server)
{
    int sockfd = 0;
    int count = 10;
    while (count--)
    {
        try
        {
            sockfd = create_socket();
            link_to(sockfd, server);
            return sockfd;
        }
        catch (const std::system_error &e)
        {
            cerr << e.what() << endl;
        }
        cout << "Reconnecting, please wait..."<<endl;
        sleep(2);
    }
    cout << "Unable to connect, exiting..."<<endl;
    exit(0);
}

int main(int argc, char *argv[])
{
    // 客户端不由我们亲自绑定

    auto server = args_parsing(argv);

    string message;
    while (true)
    {
        int sockfd = comm_prep(server);
        get_message(message);

        try
        {
            push(sockfd, message);
            auto response = pull(sockfd);
            cout << response << endl;
        }
        catch(const std::system_error &e)
        {
            cerr << e.what() << endl;
        }

        close(sockfd);
    }
    return 0;
}

```

我们可以看到, 它是可以重连的,  也可以连续请求的

```shell
[wind@starry-sky TCP]$ ./tcpclient 120.55.90.240 8888
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
Please Enter# apple
苹果
Please Enter# apple

connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
Unable to connect, exiting...
[wind@starry-sky TCP]$ make clean ; make
[wind@starry-sky TCP]$ ./tcpclient 120.55.90.240 8888
Please Enter# cat
猫
Please Enter# sleep
睡
Please Enter# listen
听
Please Enter# study
学习
Please Enter# sad
悲伤
Please Enter# small
小
Please Enter# walk
走
Please Enter# weater
unknow
Please Enter# water
水
Please Enter# ^C
[wind@starry-sky TCP]$
```

还有一件事, 就是我们的客户端并没有屏蔽信号, 因为对于客户端来说, 我们其实并不怕它突然挂掉, 而是怕它卡死, 挂掉用户可以立刻感受到, 然后重连, 卡死就进退两难, 不好操作.

最后一件事, 就是主动把服务端`ctrl c`后, 一段时间内重新启动很可能会绑定失败, 因为我们是直接退出的, 没有主动关闭监听套接字, 所以重新绑定就绑定不上,   我们可以通过`setsockopt`对监听套接字进行设置, 使其可以连续绑定

```cpp
void init()
{
    // signal(SIGPIPE, SIG_IGN);

    // 线程池已经改为单例模式
    threadPool::getInstance().start();

    socket_init();

    int opt = 1;
    // 防止服务端异常退出后出现不能立即重启的情况    复用地址  和    端口      讲原理的时候再细说
    setsockopt(_listeningSockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in local;
    sockaddr_in__init(local);

    bind_port(local);

    start_listening();
}
```

```shell
[wind@starry-sky TCP]$ ./tcpclient 120.55.90.240 8888
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
Please Enter# apple
苹果
Please Enter# apple

connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
connect error: Connection refused
Reconnecting, please wait...
Please Enter# apple
苹果
Please Enter# water
水
Please Enter# sleep
睡
Please Enter# derk
unknow
Please Enter# desk
桌子
Please Enter# ^C
[wind@starry-sky TCP]$
```

## 守护进程

对于上面的翻译服务来说, 它离实际真正意义上的服务, 似乎还有点小差距. 我们的服务是在`bash`上跑的, 如果`bash`被我们关闭, 其上的服务就有可能收到影响, 为此, 我们需要将服务守护进程化.

首先, 我们知道, 在Linux中, 有所谓的前后台概念.   我们先写一个十分简单的测试程序.

```cpp
#include<iostream>
#include<unistd.h>

using namespace std;

int main()
{
    while(true)
    {
        cout << "hello ..." <<endl;
        sleep(1);
    }
    return 0;
}
```

我们启动一个进程, 它默认会在前台运行.

```shell
[wind@starry-sky test]$ ./a.out
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
^C
[wind@starry-sky test]$
```

只有前台进程可以读到标准输入, 所以此时输入`ls`, `pwd`之类的指令, `bash`此时作为后台进程, 都不会响应, 而由于`a.out`能读到标准输入, 所以用标准输入发信号它也能收到.

对于Linux来说, 每当用户登录, 系统都会为该用户创建一个"session"会话, 同时为了让用户能够操作这个会话, 也会默认启动一个与会话对应的`bash`, 一个会话有且仅有一个前台进程和若干个后台进程, 当用`bash`直接启动一个进程, 这个新进程就会成为前台进程, `bash`自己就成为后台了, 如果这个进程被终止, 那`bash`又会重新变成前台进程, 而当以后台方式(也就是后面加`&`)启动, 新进程就会变成后台, `bash`继续是前台, 仍旧可以执行各种指令.

```shell
[wind@starry-sky test]$ ./a.out &
[1] 29691
[wind@starry-sky test]$ hello ...
hello ...
hello ...
lshello ...

a.out  test.cpp
[wind@starry-sky test]$ hello ...
hello ...
whhello ...
oamihello ...

wind
[wind@starry-sky test]$ hello ...
hello ...
hello ...
hello ...
hello ...
hello ...
hello ...

```

 当把一个进程以后台方式启动, 便会出现这种提示`[1] 29691`, 掐面的数字"1"被称为任务号, 后面的`29691`则是进程PID, 我们可以使用`fg` + 任务号的形式, 将对应的后台进程提到前台, `jobs`可以查看当前会话的后台任务.

```shell
[wind@starry-sky test]$ ./a.out >> log.txt &
[1] 30463
[wind@starry-sky test]$ fg 1
./a.out >> log.txt
^C
[wind@starry-sky test]$ ./a.out >> log1.txt &
[1] 30515
[wind@starry-sky test]$ ./a.out >> log2.txt &
[2] 30527
[wind@starry-sky test]$ ./a.out >> log3.txt &
[3] 30536
[wind@starry-sky test]$ jobs
[1]   Running                 ./a.out >> log1.txt &
[2]-  Running                 ./a.out >> log2.txt &
[3]+  Running                 ./a.out >> log3.txt &
[wind@starry-sky test]$
```

如果想将一个前台进程重新放回后台, 可以使用`ctrl z`对前台进程发送`SIGSTOP`信号 , 从而暂停前台进程, 此时`bash`就会自动顶替, 成为前台进程, 否则没进程读标准输入, 会话就会失控.

```shell
[wind@starry-sky test]$ fg 1
./a.out >> log1.txt
^C
[wind@starry-sky test]$ jobs
[2]-  Running                 ./a.out >> log2.txt &
[3]+  Running                 ./a.out >> log3.txt &
[wind@starry-sky test]$ fg 3
./a.out >> log3.txt
^Z
[3]+  Stopped                 ./a.out >> log3.txt
[wind@starry-sky test]$ jobs
[2]-  Running                 ./a.out >> log2.txt &
[3]+  Stopped                 ./a.out >> log3.txt
[wind@starry-sky test]$
```

`bg`任务号可以让后台已经暂停所谓进程继续运行

```shell
[wind@starry-sky test]$  bg 3
[3]+ ./a.out >> log3.txt &
[wind@starry-sky test]$ jobs
[2]-  Running                 ./a.out >> log2.txt &
[3]+  Running                 ./a.out >> log3.txt &
[wind@starry-sky test]$
```

下面我们引入一个新的话题, Linux的进程间关系.

```shell
[wind@starry-sky test]$ ./a.out >> log.txt &
[1] 31559
[wind@starry-sky test]$ sleep 1000 | sleep 2000 | sleep 3000 &
[2] 31577
[wind@starry-sky test]$ ps ajx | head -1 && ps ajx |grep -Ei 'a.out|sleep' | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27933 31479 27933 27933 ?           -1 S     1002   0:00 sleep 180
27883 31559 31559 27883 pts/0    31586 S     1002   0:00 ./a.out
27883 31575 31575 27883 pts/0    31586 S     1002   0:00 sleep 1000
27883 31576 31575 27883 pts/0    31586 S     1002   0:00 sleep 2000
27883 31577 31575 27883 pts/0    31586 S     1002   0:00 sleep 3000
[wind@starry-sky test]$
```

在这里我们启动了两个任务, 每个任务由若干个进程去执行, 执行同一个任务的进程, 就构成了一个进程组, 对于任务一来说, 由于只有一个进程`a.out`执行, 所以它自成一个任务组, 而对于任务二来说, 由于有三个进程执行, 所以它们就构成了一个进程组.

在其中, `PGID`表示进程组ID, `SID`表示会话ID, `TTY`则是终端号. 对于任务一来说, 因为只有一个进程, 所以这个进程就自己成为了自己进程组的组长进程, 进程组ID就与它自己的`PID`相同, 而对于任务二来说, 由于有多个进程, 所以最前面的进程就成为了组长, 进程组ID就是组长进程的ID.

为此, 我们就需要把之前的前台进程和后台进程改个说法了, 更严谨的说法是前台任务和后台任务, 同一个会话有多个任务, 其中有且只有一个任务作为前台任务, 其余的则作为后台任务, 比如上面的例子中, 任务一和任务二都是后台任务, 它们的`SID`都是`27883`, 而这个`27883`其实就是`bash`的进程ID.

```shell
[wind@starry-sky test]$ ps axj | head -1 && ps ajx | grep 27883 | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27883   464   464 27883 pts/0      464 R+    1002   0:00 ps ajx
27882 27883 27883 27883 pts/0      464 Ss    1002   0:00 -bash
27883 31559 31559 27883 pts/0      464 S     1002   0:00 ./a.out
27883 31576 31575 27883 pts/0      464 S     1002   0:00 sleep 2000
27883 31577 31575 27883 pts/0      464 S     1002   0:00 sleep 3000
[wind@starry-sky test]$
```

 另外我们还可以看到, 即使组长已经退出了, 它仍然是本进程组的组长, 剩下的两个`sleep`, 其进程组ID仍旧是`31575`.

下面我们写一个能够监视`-bash`的简易脚本`while :; do ps ajx | head -1 && ps ajx | grep -E '\-bash$'| grep -v grep; sleep 1; done`, `-bash`前面的`-`, 表示的就是远端启动,  `'\-bash$'`中的`\`表示转义, 不带的话`-`容易被识别成别的东西, `$`的意思是过滤出的内容都必须以`-bash`做结尾.

下面我们在执行该脚本的同时, 再启动多个会话, 我们看看效果

```shell
[wind@starry-sky test]$ while :; do ps ajx | head -1 && ps ajx | grep -E '\-bash$'| grep -v grep; sleep 1; done
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27882 27883 27883 27883 pts/0     1807 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27882 27883 27883 27883 pts/0     1814 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27882 27883 27883 27883 pts/0     1821 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27882 27883 27883 27883 pts/0     1828 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27882 27883 27883 27883 pts/0     1836 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1871 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1878 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1885 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1892 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1900 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1935 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1942 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1949 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1958 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1993 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     1999 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2007 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2043 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2050 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2057 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1841  1842  1842  1842 pts/8     1842 Ss+   1002   0:00 -bash
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2064 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2071 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2078 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2085 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1905  1906  1906  1906 pts/9     1906 Ss+   1002   0:00 -bash
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2092 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2100 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 1962  1963  1963  1963 pts/10    1963 Ss+   1002   0:00 -bash
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2106 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2114 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2013  2014  2014  2014 pts/11    2014 Ss+   1002   0:00 -bash
27882 27883 27883 27883 pts/0     2121 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27882 27883 27883 27883 pts/0     2129 Ss    1002   0:00 -bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27882 27883 27883 27883 pts/0     2136 Ss    1002   0:00 -bash
^C
[wind@starry-sky test]$
```

现在假设我们在一个会话中启动了多个后台任务, 当会话退出时, 会怎么样呢?

```shell
[wind@starry-sky test]$ ./a.out >> log.txt1 &
[1] 2670
[wind@starry-sky test]$ ./a.out >> log.txt2 &
[2] 2676
[wind@starry-sky test]$ ./a.out >> log.txt3 &
[3] 2684
[wind@starry-sky test]$ sleep 1000 | sleep 2000 | sleep 3000 &
[4] 2692
[wind@starry-sky test]$ ps ajx | head -1 && ps ajx | grep sleep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27933  2620 27933 27933 ?           -1 S     1002   0:00 sleep 180
 2391  2690  2690  2391 pts/8     2708 S     1002   0:00 sleep 1000
 2391  2691  2690  2391 pts/8     2708 S     1002   0:00 sleep 2000
 2391  2692  2690  2391 pts/8     2708 S     1002   0:00 sleep 3000
 2391  2709  2708  2391 pts/8     2708 S+    1002   0:00 grep --color=auto sleep
[wind@starry-sky test]$ ps ajx | head -1 && ps ajx | grep a.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
 2391  2670  2670  2391 pts/8     2722 S     1002   0:00 ./a.out
 2391  2676  2676  2391 pts/8     2722 S     1002   0:00 ./a.out
 2391  2684  2684  2391 pts/8     2722 S     1002   0:00 ./a.out
 2391  2723  2722  2391 pts/8     2722 S+    1002   0:00 grep --color=auto a.out
[wind@starry-sky test]$ jobs
[1]   Running                 ./a.out >> log.txt1 &
[2]   Running                 ./a.out >> log.txt2 &
[3]-  Running                 ./a.out >> log.txt3 &
[4]+  Running                 sleep 1000 | sleep 2000 | sleep 3000 &
[wind@starry-sky test]$ logout

Connection closed.

Disconnected from remote host(starry-sky) at 12:41:27.

Type `help' to learn how to use Xshell prompt.
[C:\~]$

Connecting to 120.55.90.240:22...
Connection established.
To escape to local shell, press 'Ctrl+Alt+]'.

WARNING! The remote SSH server rejected X11 forwarding request.
Last login: Sun Mar 23 12:36:15 2025 from 112.26.31.132

Welcome to Alibaba Cloud Elastic Compute Service !

[wind@starry-sky ~]$ jobs
[wind@starry-sky ~]$ ps ajx | head -1 && ps ajx | grep sleep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
27933  2620 27933 27933 ?           -1 S     1002   0:00 sleep 180
    1  2690  2690  2391 ?           -1 S     1002   0:00 sleep 1000
    1  2691  2690  2391 ?           -1 S     1002   0:00 sleep 2000
    1  2692  2690  2391 ?           -1 S     1002   0:00 sleep 3000
 2742  2788  2787  2742 pts/0     2787 S+    1002   0:00 grep --color=auto sleep
[wind@starry-sky ~]$ ps ajx | head -1 && ps ajx | grep a.out
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
    1  2670  2670  2391 ?           -1 S     1002   0:00 ./a.out
    1  2676  2676  2391 ?           -1 S     1002   0:00 ./a.out
    1  2684  2684  2391 ?           -1 S     1002   0:00 ./a.out
 2742  2798  2797  2742 pts/0     2797 S+    1002   0:00 grep --color=auto a.out
[wind@starry-sky ~]$
```

我们看到, 由于它们本身都是`bash`的子进程, 会话退出后, `bash`也退出, 所以它们被托孤到系统上了, 但仍在继续运行, 但它们的会话状态确实是因为我们的退出而发生了变化, 有没有一种方法, 可以让任务自成一个会话, 从而不受到任何用户登录或注销的影响呢? 有, 这种技术就叫做守护进程化.

Linux确实在会话退出后继续运行会话里面的任务, 但对于别的系统, 比如Windows来说, 就不是这样了, Windows也是用户登录启动一个会话, 它有一个"注销"选项, 注销就是用户登出, 为用户启动的会话也会退出, 会话里面的任务也都会终止.   所以如果Windows变卡了, 可以注销, 把卡的任务终止掉, 这样重新登录就会顺畅了.

守护进程的核心接口就是`setsid`

```cpp
#include <unistd.h>

pid_t setsid(void);
```

功能就是创建一个新会话, 并将当前进程设置为该会话的领导进程, 会话的领导进程就是和会话ID相同PID的进程, 对于用户登录产生的会话来说, 它们的领导进程就是`bash`.

需要注意的是, 调用该接口的进程不能是另一个进程组的组长. 我们启动一个服务, 最开始还是从`bash`上启动的, 所以它最开始是属于某个用户会话中进程组中的进程, 对于单个进程来说, 由于它自成一个进程组, 所以它自己就是该进程组的组长, 所以创建新会话的另一个要点就是要用多进程, 此时父进程就是进程组组长了, 然后让子进程去真正执行这个`setsid`.

下面我们直接写个接口, 通过复用`fork`和`setsid`, 其它进程调了就能产生一个守护进程.

```cpp
#pragma once
#include<unistd.h>
#include<iostream>
#include<string>
#include<signal>

void daemon(const std::string& cwd = std::string())
{
    // 创建守护进程一般也要屏蔽一些信号
    // 让守护进程不被信号打扰, 安心服务
    signal(SIGCLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);

    // 创建新会话
    if(fork() > 0) exit(0);
    setsid();

    // 守护进程常被直接安装在系统上
    // 它的各类配置文件或输出文件都会在系统的特定位置上
    // 所以有时需要更改工作目录, 直接移到根目录下
    if(cwd.size())
    {
        chdir(cwd.c_str());
    }
}
```

然后我们就可以把这个`daemon`直接用在上面的`TCP`服务端上面了, 至于用在那里, 那要看具体情况, 比如, 这里我们对于单例`dict`的初始化用的是相对路径, 那在`dict`初始化之前就不能该工作目录, 为了安全起见, 我们在初始化时把`dict`实例化一下, 然后再在`run`里调用`daemon`.

```cpp
void init()
{
    // signal(SIGPIPE, SIG_IGN);

    // 线程池已经改为单例模式
    threadPool::getInstance().start();

    socket_init();

    int opt = 1;
    // 防止服务端异常退出后出现不能立即重启的情况    复用地址  和    端口      讲原理的时候再细说
    setsockopt(_listeningSockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in local;
    sockaddr_in__init(local);

    bind_port(local);

    start_listening();

    dict::getInstance();
}

void run()
{
    daemon();
    _log(Info, "tcpserver is running....");
    while (true)
    {
        int sockfd = 0;
        struct sockaddr_in client;
        if (accept_connection(sockfd, client) == -1)
            continue;

        uint16_t port = ntohs(client.sin_port);
        char ipbuff[32];
        inet_ntop(AF_INET, &client.sin_addr, ipbuff, sizeof(ipbuff));

        // service(sockfd, ipbuff, port);    // 单进程版

        // service2(sockfd, ipbuff, port);   // 多进程版

        // service3(sockfd, ipbuff, port);   // 多线程基础版

        service4(sockfd, ipbuff, port);   // 线程池

    }
}
```

不过这里的`daemon`还有些问题, 我们可以看到在调用`daemon`之后仍有许多打印信息, 可成为守护进程之后, 已经没有终端了, 此时乱打印就会有风险. 对于这种问题, 我们可以分情况来讨论, 如果完全不需要日志, 可以把标准输入, 标准输出, 标准错误重定向到一个无关紧要的文件中, 如果需要日志, 那就开启日志的文件输出模式, 这种情况也要把标准输入,标准输出,标准错误重定向到无关紧要的文件上, 用来防范可能存在的不使用日志的打印. 那三个标准文件也不能关闭, 关闭套接字可能占位, 日志就会打到套接字里.                   所以无论如何, `daemon`还需要做一件事, 把三个标准文件重定向.

在Linux中, 有一个文件, 叫做`/dev/null`

```shell
[wind@starry-sky TCP]$ ll /dev/null
crw-rw-rw- 1 root root 1, 3 Sep 12  2024 /dev/null
```

它就是一个垃圾桶, 标准输入从这个文件里读, 什么都读不到, 写入的内容也会被自动丢弃.

```cpp
void daemon(const std::string &cwd = std::string())
{
    // 创建守护进程一般也要屏蔽一些信号
    // 让守护进程不被信号打扰, 安心服务
    signal(SIGCLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);

    // 创建新会话
    if (fork() > 0)
        exit(0);
    setsid();

    // 守护进程常被直接安装在系统上
    // 它的各类配置文件或输出文件都会在系统的特定位置上
    // 所以有时需要更改工作目录, 直接移到根目录下
    if (cwd.size())
    {
        chdir(cwd.c_str());
    }

    int fd = open("/dev/null", O_RDWR);
    if(fd > 0)
    {
        dup2(fd, 0);
         dup2(fd, 1);
        dup2(fd, 2);
        dup2(fd, 3);   // 这里再重定向的原因是我这里的日志在终端显示模式下会再开一个文件作为日志标准输出, 这里因人而异, 另外这里也必须把日志改成单例了, 否则多个静态日志对象会再多开文件, 下面的实验都已经建立在日志单例话的基础上进行的
        close(fd);
    }
}
```

现在我们重新编译启动, 可能会遇到一些问题.   比如, 原本服务不处于守护进程状态是都很流畅, 但一旦设置为守护进程状态, 就会变得非常卡, 甚至是服务完全不能用, 这就和 `daemon`的内部实现有关, 在`daemon`有一个`if(fork()>0) exit(0)`指令, 该指令有两个作用, 一是创建一个不作为进程组组长的子进程, 这样它才能正常调用`setsid()`, 另一方面, 它将不负责实际服务的父进程马上退出, 这样子进程就会被系统领养, 从而变为孤儿进程, 之后即使服务异常退出, 系统也会对其进行资源回收, 避免出现程序僵死而导致内存泄露的情况发生.             但这个代码, 实际上有一种风险, 那就是父进程退出的太快了,  导致子进程在继承父进程状态的过程中发生错误, 这样就会有很多非常奇奇怪怪的现象发生, 比如该有的文件都有(在/proc/PID/fd里都可以看到), 但好像不能读写了.

所以对于`daemon`的另一个重点就是, `daemon`一定要放在最最前面, 直接放在`main`第一行吧.

好的, 经过上面的这些过程, 我们应该可以很清晰地了解到网络服务的大致框架了, 而我们之所以能通过`Xshell`连上远端的Linux, 就是因为云服务器默认会开启一个名为`ssh`的服务, 用的是`22`号端口.

```shell
[wind@starry-sky TCP]$ sudo netstat -nltp
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      547/rpcbind
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      1093/sshd
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      1033/master
tcp6       0      0 :::111                  :::*                    LISTEN      547/rpcbind
tcp6       0      0 :::22                   :::*                    LISTEN      1093/sshd
tcp6       0      0 ::1:25                  :::*                    LISTEN      1033/master
[wind@starry-sky TCP]$
```

当然, 它是以守护进程的形式运行的, 对于守护进程来说, 一般会在程序名字后面加个`d`, 所以就变成了`sshd`.每次登录, 都是`Xshell`向`sshd`发送登录链接, 通过之后, 就启动一个会话, 把用户的指令发过去, 把运行的结果发回来.

对于新会话的创建, 其实有专门的系统接口, 那就是`deamon`

```cpp
#include <unistd.h>

int daemon(int nochdir, int noclose);
```

第一个参数, 表示是否要更改工作目录, 是`0`就改到根目录下, 不是就使用原路径, 第二个参数表示是否要把三个标准流重定向到`/dev/null`, `0`表示要重定向, 否则就什么也不做.

不过一般来说, 我们都自己写`daemon`, 而不用系统的, 因为他很简单, 而且自己写很明显能更个性化.

最后我们需要说的是, `TCP`和`UDP`类似, 也是全双工的, 也就是说它可以同时写和读, 至于原因, 和`UDP`一样, 它的底层读写各有各的缓冲区, 所以边写边读是完全没有问题的. 具体细节我们还是原理讲. 在这里, `read`和`write`的功能实际上就是把传输层接收缓冲区中的数据读到应用层, 把应用层发送缓冲区中的数据拷贝到传输层发送缓冲区, 至于到底什么时候发送, 什么时候接收, 怎么发, 怎么受, 都由`TCP`协议自己决定, 而与用户无关, 它控制着传输过程, 所以叫做传输控制协议.

# 完