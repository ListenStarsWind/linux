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

## 应用层自定义协议

在上面的过程中, 我们已经通过TCP搭建起了一个简易的网络服务平台, 但上面的平台实际上仍旧存在一些问题: 我们知道, `TCP`是一个传输层的全双工协议, 无论是客户端还是服务端, `TCP`都维护着它们各自的发送缓冲区和接收缓冲区. 因为传送和接收根本不是一个缓冲区, 所以自然可以同时读和写.

![image-20250325115011450](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250325115011622.png)

当服务端与大量客户端进行通信时, 就会产生这种问题:    服务端的协议层缓冲区同时存在多个报文, 并且这些报文可能并不是完整的. 报文不完整那自然无法正常进行通信, 所以我们下面就要在应用层解决这个问题. 

在之前的项目中, 我们认为, 只要确实读到了字符串, 那就一定是完整的

```cpp
void operator()()
{
    char buff[BUFFER_SIZE];
    ssize_t len = read(_sockfd, buff, sizeof(buff) - 1);

    if (len > 0)
    {
        buff[len] = 0;
        string k(buff);

        const string& v = dict::getInstance().translate(k);
        len = write(_sockfd, v.c_str(), v.size());
        if(len < 0)
        {
            _log(Warning, "write error: %s", strerror(errno));
        }
    }

    _log(Info, "user quit, close sockfd: %d", _sockfd);
    close(_sockfd);
}
```

但实际上, 在真实的服务环境中, 无论是服务端所面临的客户端数量, 还是服务端与单个客户端通信的数据量, 都会远远大于我们之前的测试, 所以这种报文不完整的发生将会很常见.

我们之前也遇到过类似的问题, 比如在之前学习进程通信的时候, 我们曾在管道中遇到类似的问题. 一个进程输"Hello world", 另一个进程收到"Hello"就走了, 或者是文件, 先向文件里一次性写多份数据, 等到之后读的时候就会很费力.

现在我们回到这张图上

![  ](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250325115011622.png)

`TCP`是传输控制协议, 它为了保证可靠性(`TCP`也在内核里面, 是系统的一部分), 具有高自主性, 诸如报文什么时候发, 发多少, 出错了怎么办之类的问题都由`TCP`自己决定,    现在假设我们的服务端接收缓冲区已经快满了, 在发送数据之前, 客户端和服务端的`TCP`协议就会进行沟通, 客户端说, "我这里有这么多的数据, 你那里能放的下吗?", 服务端说, "不行, 快满了, 你最多发这么点", 于是客户端就把数据截一半, 先发过去一部分, 剩下的部分等到服务端能收下再发, 然后, 服务端突然又缓过来了, 一口气把接收缓冲区你的数据又读到应用层了, 此时服务端的应用层就面临两个问题, 一是这个缓冲区里面有多个报文, 需要想办法把它们彼此分开, 另一个问题是, 分开之后, 有些报文不完整, 不能立刻解析, 必须要等着剩下的部分传过来再解析. 

这些问题就要靠我们应用层来解决了. 为了解决这些问题, 就需要应用层的两端建立一种约定, 或者说协议, 什么状态下, 才算一个完整的报文被读出来了,  读不完整该怎么办, 等等.

之前我们写的那些代码, 其实都是应用层的网络协议, 只不过我们那时的协议很粗糙, 都是口头规定的, 但这种口头规定不具有强制性和规范性, 所以不能称之为协议, 为了让约定变为协议, 我们就需要用某个类型来描述它, 然后通信过程中都借助于这些类型进行通信, 这样就有强约束性了.     所以我们要传输的都协议类的实例化对象.

但我们真的是直接把对象在内存中的数据直接扔给`TCP`, 让它面向字节流进行传输吗? 也不行, 因为即使是同一个类型, 所实例化出的对象, 在不同的平台上具体的组织形式也有可能是不同的, 这可能是由于多种原因造成的, 比如, 有些平台用的看上去是同一个类型, 但实际上, 并不一样, 比如, 我们知道, 32和64位的机器中的`size_t`底层并不相同, 甚至有些嵌入式环境下, `int`甚至是两个字节的. 另一个原因是, 结构体或者说自定义类型中有一个内存对齐的操作, 不同平台不同位数下的内存对齐可能不同, 在C++里面, 内存对齐还涉及到虚基表, 只要两端组织形式有一点点不一样, 都会出大问题, 这种方法的可靠性很低, 而且二进制的数据不好调试.

所以我们不直接传对象的二进制数据, (尽管系统TCP它们自己真的是直接传对象, 但我们应用层不能这样做), 而是把对象中的关键数据浓缩成一个字符串, 相当于让对象换一种形式存在, 而字符串一方面贴近人类语言, 调试时看的很清楚, 另一方面, 字符串天然面向字节流, 给`TCP`传就很合适, 然后再通过一系列手段, 保证另一端可以知道自己收到的数据完不完整, 完整再做解析, 重新还原成一个实例化对象.

把对象转化为字符串就被称之为序列化, 而把字符串还原成对象, 就称之为反序列化. 

至于对象的具体内容, 我们和别的协议一样, 也包括两个部分, 报头和负载, 报头我们可以加上应用层所需要的数据, 比如简要描述一下整个报文, 以供接受者判断是否完整, 我们可以再加上比如用户的账号信息, 而不再用以往的地址加端口区分用户, 还可以加上时间信息之类, 我们甚至还可以在应用层多设计几个层, 嵌套式的解包和加报头.

但这里我们作为初学者, 就不搞这么复杂了.    我们这里只加一次报头, 也不会在应用层里面又分层.   今天我们设计一个简易的网络计算器.

首先计算器肯定有两个部分, 首先要有两个操作数和操作符, 这是发过来的情况, 把结果发回去可能包含两个部分, 一是错误码, 表示计算是否是合法的, 二是运算结果, 所以网络计算器涉及到的类大概是这样的,

```cpp
struct input
{
    int x;
    int y;
    char op;
}

struct output
{
    int result;
    int code;
}
```

在发送的时候, 我们需要把这两个结构体转成字符串, 这样才方便进行网络通信, 对方接收之后, 又要还原出对象, 进行具体分析.    除了结构体本身的关键数据之外, 我们还需要把其它一些附属数据转成字符串, 比如, 拿聊天软件来说, 我们接收的时候, 都会收到诸如消息发送的时间, 谁发送的等附属信息, 这些附属字符串就相当于报头, 与表示计算对象的负载合在一起, 就构成了一份报文. 

![image-20250327154110318](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250327154110423.png)

无论对于任何协议, 它都包括两个部分, 一是协议本身的制定(以结构体的形式约定), 而是围绕协议的序列化和反序列化. 传送过程中, 要需要留意保证报文的完整性.

好的, 下面我们就开始写了, 项目框架如下:

```shell
[wind@starry-sky ALP]$ tree . -I "test"
.
├── ClientCal.cc
├── log.hpp
├── makefile
├── ServerCal.cc
├── Sockst.hpp
├── TcpClient.hpp
└── TcpServer.hpp

0 directories, 7 files
[wind@starry-sky ALP]$ cat makefile
CC = g++
CFLAGS = -Wall
CLIENT_STD = -std=c++11
SERVER_STD = -std=c++11
CLIENT_SRC = ClientCal.cc
SERVER_SRC = ServerCal.cc
CLIENT_TARGET = clientcal
SERVER_TARGET = servercal

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRC)
	@$(CC) $(CFLAGS) $(CLIENT_STD) $^ -o $@

$(SERVER_TARGET): $(SERVER_SRC)
	@$(CC) $(CFLAGS) $(SERVER_STD) $^ -o $@

clean:
	@rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
	[wind@starry-sky ALP]$
```

我们首先把`class socket_`写一下, 这里加下划线主要是为了避免与系统里的`socket`调用冲突.

```cpp
#pragma once

#include "log.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<iostream>
#include <memory>

enum socket_errno
{
    CREATE_ERROR = 1,
    BIND_ERROR = 2,
    LISTEN_ERROR = 3,
    ACCECT_ERROR = 4
};

using namespace wind;

class socket_
{
    public:
    socket_() {}
    ~socket_() { if(_sockfd > 0) close_();}

    void create_()
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            _log(Fatal, "socket create error: %s", strerror(errno));
            exit(CREATE_ERROR);
        }
    }

    void bind_(uint16_t port = 8888)
    {
        struct sockaddr_in sock_in;
        memset(&sock_in, 0, sizeof(sock_in));
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons(port);
        sock_in.sin_addr.s_addr = INADDR_ANY;
        if (bind(_sockfd, reinterpret_cast<const struct sockaddr *>(&sock_in), static_cast<socklen_t>(sizeof(sock_in))) != 0)
        {
            _log(Fatal, "bind error: %s", strerror(errno));
            exit(BIND_ERROR);
        }
    }

    void listen_()
    {
        if (listen(_sockfd, backlog) != 0)
        {
            _log(Fatal, "listen error: %s", strerror(errno));
            exit(LISTEN_ERROR);
        }
    }

    void connect_(uint16_t port, const char *addr)
    {
        struct sockaddr_in sock_in;
        memset(&sock_in, 0, sizeof(sock_in));
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons(port);
        if (inet_pton(AF_INET, addr, &sock_in.sin_addr) != 1)
        {
            _log(Fatal, "connect error: %s", strerror(errno));
            exit(ACCECT_ERROR);
        }
        if(connect(_sockfd, reinterpret_cast<const struct sockaddr*>(&sock_in), static_cast<socklen_t>(sizeof(sock_in))) != 0)
        {
            _log(Fatal, "connect error: %s", strerror(errno));
            exit(ACCECT_ERROR);
        }
    }

    int accept_(std::string *clientip, uint16_t *clientport)
    {
        struct sockaddr_in sock_in;
        socklen_t len = static_cast<socklen_t>(sizeof(sock_in));
        int fd = accept(_sockfd, reinterpret_cast<struct sockaddr *>(&sock_in), &len);
        if (fd < 0)
        {
            _log(Warning, "accept error: ", strerror(errno));
            return -1;
        }
        char buffer[64];
        inet_ntop(AF_INET, &sock_in.sin_addr, buffer, sizeof(buffer));
        *clientip = buffer;
        *clientport = ntohs(sock_in.sin_port);
        return fd;
    }

    void close_()
    {
        close(_sockfd);
        _sockfd = -1;
    }


    private:
    int _sockfd;
    Log &_log = Log::getInstance();
    static int backlog;
};

int socket_::backlog = 10;

void print_netstat()
{
    FILE *pipe = popen("netstat -nltp", "r");
    if (!pipe)
    {
        std::cerr << "Error: Failed to run netstat -nltp" << std::endl;
        return;
    }

    std::unique_ptr<FILE, decltype(&pclose)> pipe_guard(pipe, &pclose);

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        std::cout << buffer;
    }
}
```

然后我们把`class tcpserver`快速实现一下

```cpp
#pragma once

#include"Sockst.hpp"
#include"log.hpp"

class tcpserver
{
    public:
    tcpserver() {}
    void init()
    {
        _listensock.create_();
        _listensock.bind_();
        _listensock.listen_();
    }

    void run()
    {
        print_netstat();
        signal(SIGCHLD, SIG_IGN);
        while(true)
        {
            std::string clientip;
            uint16_t port;
            int sockfd = _listensock.accept_(&clientip, &port);
            if(sockfd == -1) continue;

            if(fork() == 0)
            {
                _listensock.close_();

                exit(0);
            }
            close(sockfd);

        }
    }

    private:
    socket_ _listensock;
    Log& _log = Log::getInstance();
};
```

我们这里重点是自定义协议, 所以这里就直接使用多进程了, 具体服务由子进程提供. 

接下来是协议,  先写两个协议类

```cpp
class expression
{
public:
    expression() : _state(false) {};
    expression(const int &x, const char *op, const int &y)
        : _x(x), _y(y), _op(op), _state(true)
    {
    }

    expression(const std::string &s)
        : _state(false)
    {
    }

    operator std::string()
    {
        std::string temp;
        temp += std::to_string(_x);
        temp += FIELD_SEPARATOR;
        temp += _op;
        temp += FIELD_SEPARATOR;
        temp += std::to - string(_y);

        return std::move(temp);
    }

    operator bool() { return _state; }

private:
    int _x;
    int _y;
    char _op;
    bool _state;
};

class result
{
public:
    result() : _state(false) {}
    result(const int &val_, const int &errno_) : _val(val_), _errno(errno_), _state(true) {}
    result(const std::string &s) : _state(false)
    {
    }

    operator std::string()
    {
        std::string temp;
        temp += std::to_string(_val);
        temp += FIELD_SEPARATOR;
        temp += std::to_string(_errno);

        return std::move(temp);
    }

    operator bool() { return _state; }

private:
    int _val;
    int _errno;
    bool _state;
};
```

我们使用类型转换重载函数负责对象的序列化, 使用构造函数负责对象的反序列化,在反序列化的时候, 正如前文所说, 可能会出现数据残缺的情况, 此时构造出的对象很明显将是不对的, 所以我们引入了`_state`, 来表示对象是否可以使用, 只有对象成功后, `_state`才会被置为`true`, 在上面的代码中, 我们也能看到, 它们是配备布尔类型转换重载函数的, 如果上一级发现对象返回的是`false`, 即意味着构造失败, 上一层就会知道现在的数据是残缺的, 它就会继续使用原先的那个缓冲区接受数据, 如果是构造成功, 上一级就会将缓冲区清空, 接收新的报文.

要注意的是, 为了实现分层结构, 我们这里的序列化和反序列化只负责结构化数据本身, 不负责其它内容. 那些其它的附属信息将在其它层序列化和反序列化, 

好的下面, 我们先来实现序列化和反序列化函数, 很明显, 如果让成员之间挨在一起, 比如`"10+20"`和`"300"`不太好, 所以我们可以用`" "`把成员之间彼此分开, 即`"10 + 20"`, `"30 0"`.   约定好序列化方式, 反序列化也很好进行.

```cpp
#define FIELD_SEPARATOR ' '           // 负载中字段间的分隔符

class expression
{
public:
    expression() : _x(0), _y(0), _op('+'), _state(false) {};
    expression(const int &x, char op, const int &y)
        : _x(x), _y(y), _op(op), _state(true)
    {
    }

    expression(const std::string &s)
        : _state(false) { init(s); }

    bool init(const std::string &s)
    {
        size_t pos1 = s.find(FIELD_SEPARATOR);
        if (pos1 == std::string::npos)
            return *this;
        size_t pos2 = s.find(FIELD_SEPARATOR, pos1 + 1);
        if (pos2 == std::string::npos)
            return *this;
        std::string x = s.substr(0, pos1);
        std::string op = s.substr(pos1 + 1, pos2 - (pos1 + 1));
        std::string y = s.substr(pos2 + 1);
        if (op.size() != 1) // 如果不为一说明格式错误,没有遵守协议, 抛出异常, 该报文已经可以放弃
            throw std::invalid_argument("Illegal parameter format");
        _x = std::stoi(x); // 当参数无法转换时, `stoi`也会抛出异常
        _op = op[0];
        _y = std::stoi(y);

        _state = true; // _state表示是否残缺, 异常表示格式错误

        return *this;
    }

    operator std::string()
    {
        std::string temp;
        temp += std::to_string(_x);
        temp += FIELD_SEPARATOR;
        temp += _op;
        temp += FIELD_SEPARATOR;
        temp += std::to_string(_y);

        return temp;
    }

    result operator()()
    {
        int val_ = 0;
        int erron_ = 0;
        switch (_op)
        {
        case '+':
            val_ = _x + _y;
            break;
        case '-':
            val_ = _x - _y;
            break;
        case '*':
            val_ = _x * _y;
            break;
        case '/':
            if (_y == 0)
                erron_ = DIVISION_BY_ZERO;
            break;
        default:
            erron_ = UNSUPPORTED_OPERATION;
            break;
        }

        result temp(val_, erron_);
        return temp;
    }

    operator bool() { return _state; }

    void print()
    {
        std::cout << "_x: " << _x << "  " << "_y: " << _y << "  " << "_op: " << _op << std::endl;
    }

private:
    int _x;
    int _y;
    char _op;
    bool _state;
};

class result
{
public:
    result() : _val(0), _errno(0), _state(false) {}
    result(const int &val_, const int &errno_) : _val(val_), _errno(errno_), _state(true) {}
    result(const std::string &s) : _state(false) { init(s); }

    bool init(const std::string &s)
    {
        size_t pos = s.find(FIELD_SEPARATOR);
        if (pos == std::string::npos)
            return *this;
        std::string val_ = s.substr(0, pos);
        std::string errno_ = s.substr(pos + 1);
        _val = stoi(val_);
        _errno = stoi(errno_);

        _state = true;

        return *this;
    }

    operator std::string()
    {
        std::string temp;
        temp += std::to_string(_val);
        temp += FIELD_SEPARATOR;
        temp += std::to_string(_errno);

        return temp;
    }

    operator bool() { return _state; }

    void print()
    {
        std::cout << "_val: " << _val << "  " << "_errno: " << _errno << std::endl;
    }

private:
    int _val;
    int _errno;
    bool _state;
};


```

我们来测试一下

```cpp
#include"Protocols.hpp"
#include<iostream>

using namespace std;

int main()
{
    expression e(100, '+', 300);
    cout<<static_cast<string>(e)<<endl;
    result r(400, 0);
    cout<<static_cast<string>(r)<<endl;
    return 0;
}
```

```shell
[wind@starry-sky ALP]$ g++ test.cc
[wind@starry-sky ALP]$ ./a.out
100 + 300
400 0
[wind@starry-sky ALP]$
```

可以看到, 效果还是不错的.     但我们朝网络里发的并不是这些,  而是要经过进一步的加工. 为什么不能这样发呢? 因为很明显, 如果对方收到两个报文, 比如`"100 + 300100 + 300"`又或者`"400 0400 0"`, 很明显, 这不太好, 所以首先我们要在负载末尾再加一个负责分割报文的特殊字符, 这个特殊字符只需要保证不在有效内容里出现就行,  比如, 不可显字符就不错, 因为键盘上敲不出不可显字符, 所以显然更加安全, 不过我们这里就先不用不可显字符, 为了方便观察调试, 我们可以先使用`"\n"`, 也就是换行符作为这个特殊字符.   

其次我们还可以再负载前面再加些内容, 比如,  我们可以再加个字符串描述有效内容的字节数, 就像`"10 + 20\n"`, 这里, 这个长度就是`7`, 注意, 末尾的换行符是不算在这个长度里的, 除此之外, 我们还可以加个协议版本号, 一方面表示协议的老旧, 以使得服务器可以针对性的进行解析, 另一方面, 版本号也可以描述协议的功能, 这里我们的计算机只支持整数的加减乘除, 如果将来要引入更多数据, 更多计算方式, 那就或许要交给别的协议类进行处理.

不过我们这里就不搞这么复杂了, 我们只要一个有效长度就行了, 另外很明显, `"710 + 20\n"`, 也不太好, 所以我们可以用个特殊字符, 比如还是`"\n"`把它们分割开来`"7\n10 + 20\n"`,  这就是我们在报头层的序列化标准(报头不一定在报文前面, 末尾的`"\n"`也属于报头)

 我们先写`pack`, `pack`表示封装, 在这里其实就是表示加报头, 这个是完全在自己本地完成的, 没有网络参与, 所以直接正常转就可以了.

```cpp
#define FIELD_SEPARATOR ' '           // 负载中字段间的分隔符
#define HEADER_PAYLOAD_SEPARATOR '\n' // 报头和负载间的分隔符
#define MESSAGE_SEPARATOR '\n'        // 报文间的分隔符

std::string pack(const std::string &message)
{
    std::string temp;
    temp += std::to_string(message.size());
    temp += HEADER_PAYLOAD_SEPARATOR;
    temp += message;
    temp += MESSAGE_SEPARATOR;

    return std::move(temp);
}
```

我们来测试一下

```cpp
int main()
{
    expression e(100, '+', 300);
    cout<<pack(e);
    result r(400, 0);
    cout<<pack(r);
    return 0;
}
```

```shell
[wind@starry-sky ALP]$ ./a.out
9
100 + 300
5
400 0
[wind@starry-sky ALP]$
```

对于`unpack`, 也就是解包, 因为有网络参与, 所以就有可能收到残缺的, 甚至是错误格式的, 还是按照之前的那种方式, 我们使用返回值的方式来判断报文是否残缺, 以异常的方式判断格式是否错误, 之所以这样设计, 是因为异常的性能开销比返回值大上一些, 所以对于常见的报文残缺, 我们用返回值, 而对于不常见的格式错误, 我们再用异常, 而且异常它也能附带很多信息, 可以事后作为我们判断格式错误的原因.

```cpp
bool unpack(std::string& in, std::string* out)
{
    size_t pos = in.find(HEADER_PAYLOAD_SEPARATOR);
    if(pos == std::string::npos) return false;
    std::string len_ = in.substr(0, pos);
    int len = stoi(len_);
    int should = len + sizeof(MESSAGE_SEPARATOR);
    std::string payload = in.substr(pos + 1, should);
    if(payload.size() < should)
        return false;

    // 存在足够长度的报文
    // 将这部分字符串移除

    in.erase(0, pos + sizeof(HEADER_PAYLOAD_SEPARATOR)+ should);
    *out = payload;

    return true;
}
```

我们测试一下

```cpp
int main()
{
    expression e(100, '+', 300);
    result r(400, 0);
    auto e_str_in = pack(e);
    auto r_str_in = pack(r);
    auto in = e_str_in + r_str_in; // 模拟多个报文相连的情况

    // 网络传输

    string e_str_out, r_str_out;
    if(unpack(in, &e_str_out))
        cout << e_str_out;
    if(unpack(in, &r_str_out))
        cout << r_str_out;

    return 0;
}
```

```shell
[wind@starry-sky ALP]$ g++ test.cc
[wind@starry-sky ALP]$ ./a.out
100 + 300
400 0
[wind@starry-sky ALP]$
```

接下来我们进行最后一步测试, 那就是把负载重新实例化为对象, 为了方便起见, 这里在`class expression`和`class result`加个打印函数, 查看一下里面的成员.

```cpp
int main()
{
    expression e(100, '+', 300);
    result r(400, 0);
    auto e_str_in = pack(e);
    auto r_str_in = pack(r);
    auto in = e_str_in + r_str_in; // 模拟多个报文相连的情况

    // 网络传输

    string e_str_out, r_str_out;
    expression e_; result r_;
    if(unpack(in, &e_str_out))
    {
        expression temp(e_str_out);
        if(temp) e_ = temp;
    }
    if(unpack(in, &r_str_out))
    {
        result temp(r_str_out);
        if(temp) r_ = temp;
    }

    if(e_) e_.print();
    if(r_) r_.print();

    return 0;
}
```

```shell
[wind@starry-sky ALP]$ g++ test.cc
[wind@starry-sky ALP]$ ./a.out
_x: 100  _y: 300  _op: +
_val: 400  _errno: 0
[wind@starry-sky ALP]$
```

我们看到, 是没有问题的.         不过我们看到, 因为用的是构造函数, 所以构造出来的对象生命周期较短, 需要进行赋值, 为此, 我们可以专门再写一个`init`成员函数, 这样就不需要拷贝了.

```cpp
int main()
{
    expression e(100, '+', 300);
    result r(400, 0);
    auto e_str_in = pack(e);
    auto r_str_in = pack(r);
    auto in = e_str_in + r_str_in; // 模拟多个报文相连的情况

    // 网络传输

    string e_str_out, r_str_out;
    expression e_; result r_;
    if(unpack(in, &e_str_out))
    {
       e_.init(e_str_out);
    }
    if(unpack(in, &r_str_out))
    {
        r_.init(r_str_out);
    }

    if(e_) e_.print();
    if(r_) r_.print();

    return 0;
}
```

```shell
[wind@starry-sky ALP]$ g++ test.cc
[wind@starry-sky ALP]$ ./a.out
_x: 100  _y: 300  _op: +
_val: 400  _errno: 0
[wind@starry-sky ALP]$
```

接下来我们在再`class expression`里加个仿函数, 用来进行计算, 返回一个`struct result`对象, 这样的话, 要把`class expression`和`class result`的位置调换一下.

```cpp
#define DIVISION_BY_ZERO      1       // 除零错误
#define UNSUPPORTED_OPERATION 2       // 不支持的运算

result operator()()
{
    int val_ = 0;
    int erron_ = 0;
    switch(_op)
    {
        case '+' : val_ = _x + _y; break;
        case '-' : val_ = _x - _y; break;
        case '*' : val_ = _x * _y; break;
        case '/' : if(_y == 0) erron_ = DIVISION_BY_ZERO; break;
        default: erron_ = UNSUPPORTED_OPERATION; break;
    }

    result temp(val_, erron_);
    return std::move(temp);
}
```

下面我们写一个计算器类, 提供一个仿函数, 传入一份计算请求的报文, 输出一个计算恢复的报文.

```cpp
#include"Protocols.hpp"

class calculator
{
    public:
    std::string operator()(std::string& package)
    {
        std::string payload; expression t;
        if(!unpack(package, &payload)) return "";
        t.init(payload); if(!t) return "";
        auto e = t();
        return std::move(pack(e));
    }
}
```

尽管代码量很少, 但我们还是不建议直接把这份代码写到`class tcpserver`, 那样耦合度就比较高, `class calculator`, 是提供具体服务的类, 应该让它独立于`class tcpserver`存在, 

本来接下来应该是在`class tcpserver`里定义一个`class calculator`用来提供具体服务, 但是, 我们认为, 这种形式的耦合度也比较高, 我们想要达到的效果是让`class tcpserver`完全看不到提供具体服务的对象, 为此, 我们可以在其中引入一个回调函数模块, `tcpserver`只要把接收到的数据往回调函数里面扔就行了, 如果返回非空字符串, 就发回去, 如果遇到非空, 说明可能是报文残缺, 需要再等一下, 如果这个模块抛出异常, 说明报文格式错误, 就可以直接跳过了,.

```cpp
class tcpserver
{
    typedef std::function<std::string(std::string &)> func;

    public:
    template<class func_>
        tcpserver(func_ callback, uint16_t port = 8888) : _callback(callback), _port(port) {}
    void init()
    {
        _listensock.create_();
        _listensock.bind_(_port);
        _listensock.listen_();
    }

    void run()
    {
        print_netstat();
        signal(SIGCHLD, SIG_IGN);
        while (true)
        {
            std::string clientip;
            uint16_t port;
            int sockfd = _listensock.accept_(&clientip, &port);
            if (sockfd == -1)
                continue;

            if (fork() == 0)
            {
                _listensock.close_();
                std::string inbuffer_stream;
                while (true)
                {
                    char buffer[128];
                    ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
                    if (n > 0)
                    {
                        buffer[n] = 0;
                        inbuffer_stream += buffer;
                        std::string out_stream = _callback(inbuffer_stream);
                    }
                }

                exit(0);
            }
            close(sockfd);
        }
    }

    private:
    socket_ _listensock;
    Log &_log = Log::getInstance();
    func _callback;
    uint16_t _port;
};

int main()
{
    calculator cal;
    tcpserver s(cal);
    s.init();
    s.run();
    return 0;
}
```

提供具体服务的对象, 只在`class tcpserver`的调用处可以看到.   

如果报文有效, `_callback`就会返回一个有实际内容的字符串, 此时, 我们就可以把收到的字符串再写到网络中.  另外需要注意的是, 如果报文的格式严重错误, `_callback`有可能抛异常, 尽管这是子进程, 影响不到父进程, 但出于严谨亦或者日志角度来看, 我们也给它加一个捕获.     至于`write`, 我们就默认成功.

所以完整的子进程逻辑应该是这样的

```cpp
class tcpserver
{
    typedef std::function<std::string(std::string &)> func;

public:
    template <class func_>
    tcpserver(func_ callback, uint16_t port = 8888) : _callback(callback), _port(port) {}
    void init()
    {
        _listensock.create_();
        _listensock.bind_(_port);
        _listensock.listen_();
    }

    void run()
    {
        print_netstat();
        signal(SIGCHLD, SIG_IGN);
        while (true)
        {
            std::string clientip;
            uint16_t port;
            int sockfd = _listensock.accept_(&clientip, &port);
            if (sockfd == -1)
                continue;

            if (fork() == 0)
            {
                _log(Info, "获取到一个新的连接");
                _listensock.close_();
                std::string inbuffer_stream;
                try
                {
                    while (true)
                    {
                        char buffer[128];
                        ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
                        if (n > 0)
                        {
                            buffer[n] = 0;
                            inbuffer_stream += buffer;
                            _log(Debug, "接收到的数据为: %s", inbuffer_stream.c_str());

                            std::string out_stream = _callback(inbuffer_stream);

                            if (out_stream.empty())
                                continue;

                            write(sockfd, out_stream.c_str(), out_stream.size());
                            break;
                        }
                        else if (n == 0)
                        {
                            //_log(Info, "User disconnected");
                        }
                        else
                        {
                            _log(Warning, "read error: %s", strerror(errno));
                            break;
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    _log(Warning, e.what());
                }
                _log(Info, "连接关闭");
                close(sockfd);
                exit(0);
            }

            close(sockfd);
        }
    }

private:
    socket_ _listensock;
    Log &_log = Log::getInstance();
    func _callback;
    uint16_t _port;
};
```

我们先用`telnet`测一下

```shell
[wind@starry-sky ALP]$ telnet 127.0.0.1 8888
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
^]
telnet> 
5
1 + 1
3
2 0
Connection closed by foreign host.
[wind@starry-sky ALP]$


[wind@starry-sky ALP]$ ./servercal
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 127.0.0.1:41067         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      11343/./servercal   
tcp        0      0 127.0.0.1:44633         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:43836         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 127.0.0.1:36477         0.0.0.0:*               LISTEN      2062/code-ddc367ed5 
tcp6       0      0 :::111                  :::*                    LISTEN      -                   
tcp6       0      0 :::22                   :::*                    LISTEN      -                   
tcp6       0      0 ::1:25                  :::*                    LISTEN      -                   
[Info][2025-3-30 17:49:24]::获取到一个新的连接
[Debug][2025-3-30 17:49:28]::接收到的数据为: 5

[Debug][2025-3-30 17:49:31]::接收到的数据为: 5
1 + 1

[Info][2025-3-30 17:49:31]::连接关闭


```

可以看到这个效果还是可以的, 不过有一个很麻烦的事, 那就是每次运算过后它就自动断开了, 因为下意识把子进程逻辑写成短服务了, 问题也不大, 我们把子进程里面的`write`的`break`关掉就行了.这样就是长服务了. 方便我们连续测试

```shell
[wind@starry-sky ALP]$ telnet 127.0.0.1 8888
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
^]
telnet> 
5
1 + 1
3
2 0
7
Connection closed by foreign host.
[wind@starry-sky ALP]$ telnet 127.0.0.1 8888
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
^]
telnet> 
7
10 + 20
4
30 0
5
Connection closed by foreign host.
[wind@starry-sky ALP]$ telnet 127.0.0.1 8888
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
^]
telnet> 
5
1 = 1
3
0 2

Connection closed by foreign host.
[wind@starry-sky ALP]$



[wind@starry-sky ALP]$ ./servercal
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 127.0.0.1:41067         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      12103/./servercal   
tcp        0      0 127.0.0.1:44633         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:43836         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 127.0.0.1:36477         0.0.0.0:*               LISTEN      2062/code-ddc367ed5 
tcp6       0      0 :::111                  :::*                    LISTEN      -                   
tcp6       0      0 :::22                   :::*                    LISTEN      -                   
tcp6       0      0 ::1:25                  :::*                    LISTEN      -                   
[Info][2025-3-30 17:56:28]::获取到一个新的连接
[Debug][2025-3-30 17:56:31]::接收到的数据为: 5

[Debug][2025-3-30 17:56:37]::接收到的数据为: 5
1 + 1

[Debug][2025-3-30 17:56:54]::接收到的数据为: 
7

[Warning][2025-3-30 17:56:54]::stoi
[Info][2025-3-30 17:56:54]::连接关闭
[Info][2025-3-30 17:57:38]::获取到一个新的连接
[Debug][2025-3-30 17:57:43]::接收到的数据为: 7

[Debug][2025-3-30 17:57:47]::接收到的数据为: 7
10 + 20

[Debug][2025-3-30 17:57:51]::接收到的数据为: 
5

[Warning][2025-3-30 17:57:51]::stoi
[Info][2025-3-30 17:57:51]::连接关闭
[Info][2025-3-30 17:58:21]::获取到一个新的连接
[Debug][2025-3-30 17:58:26]::接收到的数据为: 5

[Debug][2025-3-30 17:58:28]::接收到的数据为: 5
1 = 1

[Debug][2025-3-30 17:58:48]::接收到的数据为: 


[Warning][2025-3-30 17:58:48]::stoi
[Info][2025-3-30 17:58:48]::连接关闭


```

我们看到这个好像还是不能连续测试, 这可能是`telnet`它自己又加了些字符, 所以造成了识别错误, 对于这种情况, 我们只能去写客户端解决了.

尽管在这里我们测不出多个报文连发的效果, 但可以看到的是, 当报文残缺时, `out_stream`就是空的, 此时它就会`continue`, 去尝试接收更多字符把报文填满.  当格式错误时, `callback`就会抛异常, 子进程因此就退出了, 

下面我们来写客户端, 客户端我们就不像服务端那样明着写那么多层了, 就直接写了.  不过由于客户端要获取原始文件表下标, 所以我们加个`operator int`转换.

为了便于服务端可以连续启动, 从而快速测试, 在`class socket_`里加了一个新的复用端口地址的成员函数

```cpp
void reuse_port_address()
{
    int opt = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        _log(Warning, "set socket error: %s", strerror(errno));
    }
}
```

`class result`的`print`也略作修改, 引入了`\#define RELIABLE        0`, 并修改了打印方式

```cpp
void print()
{
    std::cout << "_val: " << _val << "  " << "_errno: ";
    if(_errno == DIVISION_BY_ZERO)
        std::cout << "DIVISION_BY_ZERO";
    else if(_errno == UNSUPPORTED_OPERATION)
        std::cout << "UNSUPPORTED_OPERATION";
    else if(_errno == RELIABLE)
        std::cout << "RELIABLE";
    std::cout<<std::endl;
}
```

下面的客户端代码主逻辑是我写的, 打印是基于我的原始打印让AI修改的, 改了之后更好看

```cpp
int main()
{
    socket_ sockfd;
    sockfd.create_();
    sockfd.connect_(8888, "120.55.90.240");

    srand(time(nullptr));
    const int count = 5;
    const string ops("+-*/%&");

    // 统一输出格式
    cout << "=========================== 测试开始 ===========================\n" << endl;

    for (int i = 0; i < count; ++i)
    {
        cout << "------------------------ 测试 " << i + 1 << " ------------------------" << endl;

        // 生成随机数和运算符
        int x = rand() % 100;
        int y = rand() % 100;
        char op = ops[rand() % ops.size()];

        // 创建表达式并打印
        expression temp(x, op, y);
        cout << "运算表达式: ";
        temp.print();

        // 打包消息并发送
        auto message = pack(temp);
        cout << "发出报文: " << endl;
        cout << "-------------------------- 原始报文 --------------------------" << endl;
        cout << message << endl;

        ssize_t len = 0;
        // 防止客户端缓冲区放不下大字符串, 而写缺失, 对于本项目, 实际不可能有这种情况
        while (true)
        {
            len += write(sockfd, message.c_str() + len, message.size() - len);
            if (static_cast<size_t>(len) == message.size()) break;
        }

        // 这里我们也不一定读到完整报文  但为了图省事就不这么严谨了, 默认成功
        char buffer[1024];
        len = read(sockfd, buffer, sizeof(buffer) - 1);
        buffer[len] = 0;
        string out_stream(buffer);
        string end;
        unpack(out_stream, &end);
        result r(end);

        // 打印结果
        cout << "------------------------- 计算结果 --------------------------" << endl;
        r.print();
        cout << "============================== 完成 =============================" << endl;
        cout << endl;

        usleep(400000);  // 休眠0.4秒
    }

    sockfd.close_();
    cout << "=========================== 测试结束 ===========================\n";
    return 0;
}
```

```shell
[wind@starry-sky ALP]$ ./clientcal
=========================== 测试开始 ===========================

------------------------ 测试 1 ------------------------
运算表达式: _x: 92  _y: 10  _op: &
发出报文: 
-------------------------- 原始报文 --------------------------
7
92 & 10

------------------------- 计算结果 --------------------------
_val: 0  _errno: UNSUPPORTED_OPERATION
============================== 完成 =============================

------------------------ 测试 2 ------------------------
运算表达式: _x: 88  _y: 86  _op: &
发出报文: 
-------------------------- 原始报文 --------------------------
7
88 & 86

------------------------- 计算结果 --------------------------
_val: 0  _errno: UNSUPPORTED_OPERATION
============================== 完成 =============================

------------------------ 测试 3 ------------------------
运算表达式: _x: 19  _y: 47  _op: +
发出报文: 
-------------------------- 原始报文 --------------------------
7
19 + 47

------------------------- 计算结果 --------------------------
_val: 66  _errno: RELIABLE
============================== 完成 =============================

------------------------ 测试 4 ------------------------
运算表达式: _x: 11  _y: 52  _op: -
发出报文: 
-------------------------- 原始报文 --------------------------
7
11 - 52

------------------------- 计算结果 --------------------------
_val: -41  _errno: RELIABLE
============================== 完成 =============================

------------------------ 测试 5 ------------------------
运算表达式: _x: 78  _y: 62  _op: +
发出报文: 
-------------------------- 原始报文 --------------------------
7
78 + 62

------------------------- 计算结果 --------------------------
_val: 140  _errno: RELIABLE
============================== 完成 =============================

=========================== 测试结束 ===========================
[wind@starry-sky ALP]$ 
```

现在我们把客户端代码稍微改一下, 一次性输入多份报文

```cpp
int main()
{
    socket_ sockfd;
    sockfd.create_();
    sockfd.connect_(8888, "120.55.90.240");

    srand(time(nullptr));
    const int count = 5;
    const string ops("+-*/%&");

    // 统一输出格式
    cout << "=========================== 测试开始 ===========================\n" << endl;

    for (int i = 0; i < count; ++i)
    {
        cout << "------------------------ 测试 " << i + 1 << " ------------------------" << endl;

        // 生成随机数和运算符
        int x = rand() % 100;
        int y = rand() % 100;
        char op = ops[rand() % ops.size()];

        // 创建表达式并打印
        expression temp(x, op, y);
        cout << "运算表达式: ";
        temp.print();

        // 打包消息并发送
        auto message = pack(temp);
        message += message;
        cout << "发出报文: " << endl;
        cout << "-------------------------- 原始报文 --------------------------" << endl;
        cout << message << endl;

        ssize_t len = 0;
        while (true)
        {
            len += write(sockfd, message.c_str() + len, message.size() - len);
            if (static_cast<size_t>(len) == message.size()) break;
        }

        // 读取返回结果
        char buffer[1024];
        len = read(sockfd, buffer, sizeof(buffer) - 1);
        buffer[len] = 0;
        string out_stream(buffer);
        string end;
        unpack(out_stream, &end);
        result r(end);

        // 打印结果
        cout << "------------------------- 计算结果 --------------------------" << endl;
        r.print();
        cout << "============================== 完成 =============================" << endl;
        cout << endl;

        usleep(400000);  // 休眠0.4秒
    }
    sockfd.close_();
    cout << "=========================== 测试结束 ===========================\n";
    return 0;
}
```

服务端也改一改, 我们就不看客户端效果, 只看服务端了

```cpp
void run()
{
    print_netstat();
    signal(SIGCHLD, SIG_IGN);
    while (true)
    {
        std::string clientip;
        uint16_t port;
        int sockfd = _listensock.accept_(&clientip, &port);
        if (sockfd == -1)
            continue;

        if (fork() == 0)
        {
            _log(Info, "获取到一个新的连接");
            _listensock.close_();
            std::string inbuffer_stream;
            try
            {
                while (true)
                {
                    char buffer[128];
                    ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
                    if (n > 0)
                    {
                        buffer[n] = 0;
                        inbuffer_stream += buffer;
                        _log(Debug, "接收到的数据为: %s", inbuffer_stream.c_str());

                        std::string out_stream = _callback(inbuffer_stream);


                        if (out_stream.empty())
                            continue;

                        _log(Debug, "发回的数据为: %s", out_stream.c_str());

                        write(sockfd, out_stream.c_str(), out_stream.size());
                        // break;
                    }
                    else if (n == 0)
                    {

                    }
                    else
                    {
                        _log(Warning, "read error: %s", strerror(errno));
                        break;
                    }
                }
            }
            catch (const std::exception &e)
            {
                _log(Warning, e.what());
            }
            _log(Info, "连接关闭");
            close(sockfd);
            exit(0);
        }

        close(sockfd);
    }
}
```

```shell
wind@starry-sky ALP]$ ./servercal
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 127.0.0.1:41067         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      27806/./servercal   
tcp        0      0 127.0.0.1:44633         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:43836         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 127.0.0.1:36477         0.0.0.0:*               LISTEN      2062/code-ddc367ed5 
tcp6       0      0 :::111                  :::*                    LISTEN      -                   
tcp6       0      0 :::22                   :::*                    LISTEN      -                   
tcp6       0      0 ::1:25                  :::*                    LISTEN      -                   
[Info][2025-3-30 20:22:20]::获取到一个新的连接
[Debug][2025-3-30 20:22:20]::接收到的数据为: 7
93 + 86
7
93 + 86

[Debug][2025-3-30 20:22:20]::发回的数据为: 5
179 0

[Debug][2025-3-30 20:22:21]::接收到的数据为: 7
93 + 86
7
78 % 98
7
78 % 98

[Debug][2025-3-30 20:22:21]::发回的数据为: 5
179 0

[Debug][2025-3-30 20:22:21]::接收到的数据为: 7
78 % 98
7
78 % 98
7
46 & 48
7
46 & 48

[Debug][2025-3-30 20:22:21]::发回的数据为: 3
0 2

[Debug][2025-3-30 20:22:21]::接收到的数据为: 7
78 % 98
7
46 & 48
7
46 & 48
7
29 + 17
7
29 + 17

[Debug][2025-3-30 20:22:21]::发回的数据为: 3
0 2

[Debug][2025-3-30 20:22:22]::接收到的数据为: 7
46 & 48
7
46 & 48
7
29 + 17
7
29 + 17
7
94 / 99
7
94 / 99

[Debug][2025-3-30 20:22:22]::发回的数据为: 3
0 2

```

我们看到最开始有两个`93 + 86`, 但后来就变成一个了, 并且, 服务端它确实发回了两份`179 0`, 这说明我们的服务端是可以应对多份报文同时发来的情况的. 至于后面发不回去是因为客户端已经关闭了, 所以发不回去了, 不是我们服务端的效果.

当然, 我们上面的网络计算器还有很多缺点, 比如可能会溢出, 报文实际上已经是错的, 但没触发异常... 等等, 这些问题在此我们就不深究了. 

-----------------------

在上面的代码中, 我们用的都是自定义序列化和反序列化, 实际上, 市面上已经有成熟的序列化和反序列化工具, 比如`json, protobuf`, `json`是面向字节流的, `protobuf`是面向二进制流的, 我们在这里只稍微介绍一下`json`. 

`json`是一种独立于编程语言的数据格式适配器, 它可以把其他数据转变为字符串, 比如

```json
{
    "firstName": "John",
    "lastName": "Smith",
    "sex": "male",
    "age": 25,
    "address": 
    {
        "streetAddress": "21 2nd Street",
        "city": "New York",
        "state": "NY",
        "postalCode": "10021"
    },
    "phoneNumber": 
    [
        {
            "type": "home",
            "number": "212 555-1234"
        },
        {
            "type": "fax",
            "number": "646 555-4567"
        }
    ]
}
```

上面的就是一个字符串, 只不过换行符比较多, 它的内部形式主要是`k-v`结构, 相应你也可以直观感受到, `json`可读性很强. 

如果我们C++想用`json`, 就要安装相应的库, 对于`Centos`来说, 安装指令为`sudo yum install -y jsoncpp-devel`, 我们之前学过第三方库, 在安装成功之后, 我们就可以在系统的特定路径下找到其对应的头文件和库文件

```shell
[wind@starry-sky ALP]$ ls /usr/include/jsoncpp -dl
drwxr-xr-x 3 root root 4096 Mar 29 17:34 /usr/include/jsoncpp
[wind@starry-sky ALP]$ ls /usr/include/jsoncpp/json
assertions.h  autolink.h  config.h  features.h  forwards.h  json.h  reader.h  value.h  version.h  writer.h
[wind@starry-sky ALP]$ ls /lib64/libjsoncpp.so
/lib64/libjsoncpp.so
[wind@starry-sky ALP]$ ls /lib64/libjsoncpp.so -l
lrwxrwxrwx 1 root root 15 Mar 29 17:34 /lib64/libjsoncpp.so -> libjsoncpp.so.0
[wind@starry-sky ALP]$ 
```

`json`里面有个数据格式适配器类型, `Json::Value`, 由它实例出的对象, 其内部是由若干个`k-v`结构构成的集合, 对于我们C++,可以认为, 它内部重载了`[]`, 可以通过`[]`添加新的`pair`, 需要注意的是, 这里面的`k`都是字符串类型, 而`v`可以是任意可被转为字符串的类型, 如果要把`Json::Value`序列化, 需要借助于`Josn`里的`FastWriter`或`StyledWriter`实例化对象, 通过调用`write`接口得到序列化后的字符串

```cpp
#include<iostream>
#include<jsoncpp/json/json.h>

using namespace std;

int main()
{
    Json::Value root;
    root["x"] = 100;
    root["y"] = 200;
    root["op"] = "+";
    root["desc"] = "this ia a + oper";

    //Json::StyledWriter w;
    Json::FastWriter w;
    string res = w.write(root);
    cout << res << endl;

    return 0;
}

```

```shell
wind@starry-sky ALP]$ g++ test.cc -ljsoncpp
[wind@starry-sky ALP]$ ./a.out
{"desc":"this ia a + oper","op":"+","x":100,"y":200}

[wind@starry-sky ALP]$
```

`json`的反序列化需要`Json::Value`和`Json::Reader`的配合, 通过`Json::Reader`中的`parse`方法, 将字符串重新转为`Value`对象.不过此时`Value`中的`k-v`还是`字符串-字符串`的形式, 要提取其中的`v`, 需要使用合适的转换接口.

```cpp
int main()
{
    Json::Value root;
    root["x"] = 100;
    root["y"] = 200;
    root["op"] = "+";
    root["desc"] = "this ia a + oper";

    Json::StyledWriter w;
    // Json::FastWriter w;
    string res = w.write(root);
    cout << res;

    Json::Value v;
    Json::Reader r;
    r.parse(res, v);

    int x = v["x"].asInt();
    int y = v["y"].asInt();
    string op = v["op"].asString();
    std::string s = v["desc"].asString();

    cout << x <<endl;
    cout << y <<endl;
    cout << op <<endl;
    cout << s <<endl;

    return 0;
}
```

```shell
[wind@starry-sky ALP]$ ./a.out
{
   "desc" : "this ia a + oper",
   "op" : "+",
   "x" : 100,
   "y" : 200
}
100
200
+
this ia a + oper
[wind@starry-sky ALP]$
```

 另外, `Value`里面是可以再放`Value`的, 对于我们来说, 目前知道这些就够了,

```cpp
int main()
{
    Json::Value set;
    set["t"] = "hello";

    Json::Value root;
    root["x"] = 100;
    root["y"] = 200;
    root["op"] = "+";
    root["desc"] = "this ia a + oper";
    root["te"] = set;

    Json::StyledWriter w;
    // Json::FastWriter w;
    string res = w.write(root);
    cout << res;

    Json::Value v;
    Json::Reader r;
    r.parse(res, v);

    int x = v["x"].asInt();
    int y = v["y"].asInt();
    string op = v["op"].asString();
    std::string s = v["desc"].asString();
    Json::Value temp = v["te"];
    string test = temp["t"].asString();

    cout << x <<endl;
    cout << y <<endl;
    cout << op <<endl;
    cout << s <<endl;
    cout << test << endl;

    return 0;
}
```

```shell
[wind@starry-sky ALP]$ ./a.out
{
   "desc" : "this ia a + oper",
   "op" : "+",
   "te" : {
      "t" : "hello"
   },
   "x" : 100,
   "y" : 200
}
100
200
+
this ia a + oper
hello
[wind@starry-sky ALP]$ 
```

下面, 我们使用`json`对我们的协议类序列和反序列接口进行重写

```cpp
class result
{
public:
    result() : _val(0), _errno(0), _state(false) {}
    result(const int &val_, const int &errno_) : _val(val_), _errno(errno_), _state(true) {}
    result(const std::string &s) : _state(false) { init(s); }

    bool init(const std::string &s)
    {
#ifdef SERIALIZATION_MANUAL
        size_t pos = s.find(FIELD_SEPARATOR);
        if (pos == std::string::npos)
            return *this;
        std::string val_ = s.substr(0, pos);
        std::string errno_ = s.substr(pos + 1);
        _val = stoi(val_);
        _errno = stoi(errno_);

        _state = true;

        return *this;
#else
        Json::Value root;
        Json::Reader r;
        r.parse(s, root);
        _val = root["val"].asInt();
        _errno = root["errno"].asInt();
        _state = true;
        return *this;
#endif
    }

    operator std::string()
    {
#ifdef SERIALIZATION_MANUAL

        std::string temp;
        temp += std::to_string(_val);
        temp += FIELD_SEPARATOR;
        temp += std::to_string(_errno);

        return temp;
#else
        Json::Value root;
        Json::FastWriter w;
        root["val"] = _val;
        root["errno"] = _errno;
        return w.write(root);
#endif
    }

    operator bool() { return _state; }

    void print()
    {
        std::cout << "_val: " << _val << "  " << "_errno: ";
        if (_errno == DIVISION_BY_ZERO)
            std::cout << "DIVISION_BY_ZERO";
        else if (_errno == UNSUPPORTED_OPERATION)
            std::cout << "UNSUPPORTED_OPERATION";
        else if (_errno == RELIABLE)
            std::cout << "RELIABLE";
        std::cout << std::endl;
    }

private:
    int _val;
    int _errno;
    bool _state;
};

class expression
{
public:
    expression() : _x(0), _y(0), _op('+'), _state(false) {};
    expression(const int &x, char op, const int &y)
        : _x(x), _y(y), _op(op), _state(true)
    {
    }

    expression(const std::string &s)
        : _state(false) { init(s); }

    bool init(const std::string &s)
    {
#ifdef SERIALIZATION_MANUAL

        size_t pos1 = s.find(FIELD_SEPARATOR);
        if (pos1 == std::string::npos)
            return *this;
        size_t pos2 = s.find(FIELD_SEPARATOR, pos1 + 1);
        if (pos2 == std::string::npos)
            return *this;
        std::string x = s.substr(0, pos1);
        std::string op = s.substr(pos1 + 1, pos2 - (pos1 + 1));
        std::string y = s.substr(pos2 + 1);
        if (op.size() != 1) // 如果不为一说明格式错误,没有遵守协议, 抛出异常, 该报文已经可以放弃
            throw std::invalid_argument("Illegal parameter format");
        _x = std::stoi(x); // 当参数无法转换时, `stoi`也会抛出异常
        _op = op[0];
        _y = std::stoi(y);

        _state = true; // _state表示是否残缺, 异常表示格式错误

        return *this;

#else
        Json::Value v;
        Json::Reader r;
        r.parse(s, v);
        _x = v["x"].asInt();
        _op = v["op"].asInt();
        _y = v["y"].asInt();
        _state = true;
        return *this;
#endif
    }

    operator std::string()
    {
#ifdef SERIALIZATION_MANUAL
        std::string temp;
        temp += std::to_string(_x);
        temp += FIELD_SEPARATOR;
        temp += _op;
        temp += FIELD_SEPARATOR;
        temp += std::to_string(_y);

        return temp;
#else
        Json::Value root;
        root["x"] = _x;
        root["op"] = _op;
        root["y"] = _y;

        Json::FastWriter w;
        return w.write(root);
#endif
    }

    result operator()()
    {
        int val_ = 0;
        int erron_ = 0;
        switch (_op)
        {
        case '+':
            val_ = _x + _y;
            break;
        case '-':
            val_ = _x - _y;
            break;
        case '*':
            val_ = _x * _y;
            break;
        case '/':
            if (_y == 0)
                erron_ = DIVISION_BY_ZERO;
            else
                val_ = _x / _y;
            break;
        default:
            erron_ = UNSUPPORTED_OPERATION;
            break;
        }

        result temp(val_, erron_);
        return temp;
    }

    operator bool() { return _state; }

    void print()
    {
        std::cout << "_x: " << _x << "  " << "_y: " << _y << "  " << "_op: " << _op << std::endl;
    }

private:
    int _x;
    int _y;
    char _op;
    bool _state;
};
```

测试一下

```shell
[wind@starry-sky ALP]$ ./clientcal
=========================== 测试开始 ===========================

------------------------ 测试 1 ------------------------
运算表达式: _x: 35  _y: 90  _op: *
发出报文: 
-------------------------- 原始报文 --------------------------
24
{"op":42,"x":35,"y":90}


------------------------- 计算结果 --------------------------
_val: 3150  _errno: RELIABLE
============================== 完成 =============================

------------------------ 测试 2 ------------------------
运算表达式: _x: 15  _y: 24  _op: %
发出报文: 
-------------------------- 原始报文 --------------------------
24
{"op":37,"x":15,"y":24}


------------------------- 计算结果 --------------------------
_val: 0  _errno: UNSUPPORTED_OPERATION
============================== 完成 =============================

------------------------ 测试 3 ------------------------
运算表达式: _x: 98  _y: 86  _op: *
发出报文: 
-------------------------- 原始报文 --------------------------
24
{"op":42,"x":98,"y":86}


------------------------- 计算结果 --------------------------
_val: 8428  _errno: RELIABLE
============================== 完成 =============================

------------------------ 测试 4 ------------------------
运算表达式: _x: 19  _y: 49  _op: +
发出报文: 
-------------------------- 原始报文 --------------------------
24
{"op":43,"x":19,"y":49}


------------------------- 计算结果 --------------------------
_val: 68  _errno: RELIABLE
============================== 完成 =============================

------------------------ 测试 5 ------------------------
运算表达式: _x: 18  _y: 52  _op: %
发出报文: 
-------------------------- 原始报文 --------------------------
24
{"op":37,"x":18,"y":52}


------------------------- 计算结果 --------------------------
_val: 0  _errno: UNSUPPORTED_OPERATION
============================== 完成 =============================

=========================== 测试结束 ===========================
[wind@starry-sky ALP]$
```

需要注意的是, 正如再测试中看到的, `json`把单字符当做整型(ASCII)来传的, 所以对面接收的时候要用`asInt()`转换.另外我这里用了条件编译, 因为没有定义宏`SERIALIZATION_MANUAL`, 所以它使用的是`json`方案, 我们也可以再编译选项里加上这个宏, 来快速更换成我们自己的风格.

```shell
[wind@starry-sky ALP]$ cat makefile
CC = g++
CFLAGS = -Wall -ljsoncpp
CLIENT_STD = -std=c++11
SERVER_STD = -std=c++11
CLIENT_SRC = ClientCal.cc
SERVER_SRC = ServerCal.cc
CLIENT_TARGET = clientcal
SERVER_TARGET = servercal

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRC)
	@$(CC) $(CFLAGS) $(CLIENT_STD) $^ -o $@

$(SERVER_TARGET): $(SERVER_SRC)
	@$(CC) $(CFLAGS) $(SERVER_STD) $^ -o $@

clean:
	@rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
[wind@starry-sky ALP]$ 
[wind@starry-sky ALP]$ #编译的时候加上选项-D后面紧跟着宏, 就可以编译时定义宏
[wind@starry-sky ALP]$ g++ -Wall -ljsoncpp -std=c++11 -DSERIALIZATION_MANUAL=1 ServerCal.cc -o servercal
[wind@starry-sky ALP]$ g++ -Wall -ljsoncpp -std=c++11 -DSERIALIZATION_MANUAL=1 ClientCal.cc -o clientcal
[wind@starry-sky ALP]$ ./servercal &
[1] 4353
[wind@starry-sky ALP]$ (Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 127.0.0.1:41067         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      4353/./servercal    
tcp        0      0 127.0.0.1:38425         0.0.0.0:*               LISTEN      31600/code-ddc367ed 
tcp        0      0 127.0.0.1:44633         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:43836         0.0.0.0:*               LISTEN      25811/language_serv 
tcp6       0      0 :::111                  :::*                    LISTEN      -                   
tcp6       0      0 :::22                   :::*                    LISTEN      -                   
tcp6       0      0 ::1:25                  :::*                    LISTEN      -                   
[wind@starry-sky ALP]$ ./clientcal
=========================== 测试开始 ===========================

------------------------ 测试 1 ------------------------
运算表达式: _x: 60  _y: 37  _op: %
发出报文: 
-------------------------- 原始报文 --------------------------
7
60 % 37

[Info][2025-3-31 19:36:42]::获取到一个新的连接
[Debug][2025-3-31 19:36:42]::接收到的数据为: 7
60 % 37

[Debug][2025-3-31 19:36:42]::发回的数据为: 3
0 2

------------------------- 计算结果 --------------------------
_val: 0  _errno: UNSUPPORTED_OPERATION
============================== 完成 =============================

------------------------ 测试 2 ------------------------
运算表达式: _x: 49  _y: 50  _op: /
发出报文: 
-------------------------- 原始报文 --------------------------
7
49 / 50

[Debug][2025-3-31 19:36:42]::接收到的数据为: 7
49 / 50

[Debug][2025-3-31 19:36:42]::发回的数据为: 3
0 0

------------------------- 计算结果 --------------------------
_val: 0  _errno: RELIABLE
============================== 完成 =============================

------------------------ 测试 3 ------------------------
运算表达式: _x: 77  _y: 47  _op: *
发出报文: 
-------------------------- 原始报文 --------------------------
7
77 * 47

[Debug][2025-3-31 19:36:42]::接收到的数据为: 7
77 * 47

[Debug][2025-3-31 19:36:42]::发回的数据为: 6
3619 0

------------------------- 计算结果 --------------------------
_val: 3619  _errno: RELIABLE
============================== 完成 =============================

------------------------ 测试 4 ------------------------
运算表达式: _x: 64  _y: 88  _op: /
发出报文: 
-------------------------- 原始报文 --------------------------
7
64 / 88

[Debug][2025-3-31 19:36:43]::接收到的数据为: 7
64 / 88

[Debug][2025-3-31 19:36:43]::发回的数据为: 3
0 0

------------------------- 计算结果 --------------------------
_val: 0  _errno: RELIABLE
============================== 完成 =============================

------------------------ 测试 5 ------------------------
运算表达式: _x: 27  _y: 51  _op: &
发出报文: 
-------------------------- 原始报文 --------------------------
7
27 & 51

[Debug][2025-3-31 19:36:43]::接收到的数据为: 7
27 & 51

[Debug][2025-3-31 19:36:43]::发回的数据为: 3
0 2

------------------------- 计算结果 --------------------------
_val: 0  _errno: UNSUPPORTED_OPERATION
============================== 完成 =============================

=========================== 测试结束 ===========================
[wind@starry-sky ALP]$ ps ajx | head -1 && ps ajx | grep servercal | grep -v grep
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
32031  4353  4353 32031 pts/7     4377 S     1002   0:00 ./servercal
 4353  4363  4353 32031 pts/7     4377 R     1002   0:11 ./servercal
[wind@starry-sky ALP]$ kill -9 4353 4363
```

除此之外, 你可能注意到了, `json`风格的代码我是没有自己抛异常的, 这是因为`json`无法进行序列化和反序列化会自己抛异常, 所以不用我们操心, 另外, 你也不用担心因为报文残缺而导致误抛异常, 导致一份可能只是残缺, 但格式没有错误的报文被丢弃, 在`class calculator`我们可以看到, 只有过了`unpack`这一关, 它才会进行`init`, 这个`unpack`用的可不是`json`, 而是我们自己写的, 只有报文够长度才会到`init`那一行, 如果在够长度的情况下它仍然抛异常, 那只能说, 它本身就是格式错误的.

```cpp
class calculator
{
    public:
    std::string operator()(std::string& package)
    {
        std::string payload; expression t;
        if(!unpack(package, &payload)) return "";
        t.init(payload); if(!t) return "";
        auto e = t();
        return std::move(pack(e));
    }
};

bool unpack(std::string &in, std::string *out)
{
    size_t pos = in.find(HEADER_PAYLOAD_SEPARATOR);
    if (pos == std::string::npos)
        return false;
    std::string len_ = in.substr(0, pos);
    int len = stoi(len_);
    size_t should = len + sizeof(MESSAGE_SEPARATOR);
    std::string payload = in.substr(pos + 1, should);
    if (payload.size() < should)
        return false;

    // 存在足够长度的报文
    // 将这部分字符串移除

    in.erase(0, pos + strlen(HEADER_PAYLOAD_SEPARATOR) + should);
    *out = payload;

    return true;
}
```

我们再回到理论, 重新谈谈七层`OSI`的概念模型

![image-20250311143430783](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311143431121.png)

在我们上面写的自定义协议中, 像`class tcpserver`这种负责通信管理的, 每次用户发出请求, 开一个线程或者进程的层就是会话层, 新开的执行流实际上就是属于用户的会话, 就像我们登录Linux, Linux为我们创建会话那样.               至于表示层, 它的功能实际上就是把数据转换成各种各样的形式,   上图说的, 设备固有格式到网络标准格式, 实际上就是序列化, 反过来则是反序列化. 所以表示层实际上就是协议负责的内容.        应用层则负责为用户提供具体的服务, 比如我们这里的网络计算功能, 由`class calculator`负责.    应用层, 表示层, 会话层实际上是要根据实际情况进行具体设计的, 所以它们三个写不进系统, 必须要由我们亲自把握. 

好的, 我们的自定义协议现在其实已经写的差不多了, 这份自定义协议可能是我们人生第一次写协议, 也有可能是我们人生中最后几次写协议, 因为对于应用层协议来说, 市面上早就有很多非常成熟的协议了, 比如我们等会儿就说的`http`, 这里我们写协议主要是找找协议的感觉, 便于我们理解其它已经成熟的应用层协议, 这里之所以说是最后几次, 因为等会儿我们说`http`和其他成熟协议时还要手搓一个简易版本.  在这之后, 我们就有可能真的就不会再写协议了, 除非你公司业务比较特殊, 有自己的需求, 才会自己写协议自已用, 比如, 公司有些业务对安全由特殊要求, 可能会不放心用别人的, 可以差一点, 但不能不安全, 所以可能会自己写, 到时候, 就算出问题, 也能找到具体的人算账, 责任划分比较明确, 另外, 对于C++开发来说, 我们的一个方向是游戏开发, 游戏一般来说, 协议真的是自己从无到有一步步写出来的, 这种场景下, 协议还挺常见的. 

哎呀, 刚刚突然看到一个bug, 在`unpack`那个函数里, 计算`HEADER_PAYLOAD_SEPARATOR`长度的应该是`strlen`, 而不是`sizeof`, 源代码我改了, 但上面的文档可能有没改的, 需要注意一下.

## HTTP

针对HTTP的学习, 我们主要有以下几个流程: 一是先介绍一下HTTP的基础知识, 二是通过两个网络工具直观看一下HTTP的请求和响应大概长什么样, 三是带大家写一个简单的HTTP服务.

### 认识URL

在我们之前的代码中, IP和端口几乎无处不在, 但在我们日常生活中, 其实很少真正直接使用IP和端口来指代某台服务器上的具体进程, 而是使用一个叫做域名的东西, 比如https://www.baidu.com中baidu.com就是一份域名, 其中, baidu是主域名, 也就是品牌或实体的名称, .com是顶级域名, 表示商业网站. 域名中其实包含着IP地址, 有专门的组织负责解析, 将其映射到某个IP地址上, 比如, 使用Windows的控制台, 执行`ping`指令, 就可以看到解析出的IP.

```shell
PS C:\Users\21066> ping www.baidu.com

正在 Ping www.a.shifen.com [36.152.44.132] 具有 32 字节的数据:
来自 36.152.44.132 的回复: 字节=32 时间=16ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=18ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=15ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=18ms TTL=52

36.152.44.132 的 Ping 统计信息:
    数据包: 已发送 = 4，已接收 = 4，丢失 = 0 (0% 丢失)，
往返行程的估计时间(以毫秒为单位):
    最短 = 15ms，最长 = 18ms，平均 = 16ms
PS C:\Users\21066>
```

直接使用"36.152.44.132", 便能访问百度主页面

![image-20250402195649574](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250402195649798.png)

不过这个可能因人而异, 有些情况下可能用不了, 不过那并不意味着里面没地址, 而是和各种原因有关, 这里我们就不深究了.

你可能会问, 那端口呢? 端口不必在意, 我们之前说过, 特定的服务对应着特定的端口,  当你把这串IP输到地址栏后, 浏览器会默认其使用的是HTTP协议, HTTP协议使用80这个特定端口作为服务端的端口, 所以即使你没写端口号, 浏览器也能通过协议名把端口号加上.  

不过我们平常用的链接也没这么直白, 往往还有其他内容, 比如我们这边随手点一篇腾讯主页的文章   https://news.qq.com/rain/a/20250402A01QIZ00

![image-20250402202521931](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250402202522081.png)

这里面还有些别的内容, 不过这个链接还是不太完整, 

![image-20250402203719616](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250402203719671.png)

这个链接被称为URL, 即统一资源定位符, 用来标识网络中的一份特定资源.比如, 文章, 音频, 视频... 它们都是网络资源.端口号可以用协议方案名推出来, 一般省略, 至于那个文件路径, 他前面也有一个`/`, 这个`/`被称为web根目录, 通常就是Linux的根目录, 但也有特殊情况, 我们之后会谈. 至于查询字符串, 是用于推送这种网络行为的, 网络行为总体上就两种, 一种是拉取, 就是把网上东西拿到本地, 除此之外就是推送, 就是把本地东西放网上, 当你推送时, 据需要用到查询字符串, 它实际上是一个kv结构的集合字符串, 最后的那个片段标识符不参与网络行为, 它的作用是定位页面中的某个特定位置, 让页面最开始位于这个位置. 

另外, 我们可以从上面的示例URL中看到, 它中间有一些用来进行字段分割的特殊字符, 比如`@, ? , +, //, : `什么的, 如果用户上传的信息中有这些关键字就会被编码成其他内容, 从而防止其影响URL的解析.

http://36.152.44.132/s?ie=utf-8&f=8&rsv_bp=1&rsv_idx=1&tn=baidu&wd=hello%20world&fenlei=256&rsv_pq=0xf524239f011e77fe&rsv_t=2e45mn%2BNozluV9%2FmM1X5M2ttO87SC22jDVYLAZYxpi3Bf%2FiwGF2wrXeMbMo9&rqlang=en&rsv_dl=tb&rsv_enter=1&rsv_sug3=12&rsv_sug1=11&rsv_sug7=100&rsv_sug2=0&rsv_btype=i&prefixsug=hello%2520world&rsp=6&inputT=6702&rsv_sug4=7944

比如, 这里我们在百度的搜索栏中搜索"hello world", 就会有如上URL, 其中就有"hello%20world"这种字符串, 空格是个特殊字符, 所以被编码了, 变成了`%20`, 这个20是`%`字符的ASCII, 不过是十六进制的, 如果字符不能用ASCII表示, 比如中文这种宽字符, 先要把它, 如汉字"中"拆成合适的段(八个比特位作为一个段), `E4 B8 AD`, 之间用`%`相隔.

对于编码的方法, 我们一般在网上复制一份现成的代码, 不过实际上, 我们一般也用不上.网上也有一些在线工具, 

### HTTP的请求和响应

HTTP的请求由多行构成, 其中的每一行都以`\r\n`为结尾(为字段分隔符), 第一行被叫做请求行, 下面的若干行被称为请求报头, 其中有许多kv形式的pair, 用于描述该请求的描述信息, 末尾是用户上传内容(正文), 这个不一定以`\r\n`为行结尾, 完全取决于用户到底传的是什么. 

![image-20250402214135572](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250402214135688.png)

对于请求行来说, 它有三个成员, 成员之间以空格分割, 第一个成员描述了该请求的方法, 方法很多, 但我们几乎只用其中两种, 负责拉取的GET, 和负责拉取的POST, 接下来是URL, 这个URL一般是域名后面的URL,  然后就是协议版本. 比如HTTP/1.0,  HTTP/2.0

为了区分请求报文和正文, 请求报头的最后一行是个空行, 当服务端把请求报头中的一行截取出来后, 发现, 这是一个空字符, 那就说明,请求报头没内容了, 接下来是正文部分.

另外, 为了保证正文内容不残缺, 请求报文中就有描述正文字节量的pair(形式大概是"Content-Length: xx:), 它用这种方式确保正文完整之后再交给下一层, 就像我们之前请手写的自定义协议那样.

还有, 之前我们写的自定义协议是报文准备完成后, 一次性发过去, 但HTTP一般不是一次性发, 而是一行一行发,

HTTP的响应格式与请求格式大致相同, 由多行构成, 每行以`\r\n`结尾, 第一行被称为"状态行", 后面的若干行为响应报头, 其内容认识kv结构构成的集合, 其中也包含对于正文字节长度的描述, 报头之后是个空行, 之后便是正文, 正文的形式任由实际情况决定.

![image-20250403135059541](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250403135059601.png)

接下来我们用`telnet`抓一份响应报文

```shell
[wind@starry-sky ~]$ telnet www.baidu.com 80
Trying 180.101.49.44...
Connected to www.baidu.com.
Escape character is '^]'.
^]
telnet> 
GET / HTTP/1.1

HTTP/1.1 200 OK
Accept-Ranges: bytes
Cache-Control: no-cache
Connection: keep-alive
Content-Length: 29506
Content-Type: text/html
Date: Thu, 03 Apr 2025 05:56:41 GMT
P3p: CP=" OTI DSP COR IVA OUR IND COM "
P3p: CP=" OTI DSP COR IVA OUR IND COM "
Pragma: no-cache
Server: BWS/1.1
Set-Cookie: BAIDUID=23355F1EDE3363147E9713EC7288B66F:FG=1; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: BIDUPSID=23355F1EDE3363147E9713EC7288B66F; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: PSTM=1743659801; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: BAIDUID=23355F1EDE336314195E7FA559FE391F:FG=1; max-age=31536000; expires=Fri, 03-Apr-26 05:56:41 GMT; domain=.baidu.com; path=/; version=1; comment=bd
Traceid: 174365980106407250029233865127660922360
Vary: Accept-Encoding
X-Ua-Compatible: IE=Edge,chrome=1
X-Xss-Protection: 1;mode=block

<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" />
    <meta content="always" name="referrer" />
    <meta
        name="description"
        content="全球领先的中文搜索引擎、致力于让网民更便捷地获取信息，找到所求。百度超过千亿的中文网页数据库，可以瞬间找到相关的搜索结果。"
    />
........ 后面省略
```

初期我们就看看状态行, 服务器发过来的状态行为

```shell
HTTP/1.1 200 OK

```

形式为   协议版本  状态码  状态码的文字描述     协议版本主要是为了依据客户端的实际情况, 为其提供符合版本的功能数据, 客户端的更新是要逐步进行的, 所以市面上会存在各种各样的客户端版本, 服务端都要兼容它们, 所以要有协议版本.     状态码描述响应的状态, 比如"404"就是我们平常经常见到的状态码, 状态码描述用于用文字描述状态码, 便于进行分析调试. 

对于HTTP来说, 即使是错误的请求(即使索要的资源不存在), 它也会有对应的响应. 

下面我们借助于`fiddler`看看HTTP的请求报文, `fiddler`是个抓包软件, 本来我们的请求是直接发到服务端那里, 但`fiddler`启动后, 就会拦截我们的请求报文, 然后再次包装, 再发给服务端, 服务端的请求, 也先到`fiddler`这里, 然后再由`fiddler`转交给需要的进程.

![image-20250403142909006](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250403142909137.png)

![image-20250403142929312](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250403142929375.png)

不过这实际是HTTPS, 所以用的是443端口, 其内容是被加密的, 所以看不到具体内容.

我们再换一个软件, `Postman`, `fiddler`相当于是代理, `Postman`是自己构建一个请求发给服务端, 它自己就相当于是浏览器. 

![image-20250403143901834](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250403143902028.png)

上面是请求, 下面是响应, 不过它给我们的不是原始请求报文, 而是已经被分割好的, 但对于初学者来说, 我们需要看到原始报文, 所以这点不太好. 下面的是响应, 给了我们正文和渲染效果. 

下面我们写一个简单的HTTP服务.

```cpp
#pragma once

#include"log.hpp"
#include"Sockst.hpp"
#include<string>
#include<pthread.h>
#include<memory>
#include<iostream>

#define DEFAULT_POTR 8888
#define BUFFER_SIZE 10240

struct threadArgs
{
    int _sockfd;

    threadArgs(int sockfd) :_sockfd(sockfd){};
    ~threadArgs() {if(_sockfd > 0) close(_sockfd);}
};

class HttpServer
{
    public:
    HttpServer(uint16_t port = DEFAULT_POTR) :_port(port) {};
    void start()
    {
        _listensock.create_();
        _listensock.reuse_port_address();
        _listensock.bind_(_port);
        _listensock.listen_();
        print_netstat();

        while(true)
        {
            std::string clientip;
            uint16_t clientport;
            int sockfd = _listensock.accept_(&clientip, &clientport);
            if(sockfd == -1) continue;

            pthread_t session; threadArgs* a = new threadArgs(sockfd);
            pthread_create(&session, nullptr, threadRun, a);
        }
    }
    ~HttpServer(){}
    private:
    static void* threadRun(void* args_)
    {
        pthread_detach(pthread_self());  char buffer[BUFFER_SIZE];
        std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs*>(args_));
        ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
        if(n > 0)
        {
            buffer[n] = 0;
            std::cout <<buffer;
        }
        return nullptr;
    }


    private:
    socket_ _listensock;
    uint16_t _port;
    Log& _log = Log::getInstance();
};
```

关于这份代码, 我们用`recv`平替了`read`, `recv`是专门用于读TCP这种面向字节流的传输层协议的. 它和`read`的用法几乎相同, 只不过末尾有个参数决定是否是阻塞读.   

这次我们直接用浏览器访问

![image-20250403162001278](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250403162001419.png)

```shell
[wind@starry-sky HTTP]$ ./HttpServer
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 127.0.0.1:41067         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      22979/./HttpServer  
tcp        0      0 127.0.0.1:44633         0.0.0.0:*               LISTEN      25811/language_serv 
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:46555         0.0.0.0:*               LISTEN      18354/code-ddc367ed 
tcp        0      0 127.0.0.1:43836         0.0.0.0:*               LISTEN      25811/language_serv 
tcp6       0      0 :::111                  :::*                    LISTEN      -                   
tcp6       0      0 :::22                   :::*                    LISTEN      -                   
tcp6       0      0 ::1:25                  :::*                    LISTEN      -                   
GET / HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Upgrade-Insecure-Requests: 1

```

我们看到, 浏览器对服务发了一个请求...

只不过, 由于我们没写请求响应, 所以浏览器说"该网页无法正常运转"

我们略微看一下报文

```shell
[wind@starry-sky HTTP]$ ./HttpServer
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 127.0.0.1:34754         0.0.0.0:*               LISTEN      28325/code-ddc367ed 
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      29702/./HttpServer  
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -                   
tcp6       0      0 :::111                  :::*                    LISTEN      -                   
tcp6       0      0 :::22                   :::*                    LISTEN      -                   
tcp6       0      0 ::1:25                  :::*                    LISTEN      -                   
GET / HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Upgrade-Insecure-Requests: 1

GET http://pingjs.qq.com/ping.js HTTP/1.1
Host: pingjs.qq.com
User-Agent: Mozilla
If-Modified-Since: Thu, 02 Dec 2021 11:17:53 GMT

GET / HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Linux; Android 10; K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Mobile Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Upgrade-Insecure-Requests: 1

```

`GET / HTTP/1.1`中的`GET`不就是协议方法吗, `/`是`Web`根目录, `HTTP/1.1`是协议版本, 第二行`Host`可以定位一个具体服务器上的具体进程, 我们再看看`User-Agent`, 它描述的是客户端的平台, 比如, `Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36`是我用Windows端的Chrome访问的, `Mozilla/5.0 (Linux; Android 10; K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Mobile Safari/537.36`是我用Android端的Chrome访问的, 我手头上没苹果设备.

我们稍微说说这个`User-Agent`到底是干什么用的, 它有两个应用, 一是确认用户的身份是否合法, 对于通过正常渠道访问的客户来说, 它都有这个信息, 但如果是爬虫, 它就一般没有这个信息, 它的身份是非法的, 我的服务端为了防爬, 可以判断一下用户请求报文中的`User-Agent`有没有, 如果有的话, 你再判断一下它看起来像不像假的. 没有或者是假的, 就把它踢出去. 

二是, 服务端依据用户的平台, 进行个性化推荐, 比如, 我们在Windows的百度搜索"微信", 它知道我这是Windows, 所以会为我推荐Windows

![image-20250404162535864](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404162535939.png)

而我们在Android端的百度搜索"微信", 它弹出的就是Android版的.

![0806e7bad23a1673e070207f0cf26ed9](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404162649163.jpg)

我还是没有苹果, 所以就不看了.

这是只是说一说, 这属于常识性知识.

接下来我们来写个响应报文发回去.

```cpp
static void* threadRun(void* args_)
{
    pthread_detach(pthread_self());  char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs*>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    if(n > 0)
    {
        buffer[n] = 0;
        std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = "hello world"; // 发个"hello world"回去
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0);  // 差错处理不做了, send和write的关系l类似于read和recv
    }
    return nullptr;
}
```

![image-20250404165144058](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404165144143.png)

尽管这个"hello world"其貌不扬, 但是我们的第一步.

我们再用`telnet`看看原始报文, 浏览器只显示正文部分, 所以看不到响应行和响应报头

```shell
[whisper@VM-12-6-centos ~]$ telnet 120.55.90.240 8888
Trying 120.55.90.240...
Connected to 120.55.90.240.
Escape character is '^]'.
^]
telnet> 
GET / HTTP/1.1  
HTTP/1.0 200 OK
Content-Length: 11

hello worldConnection closed by foreign host.
```

下面, 我们把正文改一改, 不要只有一个字符串, 太空洞了, 不过我们目前的本职是后端, 正文是前端写的, 所以后面我们的正文主要借助于AI或者网络资源. 我们可以搜索`w3schools`这个网站, 它是一个非常知名的在线学习网站，主要专注于 **网页开发技术** 的教学。它特别适合初学者和中级开发者，尤其是像我们这样的后端开发者，想快速上手一些前端知识时，W3Schools 是个不错的资源。不过它是挪威的网站, 不过我们也有[中文的](https://www.w3school.com.cn/)

![image-20250404170653886](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404170653990.png)

网页是用`HTML`写的.  它实质是文本, 浏览器收到之后, 会依据它里面的描述进行渲染, 然后就有了我们见到的那些页面.

我们先让AI写一份`HTML`, 分析一下`HTML`的大致结构

```html
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>等待界面</title>
    <style>
        /* 居中布局 */
        body {
            margin: 0;
            height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            background-color: #f0f0f0; /* 浅灰背景 */
            font-family: Arial, sans-serif;
        }

        /* 加载动画容器 */
        .loader {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 20px; /* 动画和文字间距 */
        }

        /* 旋转圆圈 */
        .spinner {
            width: 40px;
            加载动画的样式 */
            height: 40px;
            border: 4px solid #3498db; /* 蓝色边框 */
            border-top: 4px solid transparent; /* 透明顶部造成旋转效果 */
            border-radius: 50%;
            animation: spin 1s linear infinite; /* 旋转动画 */
        }

        /* 文字样式 */
        .text {
            color: #333;
            font-size: 18px;
        }

        /* 动画关键帧 */
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
    </style>
</head>
<body>
    <div class="loader">
        <div class="spinner"></div>
        <div class="text">加载中...</div>
    </div>
</body>
</html>
```

最开始的这一行`<!DOCTYPE html>`表明的是`html`标准, 这里是`HTML5`标准, 然后我们可以看到`<html>`, 这是"根标签", 相当于后端的`{}`, 我们看到, 首尾各一个, 所有 HTML 内容都包裹在这个标签里，是整个文档的根。`lang="zh-CN"` 表示页面语言是中文（简体）.   `<head>`叫做"头部", 它里面存放页面的配置，不直接显示在页面上, 比如`<meta charset="UTF-8">`, 表明字符编码为` UTF-8`，这样能让中文字符正常显示, `<meta name="viewport" content="width=device-width, initial-scale=1.0">`, 适配移动设备，让页面宽度等于设备宽度，初始缩放为 1.0, 不写的话手机上可能显示乱. `<title>等待界面</title>`, 设置浏览器标签栏的标题.  `<style> ... </style>`负责控制页面外观, `<body>`就是正文了, 用户真正看的部分. 

HTML 的格式特点是

- **层次结构**：

  - 标签嵌套，像树形结构：<html> 是根，<head> 和 <body> 是分支。
  - 缩进不是必须的，但为了可读性（像我写的这样）通常会缩进。

  **标签对称**：

  - 除了少数自闭合标签（如 <meta>），大部分标签要成对出现。
  - 忘了写结束标签（比如 </div>），可能会导致页面乱掉。

  **属性**：

  - 标签可以带属性，比如` lang="zh-CN"` 或 `class="loader"`，用` = `赋值，值通常加引号。
  - 属性给标签加“特性”，类似函数参数。

这都是AI说的, 我也不清楚.

我们先不整这么多, 先大概写写, 把"hello world"渲染成标题形式.

```cpp
static void* threadRun(void* args_)
{
    pthread_detach(pthread_self());  char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs*>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    if(n > 0)
    {
        buffer[n] = 0;
        std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = "<html><body><h3>hello world</h3></body></html>"; // 发个"hello world"回去
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0);  // 差错处理不做了, send和write的关系l类似于read和recv
    }
    return nullptr;
}
```

![image-20250404172828508](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404172828591.png)

我们打开开发者模式, 就可以看到收到的`html`了, 前端就是写`html`, 我们后端是把`html`发过去, 并负责后端逻辑的, 所以一个前端一般对应3至5个后端

```shell
[whisper@VM-12-6-centos ~]$ telnet 120.55.90.240 8888
Trying 120.55.90.240...
Connected to 120.55.90.240.
Escape character is '^]'.
^]
telnet> 
GET / HTTP/1.1                                                                            
HTTP/1.0 200 OK
Content-Length: 46

<html><body><h3>hello world</h3></body></html>Connection closed by foreign host.
[whisper@VM-12-6-centos ~]$
```

在之前的请求中, 我们给的都是`Web`根目录, 其实我们也可以去其他目录, 比如我们在浏览器地址栏里输`120.55.90.240:8888/a/b/c/d.html`, 我们就可以收到这份报文

```shell
GET / HTTP/1.1
GET /a/b/c/d.html HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Upgrade-Insecure-Requests: 1

GET /favicon.ico HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36
Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Referer: http://120.55.90.240:8888/a/b/c/d.html

```

我们主要看第一份报文, 第二份报文在请求网站图标, 就是这种东西

![image-20250404173957338](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404173957382.png)

所以如果我们想有图标, 那就在`Web`根目录下放个`favicon.ico`

接下来我们看看第一份报文, 第一份报文请求的是`/a/b/c/d.html`这个网页, 不过我们硬编码的, 所以没有反应. 之后我们可能是要返一个"404"的. 

我们可以再给`URL`加点东西

![image-20250404174655481](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404174655530.png)

服务端也可以收到对应的信息

```shell
GET /a/b/c/d.html?username=wind&passwd=123 HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Upgrade-Insecure-Requests: 1

GET /favicon.ico HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36
Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Referer: http://120.55.90.240:8888/a/b/c/d.html?username=wind&passwd=123
```

所以之后我们就可以把`URL`解析出来, 根据它的路径, 找到资源, 发回去. 

接下来我们就要看看`Web`根目录,  这个`Web`根目录其实没什么特别的, 一般情况下, 对于正式的服务, 一般就是根目录, 但实际上, 它可以为任意位置, 关键在于如何解析它, 比如, 如果我们把这个路径拼接到当前进程的工作路径, 那工作路径就是`Web`根路径, 其实就是看前面是怎么拼接的. 

比如, 我们在项目路径下加个`wwwroot`文件夹, 作为我们的`Web`根目录,

![image-20250404180003029](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404180003591.png)

将来我们的所有网络资源, 页面, 音频, 图片, 都会放在这里, 之后你把用户发来的路径拼到`wwwroot`路径下就行了

接下来我们在`wwwroot`里加个首页`html`文件, 用`AI`写的那份`html`, 

```shell
[wind@starry-sky HTTP]$ ls -al
total 168
drwxrwxr-x 4 wind wind   4096 Apr  4 17:59 .
drwxrwxr-x 6 wind wind   4096 Apr  2 19:27 ..
-rwxrwxr-x 1 wind wind 129176 Apr  4 17:27 HttpServer
-rw-rw-r-- 1 wind wind     87 Apr  3 15:44 HttpServer.cc
-rw-rw-r-- 1 wind wind   2278 Apr  4 17:26 HttpServer.hpp
-rw-rw-r-- 1 wind wind   5121 Apr  3 14:52 log.hpp
-rw-rw-r-- 1 wind wind    455 Apr  3 15:47 makefile
-rw-rw-r-- 1 wind wind   3487 Apr  3 14:52 Sockst.hpp
drwxrwxr-x 2 wind wind   4096 Apr  3 15:38 .vscode
drwxrwxr-x 2 wind wind   4096 Apr  4 18:03 wwwroot
[wind@starry-sky HTTP]$ cd wwwroot
[wind@starry-sky wwwroot]$ tree .
.
└── index.html

0 directories, 1 file
[wind@starry-sky wwwroot]$
```

 页面已经写成文件了, 我们写个函数, 把文件读出来, 写到`HTTP`正文上. 

```cpp
std::string readHtml(const char* resource_path)
{
    std::ifstream in(resource_path);
    std::string result, line;
    while(std::getline(in, line))
        result += line;
    in.close();
    return result;
}

static void* threadRun(void* args_)
{
    pthread_detach(pthread_self());  char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs*>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    if(n > 0)
    {
        buffer[n] = 0;
        std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = readHtml("wwwroot/index.html");
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0);  // 差错处理不做了, send和write的关系l类似于read和recv
    }
    return nullptr;
}
```

此时, 再进行访问就能看到相应的效果

![image-20250404194653937](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250404194654086.png)

不过我们这里, 路径目前是写死的, 将来我们会对用户发来的路径解析, 拼接成一个合法的路径. 下面, 我们就写个函数解析一下收到的请求报文, 并把用户给的路径拿出来. 我们要把用户给的路径拼接成`wwwroot/xxx/xxxx/xxx`的形式, 另外, 还需要说一句, 对于`Web`根目录, `/`, 如果用户给的是这个路径, 那就给它展示首页. 

我们这里定义了一个宏`#define WEB_ROOT "wwwroot"`, 标记`Web`根目录,在哪里, 但其实上一般不是定义宏, 而是把一些路径写在配置文件里, 然后安排一个初始化类对象读配置文件, 可能这个初始化类对象里有个`std::string`, 配置文件选择谁是`Web`根目录, 谁就是`Web`根目录, 之后进程需要`Web`根目录路径, 就从初始化类里访问, 是这种形式, 不是用宏, 我这里用宏是图省事, 学习项目, 用不着这样, 配置文件方案的另一个好处是可以无缝修改根目录, 直接改文件就行了.

下面我们写一个`class HttpRequest`, 用来进行请求报文解析

```cpp
#define HTTP_HEADER_DELIMITER "\r\n"
class HttpRequest
{
    public:
    void init(std::string req)
    {
        while(true)
        {
            size_t pos = req.find(HTTP_HEADER_DELIMITER);
            if(pos == std::string::npos) break;
            std::string temp = req.substr(0, pos);
            if(temp.empty()) break;
            _req_header.emplace_back(temp);
            req.erase(0, pos + strlen(HTTP_HEADER_DELIMITER));
        }
        _body = req;
    }

    void print()
    {
        std::cout<<"--------------------"<<std::endl;
        for(const auto& e: _req_header)
        {
            std::cout<<e<<HTTP_HEADER_DELIMITER;
        }
        std::cout<<_body <<std::endl;
    }
    private:
    std::vector<std::string> _req_header;   // 把非正文部分以每行为单位, 存在数组里
    std::string _body;                      // 存正文
};

static void* threadRun(void* args_)
{
    pthread_detach(pthread_self());  char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs*>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    if(n > 0)
    {
        buffer[n] = 0;
        HttpRequest req;
        req.init(buffer); // 默认报文完整
        req.print();
        // std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = readHtml("wwwroot/index.html");
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0);  // 差错处理不做了, send和write的关系l类似于read和recv
    }
    return nullptr;
}
```

```shell
[wind@starry-sky HTTP]$ ./HttpServer
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:40500         0.0.0.0:*               LISTEN      9230/node           
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -                   
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      19442/./HttpServer  
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -                   
tcp        0      0 127.0.0.1:35357         0.0.0.0:*               LISTEN      10262/node          
tcp6       0      0 :::111                  :::*                    LISTEN      -                   
tcp6       0      0 :::22                   :::*                    LISTEN      -                   
tcp6       0      0 ::1:25                  :::*                    LISTEN      -                   
--------------------
GET / HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/135.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1


--------------------
GET /favicon.ico HTTP/1.1
Host: 120.55.90.240:8888
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/135.0.0.0 Safari/537.36
Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Referer: http://120.55.90.240:8888/

```

接下来我们再对第一行, 也就是请求行进行处理, 把里面的组分, 比如用户请求路径, 协议版本什么的.

由于第一行以空格为分隔符, 而C/C++也默认空格为分隔符, 所以我们用`stringstream`来进行分割, 为了便于调试, 我们也更新了`print`

```cpp
class HttpRequest
{
public:
    void init(std::string req)
    {
        while (true)
        {
            size_t pos = req.find(HTTP_HEADER_DELIMITER);
            if (pos == std::string::npos)
                break;
            std::string temp = req.substr(0, pos);
            if (temp.empty())
                break;
            _req_header.emplace_back(temp);
            req.erase(0, pos + strlen(HTTP_HEADER_DELIMITER));
        }
        _body = req;
    }

    void parse()
    {
        std::string temp;
        std::stringstream ss(_req_header[0]);
        ss >> method_ >> url_ >> http_version_;
    }

    //AI以我的原型代码改出来的
    void print()
    {
        // 彩色打印标题
        std::cout << "\033[1;36m" // 亮青色
                  << "╔════════════════ HTTP REQUEST ════════════════╗"
                  << "\033[0m" << std::endl;

        // 基本信息（加粗显示）
        std::cout << "\033[1;33m" // 亮黄色
                  << "│ Method: \033[1;37m" << method_ << "\033[0m" << std::endl;
        std::cout << "\033[1;33m│ Url :   \033[1;37m" << url_ << "\033[0m" << std::endl;
        std::cout << "\033[1;33m│ HTTP:   \033[1;37m" << http_version_ << "\033[0m" << std::endl;

        // 请求头（跳过第一个）
        if (!_req_header.empty())
        {
            std::cout << "\033[1;33m├────────── Headers ───────────\033[0m" << std::endl;
            for (auto it = std::next(_req_header.begin()); it != _req_header.end(); ++it)
            {
                std::cout << "│ \033[34m" << *it << "\033[0m" << HTTP_HEADER_DELIMITER << std::endl;
            }
        }

        // 请求体（如果有）
        if (!_body.empty())
        {
            std::cout << "\033[1;33m├────────── Body ──────────────\033[0m" << std::endl;
            std::cout << "│ " << _body << std::endl;
        }

        // 底部边框
        std::cout << "\033[1;36m"
                  << "╚══════════════════════════════════════════════╝"
                  << "\033[0m" << std::endl;
    }

    // void print()
    // {
    //     std::cout << "--------------------" << std::endl;
    //     auto it = _req_header.begin();
    //     ++it;
    //     std::cout << "method       : " << method_ << std::endl;
    //     std::cout << "url          : " << url_ << std::endl;
    //     std::cout << "http_version : " << http_version_ << std::endl;
    //     while (it != _req_header.end())
    //     {
    //         std::cout << *it << HTTP_HEADER_DELIMITER;
    //         ++it;
    //     }
    //     std::cout << _body << std::endl;
    // }

private:
    std::vector<std::string> _req_header; // 把非正文部分以每行为单位, 存在数组里
    std::string _body;                    // 存正文

    std::string method_;				 // 请求方式
    std::string url_;					 // 略去域名的url
    std::string http_version_;			  // 协议版本
};


static void* threadRun(void* args_)
{
    pthread_detach(pthread_self());  char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs*>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    if(n > 0)
    {
        buffer[n] = 0;
        HttpRequest req;
        req.init(buffer); // 默认报文完整
        req.parse();
        req.print();
        // std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = readHtml("wwwroot/index.html");
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0);  // 差错处理不做了, send和write的关系l类似于read和recv
    }
    return nullptr;
}
```

这效果确实挺好

![image-20250405170917990](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405170918094.png)

另外, 我们看到, 它默认请求`Web`根目录路径, 但一般来说, 对于这种直接请求根目录的行为, 我们一般是把首页发过来, 而不是把根目录下的所有东西发过去. 

下面, 我们在`parse`里将存储在`url`中的路径截取出来, 并进行修正, 原先的`url_`中只保留搜索字符串的信息, 为此我们将其更名为`url_query_`, 另外对打印效果进行了进一步更新

```cpp
class HttpRequest
{
public:
    void init(std::string req)
    {
        while (true)
        {
            size_t pos = req.find(HTTP_HEADER_DELIMITER);
            if (pos == std::string::npos)
                break;
            std::string temp = req.substr(0, pos);
            if (temp.empty())
                break;
            _req_header.emplace_back(temp);
            req.erase(0, pos + strlen(HTTP_HEADER_DELIMITER));
        }
        _body = req;
    }

    void parse()
    {
        std::string temp;
        std::stringstream ss(_req_header[0]);
        ss >> method_ >> url_query_ >> http_version_;
        size_t pos = url_query_.find('?');
        if (pos == std::string::npos)
            temp = url_query_;
        temp = url_query_.substr(0, pos);
        url_query_.erase(0, pos);
        if (temp == "/")
        {
            path_ = HOME_PAGE;
        }
        else
        {
            path_ += WEB_ROOT;
            path_ += temp;
        }
    }

    // AI以我的原型代码改出来的
    void print()
    {
        // 标题（亮青色+下划线）
        std::cout << "\033[1;4;36m" // 亮青色+粗体+下划线
                  << "HTTP Request Details"
                  << "\033[0m" << std::endl
                  << std::endl;

        // 元信息（黄色标签+白色值）
        std::cout << "\033[33mMethod: \033[37m" << method_ << std::endl
                  << "\033[33mPath:   \033[37m" << path_ << std::endl
                  << "\033[33mHTTP:   \033[37m" << http_version_ << std::endl
                  << "\033[33mQuery:  \033[37m" << (url_query_.empty() ? "(empty)" : url_query_) << std::endl;

        // 请求头（品红色标题+绿色内容）
        if (!_req_header.empty())
        {
            std::cout << std::endl
                      << "\033[35m" << "── Headers ──" << "\033[0m" << std::endl;
            for (auto it = std::next(_req_header.begin()); it != _req_header.end(); ++it)
            {
                std::cout << "\033[32m" << *it << HTTP_HEADER_DELIMITER << "\033[0m" << std::endl;
            }
        }

        // 请求体（橙色标题+灰色内容）
        if (!_body.empty())
        {
            std::cout << std::endl
                      << "\033[38;5;208m" << "── Body ──" << "\033[0m" << std::endl
                      << "\033[90m" << _body << "\033[0m" << std::endl;
        }

        // 结尾线
        std::cout << "\033[90m" << "────────────────────" << "\033[0m" << std::endl;
    }

    // void print()
    // {
    //     std::cout << "--------------------" << std::endl;
    //     auto it = _req_header.begin();
    //     ++it;
    //     std::cout << "method       : " << method_ << std::endl;
    //     std::cout << "path         : " << path_ << std::endl;
    //     std::cout << "http_version : " << http_version_ << std::endl;
    //     std::cout << "url_query    : " << url_query_ << std::endl;
    //     while (it != _req_header.end())
    //     {
    //         std::cout << *it << HTTP_HEADER_DELIMITER;
    //         ++it;
    //     }
    //     std::cout << _body << std::endl;
    // }

private:
    std::vector<std::string> _req_header; // 把非正文部分以每行为单位, 存在数组里
    std::string _body;                    // 存正文

    std::string method_;
    std::string url_query_;
    std::string http_version_;

    std::string path_;
};
```

我们看到, 效果还是非常好的

![image-20250405172911668](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405172911757.png)

![image-20250405173147904](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405173148007.png)

在这之后, 我们就可以根据用户的请求动态调整页面了, 而不是像以前那样写死.

为了快速搭建一定的文件层次结构, 可以在`web`根目录下执行以下脚本

```shell
#!/bin/bash

# 定义要创建的目录结构
DIRS=(
    "css"
    "js"
    "images"
    "assets/fonts"
    "assets/icons"
    "pages/about"
    "pages/contact"
    "posts/2023"
)

# 定义要创建的基本HTML文件
FILES=(
    "index.html"
    "about.html"
    "contact.html"
    "404.html"
    "css/style.css"
    "js/main.js"
    "posts/2023/post1.html"
    "posts/2023/post2.html"
)

# 创建目录
echo "创建目录结构..."
for dir in "${DIRS[@]}"; do
    mkdir -p "$dir"
    echo "  + $dir/"
done

# 创建文件并写入基本HTML结构
echo "生成HTML文件..."
for file in "${FILES[@]}"; do
    cat <<EOF > "$file"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>${file%.*}</title>
    <link rel="stylesheet" href="/css/style.css">
</head>
<body>
    <h1>${file%.*} Page</h1>
    <p>This is a generated test file: $file</p>
    <script src="/js/main.js"></script>
</body>
</html>
EOF
    echo "  + $file"
done

# 创建示例图片
echo "生成占位图片..."
convert -size 100x100 xc:gray images/placeholder1.jpg 2>/dev/null ||
    echo "  ! 需要安装ImageMagick生成图片，跳过"
convert -size 200x150 xc:blue images/placeholder2.png 2>/dev/null ||
    echo "  ! 需要安装ImageMagick生成图片，跳过"

echo "完成！生成的文件结构："
tree -L 3
```

```shell
[wind@starry-sky HTTP]$ ls
HttpServer  HttpServer.cc  HttpServer.hpp  log.hpp  makefile  Sockst.hpp  wwwroot
[wind@starry-sky HTTP]$ cd wwwroot
[wind@starry-sky wwwroot]$ ls
cosmic_404.html  generate_web_structure.sh  index.html
[wind@starry-sky wwwroot]$ ./generate_web_structure.sh
-bash: ./generate_web_structure.sh: Permission denied
[wind@starry-sky wwwroot]$ bash ./generate_web_structure.sh
创建目录结构...
  + css/
  + js/
  + images/
  + assets/fonts/
  + assets/icons/
  + pages/about/
  + pages/contact/
  + posts/2023/
生成HTML文件...
  + index.html
  + about.html
  + contact.html
  + 404.html
  + css/style.css
  + js/main.js
  + posts/2023/post1.html
  + posts/2023/post2.html
生成占位图片...
  ! 需要安装ImageMagick生成图片，跳过
  ! 需要安装ImageMagick生成图片，跳过
完成！生成的文件结构：
.
├── 404.html
├── about.html
├── assets
│   ├── fonts
│   └── icons
├── contact.html
├── cosmic_404.html
├── css
│   └── style.css
├── generate_web_structure.sh
├── images
├── index.html
├── js
│   └── main.js
├── pages
│   ├── about
│   └── contact
└── posts
    └── 2023
        ├── post1.html
        └── post2.html

11 directories, 10 files
[wind@starry-sky wwwroot]$
```

下面我们再把之前的`readHtml`改一改, 没有对应文件跳到`cosmic_404.html`.

```cpp
#define COSMIC_404 "wwwroot/cosmic_404.html"
std::string readHtml(const std::string& resource_path)
{
    std::ifstream in(resource_path.c_str());
    if(!in) in.open(COSMIC_404);  // operator bool 重载, 打开失败返回假
    std::string result, line;
    while (std::getline(in, line))
        result += line;
    in.close();
    return result;
}

static void *threadRun(void *args_)
{
    pthread_detach(pthread_self());
    char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs *>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
        buffer[n] = 0;
        HttpRequest req;
        req.init(buffer); // 默认报文完整
        req.parse();
        // req.print();
        // std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = readHtml(req.getPath());
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0); // 差错处理不做了, send和write的关系l类似于read和recv
    }
    return nullptr;
}
```

下面可以根据这些测试用例进行测试, 可能有些测试无法通过, 不用担心, 这是正常现象, 因为上面`readHtml`写法其实是有坑的, 这个坑我们之后再说.

| 输入的 URL 路径          | 预期现象                                                     |
| ------------------------ | ------------------------------------------------------------ |
| `/` 或 `/index.html`     | 正常显示 `index.html` 内容                                   |
| `/about.html`            | 正常显示 `about.html` 内容                                   |
| `/contact.html`          | 正常显示 `contact.html` 内容                                 |
| `/posts/2023/post1.html` | 正常显示 `posts/2023/post1.html` 内容                        |
| `/assets/fonts/`         | ▶ 跳转到 `cosmic_404.html`（目录下无默认文件如 `index.html`） |
| `/nonexistent.html`      | ▶ 跳转到 `cosmic_404.html`（文件不存在）                     |
| `/pages/about/`          | ▶ 跳转到 `cosmic_404.html`（目录下无 `index.html`）          |
| `/images/phantom.jpg`    | ▶ 跳转到 `cosmic_404.html`（文件不存在）                     |
| `/css/style.css`         | 正常返回 CSS 文件内容（若文件存在）                          |
| `/js/main.js`            | 正常返回 JS 文件内容（若文件存在）                           |
| `/404.html`              | 直接显示 `404.html` 内容（不触发自动跳转）                   |
| `/cosmic_404.html`       | 直接显示太空主题 404 页面（手动访问不触发跳转）              |

下面我们说一下`html`中的一些知识, 一般来说, 页面里或多或少都有几个跳转按钮, 点了之后, 就能跳转到相应的页面, 比如我们的`cosmic_404.html`就有一个跳转, 可以返回我们的首页

![image-20250405184145074](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405184145188.png)

页面跳转的`html`标签是这样的`<a href="/" class="home-btn">返回安全地带</a>`

`href`后面的就是要调跳转到的页面地址, 如果你什么都没有, 只有路径, 那么, 它就会默认继续使用当前的`IP`端口, 如果你写了`https://www.baidu.com/`, 那就会跳到百度首页.

![image-20250405184915795](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405184915961.png)

### HTTP的方法

HTTP有如下方法:

![image-20250405203314795](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405203314978.png)

不过我们一般只能看到`GET`和`POST`, `GET`就是获得服务器上的资源, `POST`也有类似于`GET`的功能, 不过它更多的是把本地资源传到服务器上, `PUT`正如这张图所说, 传输文件, `HEAD`的意思就是我不要正文, 你服务器只要发响应行和响应报头就行了, `DELETE`是删除文件, 但由于存在明显的安全问题, 所以一般来说, 都会被关掉, `OPTIONS`询问服务器支持那些方法, 会以正文的形式返回到客户端, `CONNECT`我们在`fiddler`那里见过, 是连接代理的, 其它的就不说了.

我们这里只稍微说说`GET`和`	POST`. 我们上面说过, `POST`是向服务器提交数据, 那我们平时都是怎么提交数据的呢? 答案是表单, 什么是表单, 表单是前端的概念, 像什么搜索框, 

![image-20250405204337434](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405204337592.png)

登录框(就这样叫吧)

![image-20250405204443586](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405204443767.png)

还有其他类似的东西

表单, 在`html`里大概长这样

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>简单登录表单</title>
</head>
<body>
    <form action="/login" method="POST">
        <label for="username">用户名:</label>
        <input type="text" id="username" name="username" /><br/>
        
        <label for="password">密码:</label>
        <input type="password" id="password" name="password" /><br/>
        
        <input type="submit" value="登录" />
    </form>
</body>
</html>
```

被`<form`和`</form>`包围的那部分就是表单, `action="/login"    method="POST"`的意思是往服务器`/login`路径下发请求, 这个请求的方法是`POST`,  如果里不写`method`, 浏览器默认使用`GET`,  `label`是文本提示信息, 提升用户体验的, `input`是读用户输入, 它末尾的`name`的值, 比如`username`, 就相当于`key`, 用户输入的信息就是`val`, 所以等会我们做实验会看到, 如果第一个框你填的是"张三", 那就会有"username=张三"这种pair,  那个`type, id, name`什么东西的, 可以理解属性描述符,解释属性用的,  最后一个`input`没有`name`, 所以生成不了`k-v`只是一个事件触发按钮, 点下它, 浏览器就会向服务器发报文.

`/login`一般是某个程序, 服务端可能会以`fork`和进程替换的方式把用户的输入内容输到这个程序里, 程序给服务段处理结果, 服务端再把结果交给用户什么的.

![image-20250405214104298](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250405214104426.png)

我这里说表单, 其真正目的是为了展示`GET`和`POST`的不同, 现在我们是`POST`方法, 提交一下, 看看服务器打印的报文

![image-20250406154203310](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250406154203428.png)

我们看到. 它这个`k-v`值是放在正文里面的, 如果我把表单的方法改成`"GET"`又会怎样呢?

![image-20250406154600019](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250406154600116.png)

此时我们就可以看到, 已经没有`Content-Length`字段了, 也就是没有正文了, `k-v`是以搜索字符串的形式被拼到`url`上的.

和`POST`相比`GET`隐私性更差, 比如密码登录的时候, 如果用`GET`, 密码就会回显到`url`上, 所以就不太好, 就信息安全角度来说, `ROST`和`	GET`都不安全, 因为报文都是明码发的, 只要有心, 都能直接获取, 而且可以直接查看.  如果你想保证安全性, 那要用`https`, 之后我们会说.

### HTTP的状态码

状态码, 位于相应行的第二列, 用来表示响应的状态

![image-20250407164854306](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250407164854416.png)

`1`开头的比较少见, 它一般用在这种场景下: 用户的请求需要处理一段时间, 为了让页面有反应, 先发一个等待页面, 向用户表示, 请求已经接收到了, 现在正在处理.   以`2`为开头的表示请求正常处理, 在上面的代码中, 我们就把状态码写死成了`200`,即使是`404`界面, 用的仍然是`200`, 这并不规范, 所以等会儿我们会改, `4`开头的, 表示客户端发出了逻辑上不应该发出的请求, 比如, 请求服务端没有的资源, 也就是`404`,还有一个是`403`, 表示用户没有足够的权限获得资源 `5`开头的, 表示服务端本身出问题了, 可能是线程, 进程创建失败了, 无法给用户开一个新会话, 从而无法处理请求, 或者 什么数据库坏了, 查不了数据之类的, `3`开头的等会儿我们再说, 状态码我们不需要记, 只要了解就行了.

下面我们改一下之前的代码, 不把状态码写死, 

```cpp
std::string readHtml(const std::string& resource_path)
{
    std::ifstream in(resource_path.c_str());
    if(!in.is_open()) return std::string();
    std::string result, line;
    while(std::getline(in, line))
        result += line;
    in.close();
    return result;

    // std::ifstream in(resource_path.c_str());
    // if(!in) in.open(COSMIC_404);  // operator bool 重载, 打开失败返回假
    // std::string result, line;
    // while (std::getline(in, line))
    //     result += line;
    // in.close();
    // return result;
}

static void *threadRun(void *args_)
{
    pthread_detach(pthread_self());
    char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs *>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    // std::cout << n << std::endl;
    if (n > 0)
    {
        buffer[n] = 0;
        HttpRequest req;
        req.init(buffer); // 默认报文完整
        req.parse();
        req.print();
        // std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = readHtml(req.getPath());
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        if(body.empty())
        {
            body = readHtml(COSMIC_404);
            response_line = "HTTP/1.0 404 Not Found\r\n";
        }
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0); // 差错处理不做了, send和write的关系l类似于read和recv
        std::cout << response<<std::endl;
    }
    return nullptr;
}
```

测试一下, 就会发现服务端确实发回了一份`404`状态码的报文

![image-20250407172459275](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250407172459401.png)

`404`并不是说页面不响应, 还是要有反应的, 告知用户请求的资源是不存在的.

有的服务端并不严格遵循状态码, 可能什么页面都用`200`, 就像我们之前写的那样, 但我们不推荐这样做, 我们写服务的话, 当然都要考虑一下.

### HTTP常见Header

上面我们并没有说以`3`为开头的状态码, 这是因为`3`开头的, 往往要搭配请求报头中的`Location`使用, `3`开头的系列状态码是一个重点, 主要用于页面重定向, 分为两种, 一是临时重定向, 二是永久重定向, 

重定向的使用场景主要是这样的: 浏览器, 或者 客户端, 向服务端发出了一个请求, 服务端发现, 自己可能因为某些原因无法处理这些请求, 但它知道另一台服务器上的服务可以处理这个请求, 所以就会给浏览器发一份`3`开头系列状态码的响应报文, 这个报文的报头里有一个字段, 大概这样`Location: www.XXX.com`, 浏览器收到之后, 就会对`www.XXX.com`这个服务器重新发送请求, 让这台服务器来处理. 

下面我们就用代码实验一下.

```cpp
static void *threadRun(void *args_)
{
    pthread_detach(pthread_self());
    char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs *>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    // std::cout << n << std::endl;
    if (n > 0)
    {
        buffer[n] = 0;
        HttpRequest req;
        req.init(buffer); // 默认报文完整
        req.parse();
        req.print();
        // std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = readHtml(req.getPath());
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        if(body.empty())
        {
            body = readHtml(COSMIC_404);
            response_line = "HTTP/1.0 404 Not Found\r\n";
        }
        response_line = "HTTP/1.0 302 Found\r\n";               // 硬编码
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "Location: https://www.qq.com";
        response_header += "\r\n"; // 把"Location"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0); // 差错处理不做了, send和write的关系l类似于read和recv
        std::cout << response<<std::endl;
    }
    return nullptr;
```

现在我们只要一访问, 就会被重定向到腾讯官网.

另外我们也可以看到, 有这个字段的话, 浏览器就会忽略初始服务器的正文内容

![image-20250407180228833](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250407180228987.png)

重定向又可以被细分为"临时重定向"和"永久重定向两种", 拿生活中的例子, 你喜欢去一家饭馆, 有一天, 你去饭馆, 发现门上贴条告示, "本店正在升级装修, 已临时移到某某地方",这就是临时重定向, 于是你就去那个"某某地方了", 饭馆在新位置吸引到了一些新客户, 有一天, 原位置装修好了, 要搬回去, 但怕那些新客户找不到店面, 于是在门上说, "本店原址已经装修完毕, 现在迁回去了, 原址在哪里哪里", 于是那些新用户就知道, 以后就再也不用来这个地方了, 这就是"永久重定向"

有时候服务器已经很老了, 它上面的服务实际上已经迁到了另一个服务器上, 域名也重新换了一个, 但很多人只知道旧域名, 所以就可以在这个旧服务器上只部署一个服务, 告诉浏览器, 给用户发个通知页面, 什么该网站已经永久迁移, 几秒钟之后自动跳到新的页面之类.

接下来我们说`"Content-Length"`这个字段, 这个我们用的已经很久了, 客户端和服务端都会用, 用来描述正文的字节数. `Host` 字段, 用来定位服务端的IP和端口, `User-Agent`字段我们已经说过, 代表客户的操作系统和浏览器信息, `Referer`字段是用来支持页面前翻后翻的, 客户端本来在A页面, 向服务端请求B页面, 服务端在响应的时候就会添加`Referer`字段, 正文给B页面, 这样客户端来到B页面之后就能快速回到之前的A页面, 

有时候我们还会看到`Connection: keep-alive`, 在说这个字段前, 我们先来说一下有关HTTP的背景知识, 在早期的HTTP, 使用的是所谓"短连接"的方式, 这个短连接就类似于我们之前写自定义协议的那种方式, 就是一个会话在提供完用户请求的资源后立刻关闭, 需要说明的是, 有些页面上可能还有多份资源, 比如若干张照片, 动图, 甚至视频, 此时如果采用短连接, 由于短连接每次只发送一份资源, 所以可能的情况是, 服务端先把页面HTML文本发过去, 然后浏览器发现HTML文本里有些路径, 标识着服务端中的其它资源, 于是浏览器会再次与服务端连接, 然后拿到一份资源, 然后连接再次关闭, 浏览器再次申请.... 直到把所有资源都申请完, 在互联网早期, 这种方式还是不错的, 但随着网络的发展, 经常会出现许多很大的页面, 里面有很多资源, (比如, 京东这种购物平台, 哔哩哔哩这种流媒体), 如果还采用短连接, 因为资源数量太多了, 所以就很拖效率, 因此就有了长连接方案, 在长连接方案中, 在一个会话里, 客户端和服务端会进行多次通信, 服务端会把页面上的资源再一个会话里, 一个个发给客户端, 而不必再建立新的会话, 资源发完再关闭会话.

至于这个`Connection: keep-alive`, 其实就是客户端和服务端正在进行协商, 因为客户端和服务端双方的版本很可能不一致, 因此对于协议的支持程度也是不同的, 只有双方都支持长连接, 双方才能使用长连接, 如果请求报文里有这个字段, 就意味着, 客户端支持长连接, 并请求进行长连接, 如果响应报文里也有这个字段, 就意味着, 服务端也支持长连接, 允许进行长连接通信, 于是客户端和服务端就可以用长连接方式通信了.

有时候我们还会看到`Upgrade`开头的字段, 比如`Upgrade-Insecure-Requests: 1`, `Upgrade`的字段是由于协议升级请求的, 具体有很多种升级方向, 比如`Upgrade-Insecure-Requests: 1`的意思是说, 现在我们是用HTTP协议进行通信的, 但HTTP用的是明文, 不太好, 我们能不能升级到HTTPS协议, 如果服务端支持HTTPS, 那就可以再发回去, 表明, 服务端可以将协议升级为HTTPS, 除此之外, 我们之前接触到的网络通信都是CS模式, 所谓CS模式就是服务端等待客户端的请求, 这种模式已经可以满足绝大多数的网络通信需求了, 但在一些情况下, 需要服务端与客户端平等交流, 甚至服务端找客户端, 此时就需要升级协议.

下面, 我们通过代码, 来看一下短连接.

其实把之前那个重定向代码注释一下, 然后改一下首页就行了

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>index</title>
    <link rel="stylesheet" href="/css/style.css">
</head>
<body>
    <h1>index Page</h1>
    <p>This is a generated test file: index.html</p>
    <script src="/js/main.js"></script>
    <img src="images/1.jpg" alt="表情包一" width="500" height="500">
    <img src="images/2.jpg" alt="表情包二" width="300" height="300">
</body>
</html>
```

```shell
# 在images文件夹下准备了两副照片
[wind@starry-sky HTTP]$ ls wwwroot/images
1.jpg  2.jpg
[wind@starry-sky HTTP]$
```

我们用浏览器重新登录一下, 会发现, 没有照片, 但根据服务端的打印来看, 照片应该发出去了

![image-20250407214502872](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250407214503208.png)

只不过因为照片是二进制文件, 所以都是乱码.

不过我既然发出去了, 为什么浏览器不渲染呢? 有两个原因, 我们先说第一种, 这种方法比较好察觉, 那就是我们读文件有问题, 

```cpp
std::string readHtml(const std::string& resource_path)
{
    std::ifstream in(resource_path.c_str());
    if(!in.is_open()) return std::string();
    std::string result, line;
    while(std::getline(in, line))
        result += line;
    in.close();
    return result;
}
```

我们这里用的是文本形式读, 但图片是二进制文件, 所以我们应该用二进制的方式去读, 二进制读取的另一个好处是在编码方式正确的前提下, 也可以支持文本文件的读取

```cpp
std::string readHtml(const std::string& resource_path)
{
    std::ifstream in(resource_path.c_str(), std::ios::binary);
    if(!in.is_open()) return std::string();
    in.seekg(0, std::ios::end);                                     // 将文件指针移到文件末尾
    size_t size = in.tellg();                                       // 获取当前文件指针距离文件开头的偏移量
    in.seekg(0, std::ios::beg);                                     // 将文件指针重新移到开头

    std::string result(size, ' ');
    in.read(const_cast<char*>(result.c_str()), size);               // 把文件读到缓冲区
    in.close();
    return result;
}
```

改了之后我们发现加载出来了

![image-20250408130800341](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408130800494.png)

但其实上, 这是侥幸, 文件有很多种, 有文本, 有图片, 有视频, 音频, 浏览器怎么知道正文里的究竟是什么文件, 而且, 即使单对照片来说, 还有`.jpg, .png....`等形式, 浏览器它不一定能仅靠正文内容分析出来文件的具体种类和具体格式, 为此, 我们需要在响应报头里再加一个字段, 那就是`Content-Type`字段, 你问这里为什么,能加载出来? 我只能说, 可能是Chrome很强大, 它单靠正文自己分析出了正确的格式,不过我们该加还是要加的, 以下是`Content-Type`对照表

| 文件种类      | 文件扩展名  | Content-Type（MIME 类型）              |
| ------------- | ----------- | -------------------------------------- |
| HTML 文件     | .html, .htm | text/html                              |
| CSS 样式表    | .css        | text/css                               |
| JavaScript    | .js         | application/javascript                 |
| JSON 数据     | .json       | application/json                       |
| XML 数据      | .xml        | application/xml 或 text/xml            |
| JPEG 图片     | .jpg, .jpeg | image/jpeg                             |
| PNG 图片      | .png        | image/png                              |
| GIF 图片      | .gif        | image/gif                              |
| SVG 向量图    | .svg        | image/svg+xml                          |
| ICO 图标      | .ico        | image/x-icon                           |
| PDF 文档      | .pdf        | application/pdf                        |
| ZIP 压缩包    | .zip        | application/zip                        |
| GZIP 文件     | .gz         | application/gzip                       |
| MP3 音频      | .mp3        | audio/mpeg                             |
| MP4 视频      | .mp4        | video/mp4                              |
| WebM 视频     | .webm       | video/webm                             |
| OGG 音频/视频 | .ogg        | application/ogg 或 audio/ogg/video/ogg |
| WOFF 字体     | .woff       | font/woff                              |
| WOFF2 字体    | .woff2      | font/woff2                             |
| TTF 字体      | .ttf        | font/ttf                               |
| OTF 字体      | .otf        | font/otf                               |
| CSV 表格      | .csv        | text/csv                               |
| 纯文本        | .txt        | text/plain                             |

对于`Content-Type`字段的分析, 一般通过两个渠道解析出来, 一是通过文件的后缀名, 下面我们用的就是这个方法, 但相信你也知道, 后缀名这个东西, 可能是有错误的, Linux本身对于文件类型的判断都不借助于后缀名, 所以另一个渠道是通过某些第三方库, 来通过对文件内容的分析直接得出MIME 类型, 比如`libmagic `库, 我们这里力求简洁易懂, 所以用的是第一个渠道.

如果采用第一种方法, 可能需要一张像上面表格那样描述类型对应关系的配置文件, 然后文件初始化对象会读这个文件, 并进行一定的处理, 然后别的组件直接向内存文件对象直接询问, 这个`readHtml`最好改成类, 从职责划分来看, `readHtml`适合干这件事: 确定请求资源的MIME 类型, 但我们这里还是简单一点, 直接写到`HttpRequest`里面

```cpp
void parse()
{
    // std::cout << _body<<std::endl;
    std::string temp;
    std::stringstream ss(_req_header[0]);
    ss >> method_ >> url_query_ >> http_version_;
    size_t pos = url_query_.find('?');
    if (pos == std::string::npos)
        temp = url_query_;
    temp = url_query_.substr(0, pos);
    url_query_.erase(0, pos);
    if (temp == "/")
    {
        path_ = HOME_PAGE;
    }
    else
    {
        path_ += WEB_ROOT;
        path_ += temp;
    }
    mime_ = "text/html";
    pos = path_.rfind('.');
    if(pos == std::string::npos) return;
    temp = path_.substr(pos);
    if(temp == ".jpg" || temp == ".jpeg") mime_ = "image/jpeg";
    // std::cout << _body<<std::endl;
}


static void *threadRun(void *args_)
{
    pthread_detach(pthread_self());
    char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs *>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    // std::cout << n << std::endl;
    if (n > 0)
    {
        buffer[n] = 0;
        HttpRequest req;
        req.init(buffer); // 默认报文完整
        req.parse();
        req.print();
        // std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = readHtml(req.getPath());
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        if(body.empty())
        {
            body = readHtml(COSMIC_404);
            response_line = "HTTP/1.0 404 Not Found\r\n";
        }
        // response_line = "HTTP/1.0 302 Found\r\n";               // 硬编码
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "Content-Type: ";
        response_header += req.getMime();
        response_header += "\r\n"; // 把"Location"这行末尾加换行
        // response_header += "Location: https://www.qq.com";
        // response_header += "\r\n"; // 把"Location"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        std::cout << response<<std::endl;    // 不看正文了
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0); // 差错处理不做了, send和write的关系l类似于read和recv
    }
    return nullptr;
}
```

![image-20250408135156130](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408135156238.png)

![image-20250408135233157](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408135233241.png)

![image-20250408135332506](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408135332633.png)

另外说一下, 在`readHtml`那里, 可能会有人这样这样写

```cpp
std::string readHtml(const std::string& resource_path)
{
    std::ifstream in(resource_path.c_str(), std::ios::binary);
    if(!in.is_open()) return std::string();
    std::string result((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
    in.close();
    return result;
}
```

首先要说的是它用的是`std::string`的迭代器构造, `std::istreambuf_iterator`是个模版类, 可以把C++输入流的底层缓冲区包装成迭代器的样子, 所以就可以不用计算文件大小了, 需要说明的是`std::istreambuf_iterator`是一次性的, `(std::istreambuf_iterator<char>(in))`的意思就是把`in`这个输入流底层缓冲区的起始位置包装成迭代器, `(std::istreambuf_iterator<char>())`就是把当前使用的缓冲区, 这里就是`in`的末尾位置包装成迭代器, 所以它是一次性的, 不能混着用, 你必须要先做完这个输入流, 再用它包装另一个输入流, 否则它会对不上的.

好的, 在上面的过程中, 我们看到, 首页有多个资源, 所以尽管我们在浏览器上只访问了首页, 但客户端和服务端进行了多次会话, 每次会话只传一份资源, 这就是短连接.	

下面我们谈另一个字段, 那就是`Cookie`, 这个`Cookie`是用来保持进行会话保持的, 这里要说到一个背景知识, 就是HTTP它默认是无状态的, 什么意思呢? 无状态的意思是, HTTP 协议在每次请求和响应之间不保留任何关于客户端状态的信息。每次请求都是独立的，服务器不会自动记住之前的请求或客户端的状态。这些状态有很多种, 比如登录状态,   我们拿B站举例, B站上有很多视频, 每个视频可能都对应一个页面, 很明显, 看视频你需要先登录, 于是服务端把你重定向到了登录页面, 你登录成功, 看到了视频, 接着你又想看你一个视频, 由于每次请求是独立的, 所以这次请求会丧失之前请求的登录信息, 所以这次请求会认为你没有登录, 又把你重定向到登录界面, 要求你进行登录, 但很明显, 实际上, 并不是这样,  你发现把B站页面关掉, 再重新打开, 它还认识你是谁, 不会让你重新登陆 .          这就是借助了`Cookie`这个字段实现的.

 具体怎么实现的呢? 其实也比较简单, 首先你进入B站首页, 点了登录按钮, 于是被重定向到了登录界面, 你把账号密码输进去, 服务端就会依据你的账号密码再数据库里面找到并验证你, 然后就给你发一个响应报文, 这个响应报文的报头里会有一个`Set-Cookie`选项, 该选项描述了客户端的状态, 比如它可能就是直接的账号密码`username=wind&&passwd=1234`, 也有可能不直接包含状态, 但通过某些标识符映射着某些状态, 然后浏览器收到这份报文后, 就会把`Set-Cookie`存起来, 等到你访问其它页面时, 浏览器就会把`Coolie`放在请求报头里面, 服务端一解析, 就会知道, 你是谁, 就不用再重新进行登录了. 

浏览器对`Cookie`的保存分为两种, 一是内存级, 就是不往磁盘上写, 所以你把浏览器关闭, `Cookie`信息就丢失了, 二是磁盘机文件, 因为已经写到磁盘上了, 所以你即使关机重启, 上面的内容还保存着.

这里我们打开`Edge`浏览器, 不用`Chrome`的原因是因为`Chrome`会把`Cookie`内容封装起来, 所以我们看不到实际内容, `Edge`是可以直接看到的.

![image-20250408154729131](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408154729804.png)

![image-20250408154809982](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408154810688.png)

![image-20250408154840620](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408154841220.png)

![image-20250408154912740](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408154913422.png)

![image-20250408154934249](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408154934879.png)

接下来我们硬编码, 给响应报文都加个`Set-Cookie`

```cpp
static void *threadRun(void *args_)
    {
        pthread_detach(pthread_self());
        char buffer[BUFFER_SIZE];
        std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs *>(args_));
        ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
        // std::cout << n << std::endl;
        if (n > 0)
        {
            buffer[n] = 0;
            HttpRequest req;
            req.init(buffer); // 默认报文完整
            req.parse();
            req.print();
            // std::cout <<buffer;

            // 返回响应的过程     初学阶段, 硬编码
            std::string body = readHtml(req.getPath());
            std::string response_line = "HTTP/1.0 200 OK\r\n";
            if(body.empty())
            {
                body = readHtml(COSMIC_404);
                response_line = "HTTP/1.0 404 Not Found\r\n";
            }
            // response_line = "HTTP/1.0 302 Found\r\n";               // 硬编码
            std::string response_header = "Content-Length: ";
            response_header += std::to_string(body.size());
            response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
            response_header += "Content-Type: ";
            response_header += req.getMime();
            response_header += "\r\n"; // 把"Content-Type"这行末尾加换行
            response_header += "Set-Cookie: name=2131&&passwd=12345";
            response_header += "\r\n"; // 把"Set-Cookie"这行末尾加换行
            // response_header += "Location: https://www.qq.com";
            // response_header += "\r\n"; // 把"Location"这行末尾加换行
            response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
            std::string response = response_line;
            response += response_header;
            std::cout << response<<std::endl;    // 不看正文了
            response += body;
            send(args->_sockfd, response.c_str(), response.size(), 0); // 差错处理不做了, send和write的关系l类似于read和recv
        }
        return nullptr;
    }
```

我们再次访问, 就能看到我们的Cookie了

![image-20250408160144997](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408160145283.png)

另外, 在后来访问的时候, 浏览器就自动把`Cookie`带上了

![image-20250408161741599](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250408161741811.png)

我们上面的`Set-Cookie`字段写的稍有问题, 在响应报文中, 每个`Set-Cookie`都只是一个`k-v`结构, 所以如果要传送多个`k-v`, 需要用到多个`Set-Cookie`, 而不是多个`k-v`用`&&`连起来, 挤到一个`Set-Cookie`里, 下面我们改一下

```cpp
static void *threadRun(void *args_)
{
    pthread_detach(pthread_self());
    char buffer[BUFFER_SIZE];
    std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs *>(args_));
    ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
    // std::cout << n << std::endl;
    if (n > 0)
    {
        buffer[n] = 0;
        HttpRequest req;
        req.init(buffer); // 默认报文完整
        req.parse();
        req.print();
        // std::cout <<buffer;

        // 返回响应的过程     初学阶段, 硬编码
        std::string body = readHtml(req.getPath());
        std::string response_line = "HTTP/1.0 200 OK\r\n";
        if(body.empty())
        {
            body = readHtml(COSMIC_404);
            response_line = "HTTP/1.0 404 Not Found\r\n";
        }
        // response_line = "HTTP/1.0 302 Found\r\n";               // 硬编码
        std::string response_header = "Content-Length: ";
        response_header += std::to_string(body.size());
        response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
        response_header += "Content-Type: ";
        response_header += req.getMime();
        response_header += "\r\n"; // 把"Content-Type"这行末尾加换行
        response_header += "Set-Cookie: name=2131";
        response_header += "\r\n"; // 把"Set-Cookie"这行末尾加换行
        response_header += "Set-Cookie: passwd=12345";
        response_header += "\r\n"; // 把"Set-Cookie"这行末尾加换行
        // response_header += "Location: https://www.qq.com";
        // response_header += "\r\n"; // 把"Location"这行末尾加换行
        response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
        std::string response = response_line;
        response += response_header;
        std::cout << response<<std::endl;    // 不看正文了
        response += body;
        send(args->_sockfd, response.c_str(), response.size(), 0); // 差错处理不做了, send和write的关系l类似于read和recv
    }
    return nullptr;
```

![image-20250409153807296](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250409153807415.png)

在上面我们写的`Cookie`里面就是用户的账号密码, 相当于通过账号密码直接对用户进行身份验证, 但实际上, 现在已经没有人会这样做了, 因为很明显, 这种`Cookie`很容易造成信息安全问题. 

在细说原因之前, 我们需要知道的一个前置信息就是, 在今天, 对于专业人士来说, 普通用户的安全防护等级几乎为零, 因此, 对于用户来说, 几乎非常容易因为各种各样的原因, 而沾染上恶意程序, 于是那些专业人士就可以很方便地获取到`Cookie`文件中的账号密码, 从而获得对账号的控制权, 不仅仅是使用权, 控制权是可以直接修改密码, 从而让用户失去对账号的控制, 并且, 某些客户端具有很强的社交属性, 它们里面其实也有`Cookie`, 对于这种软件来说, 即使只有使用权, 也可以利用社交账号的身份属性对用户进行冒充, 进而造成很严重的后果. 

 所以`Cookie`里面不能直接存放账号密码, 而是间接存储你的身份属性.

当用户第一次登录服务端时, 客户端会把用户的账号密码交给服务端, 服务端对其进行身份验证, 当验证通过时, 便会为用户创建一个`Session`(会话)文件, `Session`文件中存放着用户的敏感信息数据, 比如账号密码, 随后, 服务端会通过一些方式, 将`Session`文件映射到一个特定长度的字符串中, 该字符串就被称为`Session ID`, 而服务端向客户端返回的`Set-Cookie`, 里面将不会有账号密码这类敏感数据, 而是只有`Session ID`这种间接映射账号密码的全服务端唯一标识符, 等到用户下次再次登录该服务端, 客户端或者浏览器就会把`Session ID`发给服务端, 服务端就能借助于这个`Session ID`找到`Session`文件, 从而知道你是谁, 这样, 尽管因为HTTP的无状态性质, 每次会话都是独立的, 但这些独立的会话就能通过`Session`文件联系起来, 从而达到会话保持的效果.

当然, 服务端会有很多`Session`文件, 所以它会建立专门的子模块去维护这些`Session`文件,  常见的就如`Redis `集群 , 这种方案就相当于把敏感信息交由更安全的服务端进行管理, 尽管黑客仍可以通过盗取`Session ID`去冒充用户, 但只能有使用权, 没有控制权, 而且服务端还可以通过分析诸如用户的`IP`, 用户的行为, 从而判断这个用户是不是冒充的, 服务端觉得你是冒充的, 就会让你重新输入密码进行验证, 但`Cookie`没有密码, 所以就增加了冒充成本, 甚至把账号冻结.一个常见的例子是, 比如放假, 你回到老家, 你登录时, 服务端就会说, "登录一个不常用的IP, 请重新登录", 至于用户行为,   就是看用户的行为是不是非常反常, 就像换了一个人一样, 原来可能QQ很少说话, 但突然, 天天聊天, 类似于这种.

现在还有一个问题, HTTP是明文传送的, 即使你`Cookie`里面没有账号密码, 但登录的时候肯定是要输账号密码的, 那在客户端向服务端传账号密码的时候, 因为是明文, 这信息不就泄露了吗? 为此, 下面我们就进入新的章节, HTTPS.

## HTTPS

### 初识HTTPS

首先我们需要说的是, 在应用层, HTTP和HTTPS是共存的.  在以往, HTTP把报文构造好之后, 就直接把报文通过系统接口交给操作系统, 操作系统再把报文发到网络中, 由于系统并没有进行数据加密的职责, 所以它就会明文发送到网络中, 所以我们的报文就相当于在网络中裸奔, 这种数据安全问题催生出人们对于数据加密的需求, 于是, 就有组织在应用层中又加了一个新的软件层, 叫做加密解密层, 典型的就如`ssl`协议, 此时HTTP在构造完报文后就可以选择将报文先交给加密解密层, 然后再由加密解密层经过对报文的加密解密之后再交给操作系统, 这样报文就不是明文了, 光一个HTTP那就是HTTP, 如果HTTP再加上`ssl`, 那就成了HTTPS, 不过我们从计算机哲学角度来看, 即使HTTPS它有加密, 但也不是高枕无忧的, 它也有安全问题, 但比HTTP少了很多, 没有协议是完全安全的, 判断协议是否安全靠的不是协议本身, 而是协议背后的人, 绝对安全是无法从技术角度上探讨的, 而是从社会工程学上探讨, 如果一个协议攻破的成本大于攻破的收益, 那它才是比较安全的, 其中涉及到`ssl`这种加密解密层的社区维护人员和进行网络攻击相关人士的较量.

在继续更近一步前, 我们先要了解一下与加密解密相关的概念, "加密", "解密"这两个词其实可以望文生义, 加密就是把明文变成密文的过程, 解密就是把密文变回明文的过程, 在加密解密过程中, 往往需要一系列辅助工具, 对于计算机科学来说, 那其实就是某种数据, 毕竟计算机就是数据处理工具, 这种在加密解密过程中起辅助作用的数据, 我们就称之为"秘钥".

比如, 我们可以使用按位异或模拟一次简单的加密解密过程, 

![image-20250409175056435](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250409175056520.png)

当然我们实际用的肯定不是这么简单的, 实际上, 对于加密解密这件事,  计算机还没出来的时候人们就有需求了, 如今, 只不过是需求延伸到计算机里面, 对于加密解密, 早就有了对应的学派, 密码学, 这个学派里面都是数学神仙, 我们下面不会谈数学原理, 而只是从工程学角度略微说说.

首先从需求角度来看, (这个好像是经济学角度)

在互联网早期, 大家都用HTTP的那个年代, 有时候会出现一种被称为"运营商劫持"的怪事, 那就是你本来想要下载什么软件, 但最后你拿到的链接是用来下载另一个软件的, 然后稀里糊涂把它下下来, 发现不是自己想要的.

出现这种情况的原因是, 你和下载服务器之间的通信用的是明文, 是明文的话, 里面的内容就很容易被被人获知甚至篡改, 由于运营商管的是物理层的事, 所以很明显, 不管什么报文, 你最后都要到物理层, 所以如果报文是明文的, 运营商就能看得一清二楚, 当服务端响应客户端的请求, 为客户端发出一份含有软件下载链接的报文时, 运营商就会看到, 然后他可能就会因为有一些公司给他利益之类的原因, 就把报文拦下来, 把里面的链接换成某些公司的软件, 从而给某些公司的平台增大曝光度流量之类的,     我们把这种基于网络传输中间过程的攻击称为"中间人攻击", 缩写为`MITM`, 在这里, 运营商就扮演着"中间人"的这个角色, 运营商其实还好, 要是黑客做中间人, 那就非常不好了. 中间人不是说一定干坏事, 但最好不要让他们看到.

总而言之, 网络通信不能用明文.所以我们要加密.

### 常见的加密方式

 我们首先说"对称加密". 对称加密就是说, 加密和解密用同一份密钥, 我们之前的那个按位异或, 其实就是一种对称加密.它的特点就是加密速度快, 效率也高. 另外的一种就是"非对称加密", 在"非对称加密"里, 会涉及到两个密钥, 我们记为`A`和`	B`, 用`A`加密出的密文只能由`B`解密出明文, 而对于用`	B`加密出的密文也只能由`A`来进行解密, 一般来说, 会把其中的一个密钥公开出去, 称之为"公钥", 另外的, 不公开的, 就被称之为"私钥". 它的特点就是加密解密速度很慢, 效率也不行.因为涉及到很多算法. 在分了"公钥"和"私钥"之后, "私钥"加的密只有"公钥"能解, "公钥"加的密只能由"私钥"来解.

### 数据摘要和数据指纹

数据摘要或者说数据指纹, 其基本原理是利用哈希函数, 对信息进行运算, 从而生成一串固定长度的字符串, 数据摘要的本身不是加密, 它的主要应用是用来判断数据有没有遭到篡改.   数据摘要有极小可能会发生冲突, 但概率确实很小, 所以可以认为在全局具有唯一性.  比如, 有一个文件`a`, 它用某种摘要算法(专门生成数据摘要的哈希算法), 比如`MD5`, 生成了一份数据摘要, 此时, 即使对`a`改了一个字节, 那用同样的摘要算法所得出的数据摘要也会和之前的那份有很大的差别, 所以就可以快速判断出文件是否被篡改.

数据摘要有很多应用, 比如, 刚刚我们在HTTP里面说的 `Session ID`, 由于同一个站点内不会有相同的账号密码, 所以由此生成的`Session`文件也是不同的, 此时就可以使用摘要算法把`Session`文件转成一个固定的字符串, 作为用户身份的标识.                    另一个应用是, 网盘里面的"秒传", 对于网盘公司来说, 经常会遇到这种情况, 就是很多用户传了一样的文件, 比如最近一个站点提供免费的电影资源, 然后就有很多人从这个站点里下了一部电影, 由于资源都是一样的, 所以多个用户下载下来的电影其实都是一样的, 对于网盘公司来说, 它当然不想让自己硬盘里存着多份完全相同的文件, 所以当其中一个用户把电影传上去了, 它就会用这份电影生成一份数据摘要, 等到之后, 又有用户要上传同份文件, 网盘就先会在本地用摘要算法生成一份摘要, 结果往数据库里一比对, 发现, 有相同的, 那我就不传了, 此时网盘就会对之前的那份原始文件做类似于引用计数的操作, 比如弄个软链接指向那份原始数据.

数据摘要有时也需要再被加密, 此时得到的密文就被称为数字签名.

### HTTPS的工作流程

HTTPS的加密解密是怎么进行的呢? 由于空说太干巴, 所以我们将从最简单的方案开始, 逐一进行分析, 改进解决方案, 最终获得我们目前主流使用的主流方案.

**方案一: 只使用对称加密**

![image-20250409195346720](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250409195346812.png)

在这种情况下, 客户端和服务端通过某种方式共同维护着同一份密钥, 无论是请求还是响应, 都会先被密钥加密, 所以中间人无法获知其中的内容.

但这种方案有一个致命缺点, 就是客户端很容易被黑客直接攻击, 从而导致密钥泄露, 而且, 这种方案会导致客户端和服务端的解耦性太高, 比如服务端如果要升级加密方式, 它很难让所有用户一夜之间全换密钥, 有些用户, 他就不喜欢更新, 因为各种原因, 比如我的`vscode`已经不能更新了, 再更新就连不上我的`Centos9`了. 我们的密钥必须由服务器生成, 然后安全的传过去给客户端用. 

但很明显, 最起码在对称加密的情况下, 客户端与服务端正常通信前, 还要获得来自服务端的密钥, 由于你之前没有密钥, 所以密钥本身会以明文形式进行传输, 这就是掩耳盗铃.

**方案二: 只使用非对称加密**

既然仅靠对称加密握不了手, 那么我们就用非对称加密试试

![image-20250409202416156](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250409202416232.png)

服务端预先生成一对公钥私钥, 等待客户端连接, 最开始, 用的当然是HTTP协议, 但HTTP不安全, 所以客户端请求使用HTTPS, 服务端收到之后, 就会把公钥以正文形式交付给客户端, 在这之后, 就进入了HTTPS, 客户端的请求先借助于公钥变为密文, 然后传输到服务端, 服务端通过私钥还原为明文, 并把请求报文使用私钥加密, 交付给客户端, 客户端使用公钥解密, 转为明文.    

不过这个方案仍有不足之处, 尽管用户端发出的被公钥加密的密文黑客解不了密, 但服务端用私钥生成的密文, 黑客可以解密, 获知其中的内容, 但这还不是最严重的情况. 我们先不说最严重的情况

**方案三: 使用两对公钥私钥**

在这种方案下, 在正式通信前, 服务端和客户端都各自生成一对公钥私钥, 握手时交换彼此的公钥私钥

![image-20250409210122724](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250409210122798.png)

在握手成功之后, 客户端再发请求, 先要用服务端的公钥进行加密, 此时密文只能由服务端的私钥解密, 服务端发响应, 先用客户端的公钥进行加密, 此时密文只能由客户端的私钥解密, 这样就构成了安全信道

但这种方案首先, 由于使用了俩对非对称加密, 所以效率低, 速度也慢

另外, 这种方案和之前的方案一样, 都存在着致命性安全问题 

**方案四: 握手使用非对称, 正常通信使用对称**

![image-20250409213012966](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250409213013040.png)

服务端生成公钥和私钥, 将公钥发给客户端, 客户端本地创建一个对称密钥, 并使用公钥加密, 发给服务端, 服务端使用密钥解密, 获取对称密钥.

在握手阶段, 因为发的是公钥, 所以即使被黑客获取也没有关系, 因为靠这个公钥解密不出来对称密钥, 这样在正常会话的时候, 就可以直接使用对称密钥了.

现在我们已经解决了效率问题, 现在我们要看之前方案都存在的致命问题

![image-20250409214150885](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250409214150960.png)

在客户端明文发出请求协议升级时, 黑客就意识到了对应的服务端马上会分发公钥, 他自己生成一对公钥私钥, 等到服务端把包含公钥的报文发过来时, 对报文进行分析, 把其中的公钥换成黑客自己的公钥, 客户端使用黑客的公钥对本地生成的对称密钥进行加密, 黑客使用自己的私钥解密, 获取对称密钥, 黑客使用之前拦截下来的公钥对解密后的报文进行再加密, 发给服务端, 服务端拿到密文, 使用私钥解密, 获得对称密钥.

此处的关键在于, 客户端没有办法去查证拿到的公钥是不是服务端的公钥, 如果它能意识到拿到的实际是黑客的公钥, 就会进行规避动作.为此, 我们要想一个办法, 确保公钥不会被黑客替换, 确保公钥的合法性.

### 证书

服务端在使用HTTPS之前, 需要向CA机构申请一份数字证书, 数字证书中含有证书申请者信息, 公钥信息等, 服务器把证书传输给浏览器, 浏览器就可以从证书中获得公钥, 证书就如同身份证, 证明服务端公钥的权威性.

CA机构是一种权威性的机构, 它们能够给个人, 公司或者组织颁发 无法被伪造, 修改能被察觉的"证书", 这些"证书"里面就含有服务端的公钥, 所以只要客户端获得了证书, 就可以验证并获得服务端的公钥, 从而与服务端建立加密信道.

这个小节的知识点比较零碎, 所以我们找不到明确的线索把它们连起来, 只有把零碎知识全都说的差不多了, 才能把它们连起来. 放轻松, 先把这些零碎知识当做独立事件来看, 不要彼此弄混了.

我们先简略说说这个CA证书是怎么申请的. 首先负责运营服务端的个人或者集体需要先自己生成一对非对称加密的公钥和私钥, 然后收集一些能够证明自己身份的信息, 具有法律效力的身份信息, 比如, 网站的域名, 申请者或者组织是谁...... 再加上自己刚刚生成的公钥, 用这些信息生成一份`.csr`文件, 提交给CA机构, CA机构会对其中的信息进行审核, 审核通过后, 就会给服务端的管理者颁发一份数字证书. 然后之后服务端以客户端进行通信时, 服务端就不直接发公钥, 而是发这份数字证书, 这份证书里就含有用于进行cs通信的公钥, 客户端可以通过一些手段来验证证书的合法性, 验证通过, 就意味着里面的内容没有被替换, 就意味着里面的公钥没有被替换, 就是服务端的公钥, 从而避免了之前我们描述HTTPS工作流程所说的致命安全问题. 

这里有一张图, 我们先大致看看, 后面会说细节的:

![image-20250410211816814](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250410211817032.png)

然后我们看看这个证书里面大致有什么内容, 签名我们先略过, 签发机构就是这个证书是谁颁发的, 比如我们身份证上面也有签发机构, 有效时间表明了证书的有效时间(好像是废话), 就像身份证不是永久的, 证书也不是永久的, 只有一段的时间期限, 过期了, 浏览器就会说, 证书过期了, 不安全, 扩展信息和技术没有太大关系, 我们略过, 域名就是网站的域名是什么, 申请者描述了谁申请了这份证书, 公钥就是服务端在申请证书前生成的那份公钥, 就是`.csr`里面的那份公钥, 就是提交给CA机构的那份公钥, 就是服务端与客户端进行cs通信时, 从HTTP转为HTTPS的那份公钥, 等会儿还有个公钥, 注意不要弄混了.   注意, 服务端生成的那份私钥一直都被自己保管着, 并没有交给任何人, 并没有外泄, CA机构也不知道服务端的私钥.

下面我们要探讨两点, 一是该如何保证这份证书是CA机构颁发的, 而不是其它人伪造的, 二是该如何保证这张证书只要被改了, 客户端立刻就能发现, 从而避免数据泄露.这两个话题是靠签名实现的, 具体原理我们先不说.

我们换个话题, 先说`.csr`怎么生成, 你可以本地通过某些算法直接生成一份, 也可以在网上找一个在线生成网页生成. 这个仅仅是生成`.csr`, 还没有把`.csr`提交给CA机构

这里就有一个[在线生成csr文件的网站](https://myssl.com/csr_create.html)

![image-20250410214144262](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250410214144339.png)

把信息填好之后, 点击"生成", 网站就会自己生成一份私钥, 公钥, 然后把这份刚刚生成的公钥加到`csr`文件中, 并向你返回私钥和`csr`文件. 所以这个在线网站是可以知道私钥信息的, 因此要选择具有安全认证的在线网站.

之后, 服务端就可以把私钥放到本地, 再也不拿出来, 然后把`.csr`提交给CA机构, 不过我们一般不直接给CA机构, 因为它的认证过程很繁琐, 所以一般委托给提供相应服务的服务商, 让他们交给CA机构.

之后, 我们先了解一下什么是签名, 也就是证书里面的签名大概是什么生成的., 注意, 这下面也有一对公钥私钥, 这个公钥私钥可不是服务端的公钥私钥, 而是CA机构的公钥私钥, 千万不要混了.

![image-20250410215440960](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250410215441052.png)

首先我们有一份原始数据, 对于CA证书来说, 这份原始数据就是把CA证书的签名字段去除后剩下的原文内容, 然后使用哈希算法, 为这份原始数据生成一份数据摘要, 我们前面说过数据摘要, 我们说, 它可以用来验证原始文件是否遭到了修改., 之后CA机构用自己的密钥把这份数据摘要进行了加密处理, 所生成的这份密文就是签名, 随后把这份签名附加到原始文件上, 这样就得到了一份证书.  

等到客户端拿到这个证书之后, 先把签名和明文分离, 为明文再次生成一份数据摘要, 然后再使用内置在系统里的, 由CA机构发布的公钥去解密签名, 这样就得到了以前的数据摘要, 把现在的数据摘要和以前的数据摘要一比较, 就知道, 这份证书有没有被修改. 

如果黑客对明文进行篡改, 历史数据摘要和现在的数据摘要就会不相同, 客户端就知道这份证书不安全, 从而进行规避, 至于签名, 黑客改不动, 因为黑客没有CA机构的私钥, 如果他用别的私钥, 客户端手里的CA机构公钥就解不出来签名, 客户端就可以知道签名被篡改了, 执行规避动作. 当然, 用户无视风险继续访问是用户的事, 不是我们技术原因

如果黑客自己或者威胁他人, 办了一张真正的CA证书, 那么首先, 这份CA证书上面的身份信息就是线下找到黑客的线索, 其次, 证书上面有域名, 如果客户端访问的是`baidu.com`, 结果收到的证书是`xxx.com`, 那也能判断出证书不是自己要的, 所以连验证都不会验证.

这世界上只有CA机构能颁发证书, 因为证书里的签名必须用到CA机构的私钥, 而如果有人篡改, 通过签名解密之后的数据摘要也能判断出来.

在确认证书安全之后, 客户端就会从其中的明文拿到服务端的公钥, 由于证书安全, 所以这个公钥也一定是服务端的.  在这之后, 客户端本地生成对称密钥, 并使用公钥对其进行加密, 发回给服务端, 服务端收到密文之后, 使用私钥解密, 这样就能拿到对称密钥, 接着就转为HTTPS.

对于一些关键网站, 浏览器可能会自备证书, 此时第一次连接时客户端可以直接发加密的对称密钥, 服务端直接解密就行, 不用再让服务端再发证书. 

![image-20250411143707730](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250411143707943.png)

最后我们总结一下

HTTPS 工作过程中涉及到的密钥有三组

第一组(非对称加密): 用于校验证书是否被篡改. 服务器持有私钥(私钥在形成 CSR 文件与申请证书时获得), 客户端持有公钥(操作系统包含了可信任的 CA 认证机构有哪些, 同时持有对应的公钥). 服务器在客户端请求时， 返回携带签名的证书. 客户端通过这个公钥进行证书验证, 保证证书的合法性， 进一步保证证书中携带的服务端公钥权威性。

第⼆组(非对称加密): 用于协商生成对称加密的密钥. 客户端用收到的 CA 证书中的公钥(是可被信任的)给随机生成的对称加密的密钥加密, 传输给服务器, 服务器通过私钥解密获取到对称加密密钥.  

第三组(对称加密): 客户端和服务器后续传输的数据都通过这个对称密钥加密解密  

其实一切的关键都是围绕这个对称加密的密钥. 其他的机制都是辅助这个密钥工作的.  

>第⼆组非对称加密的密钥是为了让客户端把这个对称密钥传给服务器.  
>
>第一组非对称加密的密钥是为了让客户端拿到第⼆组非对称加密的公钥  

最后我们需要知道的是, 世上没有最安全的协议, HTTPS仍有弱点, 比如以往黑客都是攻击客户端的, 现在可以直接自己做客户端, 去攻击服务端......

## 传输层

应用层我们已经搞得差不多了, 下面我们重新回到传输层, 准备谈谈UDP和TCP它们的底层逻辑, 由于传输层的职责只有传输, 而不考虑其他, 比如应用层的数据安全需求, 报文完整性需求, 会话表示需求.... 非常多的需求, 所以应用层谈的比较散, 但传输层就不会这样, 逻辑会比较顺, 另外, 传输层, 网络层, 实际是系统的一部分, 所以我们又回到内核了, 下面我们就开始Linux内核网络部分的学习.

我们知道, 端口号标识了一个主机上进行通信的不同的应用程序.

![image-20250411150906863](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250411150906992.png)

在TCP/IP协议中, 用 "源IP", "源端口号", "目的IP", "目的端口号", "协议号" 这样一个五元组来标识一个通信(可以通过netstat -n查看);  

![image-20250411151013751](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250411151013947.png)

上图描述了一个网络服务的场景, 图中服务端以多执行流的状态建立与多个用户的会话, 客户端B只有一个页面, 所以只有一个会话, 客户端A有两个页面, 所以有两个会话, 并且这两个页面的端口号并不相同, 这样服务端才能对这两个页面进行区分.

多个画面, 实际上就是用户机器有多个页面访问服务端

![image-20250411151444249](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250411151444388.png)

端口号是16位的, 也就是`0-65535`, 其中`0-1023`被称为"知名端口号", HTTP(80), FTP(21), SSH(22)等这些广为使用的应用层协议, 他们的端口号都是固定的.  另外的, 也就是`1024-65535  `, 是操作系统动态分配的端口号. 客户端程序的端口号, 就是由操作系统从这个范围分配的.  不过也有些服务不在知名端口号, 比如MySQL(3306)

系统里也有对应的配置文件, 记录了知名服务对应的固定端口号, 我们可以执行`vim /etc/services`查看它们

![image-20250411152930361](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250411152931028.png)

对于这些知名服务的端口, 除非正式服务, 否则不要用. 

另外我们还需要知道的是一个进程也可以绑定多个端口号, 比如刚刚的Chrome, 当然, 服务端也可以多执行流绑定多个端口, 每个端口对应某个具体的活动.

下面我们正式认识一下netstat, 一个用来查看网络状态的重要工具.  以下是常见选项

- n 拒绝显示别名，能显示数字的全部转化成数字  
- l 仅列出有在 Listen (监听) 的服务状态 
-  p 显示建立相关链接的程序名  
- t (tcp)仅显示tcp相关选项  
- u (udp)仅显示udp相关选项  
- a (all)显示所有选项，默认不显示LISTEN相关 

```shell
[whisper@starry-sky projects]$ netstat
Active Internet connections (w/o servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 localhost:41398         localhost:38095         ESTABLISHED
tcp        0      0 localhost:41406         localhost:38095         ESTABLISHED
tcp        0      0 localhost:38095         localhost:41406         ESTABLISHED
tcp        0      0 localhost:38095         localhost:41398         ESTABLISHED
tcp        0      0 my-ubuntu:48716         169.254.0.55:5574       ESTABLISHED
tcp        0      0 my-ubuntu:38830         169.254.0.138:8186      ESTABLISHED
tcp        0      0 my-ubuntu:48714         169.254.0.55:5574       ESTABLISHED
tcp6       0    336 my-ubuntu:ssh           112.26.31.132:2827      ESTABLISHED
Active UNIX domain sockets (w/o servers)
Proto RefCnt Flags       Type       State         I-Node   Path
unix  3      [ ]         STREAM     CONNECTED     9527     
unix  3      [ ]         SEQPACKET  CONNECTED     9415     
unix  3      [ ]         STREAM     CONNECTED     4721189  
unix  2      [ ]         STREAM     CONNECTED     4693260  
unix  3      [ ]         STREAM     CONNECTED     4721192  
unix  3      [ ]         STREAM     CONNECTED     4721190  
unix  3      [ ]         STREAM     CONNECTED     4721193  
unix  3      [ ]         STREAM     CONNECTED     9197     /run/dbus/system_bus_socket
unix  3      [ ]         STREAM     CONNECTED     4721194  
unix  3      [ ]         STREAM     CONNECTED     4695267  
unix  2      [ ]         DGRAM                    4694328  /run/user/1003/systemd/notify
unix  3      [ ]         STREAM     CONNECTED     4721196  
unix  2      [ ]         DGRAM      CONNECTED     9403     
unix  3      [ ]         STREAM     CONNECTED     28499    /run/dbus/system_bus_socket
unix  3      [ ]         STREAM     CONNECTED     9306     
unix  3      [ ]         STREAM     CONNECTED     4695892  
unix  3      [ ]         STREAM     CONNECTED     4695266  
unix  3      [ ]         STREAM     CONNECTED     9361     
unix  2      [ ]         DGRAM                    14156    
unix  3      [ ]         STREAM     CONNECTED     4721191  
unix  2      [ ]         DGRAM      CONNECTED     3739     
unix  3      [ ]         SEQPACKET  CONNECTED     9414     
unix  3      [ ]         STREAM     CONNECTED     4721195  
unix  2      [ ]         DGRAM                    9348     
unix  3      [ ]         STREAM     CONNECTED     10505    /run/dbus/system_bus_socket
unix  2      [ ]         DGRAM      CONNECTED     4693853  
unix  3      [ ]         DGRAM      CONNECTED     4694329  
unix  3      [ ]         STREAM     CONNECTED     9582     
unix  3      [ ]         DGRAM      CONNECTED     7060     
unix  3      [ ]         STREAM     CONNECTED     4693926  /run/dbus/system_bus_socket
unix  3      [ ]         STREAM     CONNECTED     5697     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     4694834  
unix  3      [ ]         DGRAM      CONNECTED     7058     
unix  3      [ ]         DGRAM      CONNECTED     3939     
unix  3      [ ]         STREAM     CONNECTED     8648     
unix  3      [ ]         STREAM     CONNECTED     9787     
unix  3      [ ]         STREAM     CONNECTED     9792     
unix  3      [ ]         STREAM     CONNECTED     29412    
unix  3      [ ]         STREAM     CONNECTED     4694833  
unix  3      [ ]         STREAM     CONNECTED     5693     
unix  2      [ ]         DGRAM      CONNECTED     4694298  
unix  3      [ ]         DGRAM      CONNECTED     7059     
unix  2      [ ]         DGRAM      CONNECTED     9413     /run/chrony/chronyd.sock
unix  3      [ ]         DGRAM      CONNECTED     3479     
unix  2      [ ]         DGRAM      CONNECTED     6136     
unix  3      [ ]         STREAM     CONNECTED     9583     /run/dbus/system_bus_socket
unix  3      [ ]         STREAM     CONNECTED     4694290  
unix  3      [ ]         STREAM     CONNECTED     4694804  
unix  3      [ ]         STREAM     CONNECTED     10557    /run/dbus/system_bus_socket
unix  2      [ ]         DGRAM      CONNECTED     4694307  
unix  3      [ ]         STREAM     CONNECTED     9789     
unix  3      [ ]         STREAM     CONNECTED     9305     
unix  3      [ ]         STREAM     CONNECTED     9785     
unix  3      [ ]         DGRAM      CONNECTED     4694330  
unix  3      [ ]         STREAM     CONNECTED     9786     
unix  3      [ ]         STREAM     CONNECTED     9500     
unix  3      [ ]         STREAM     CONNECTED     4694820  
unix  3      [ ]         STREAM     CONNECTED     28494    /run/systemd/journal/stdout
unix  3      [ ]         DGRAM      CONNECTED     3480     
unix  3      [ ]         STREAM     CONNECTED     10556    
unix  3      [ ]         STREAM     CONNECTED     9790     
unix  3      [ ]         DGRAM      CONNECTED     3938     
unix  3      [ ]         STREAM     CONNECTED     4693891  /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     9788     
unix  3      [ ]         DGRAM      CONNECTED     7057     
unix  3      [ ]         STREAM     CONNECTED     4694819  
unix  3      [ ]         STREAM     CONNECTED     9503     /run/dbus/system_bus_socket
unix  3      [ ]         STREAM     CONNECTED     29398    
unix  2      [ ]         DGRAM      CONNECTED     4517     
unix  3      [ ]         STREAM     CONNECTED     10581    
unix  3      [ ]         STREAM     CONNECTED     4695890  
unix  3      [ ]         STREAM     CONNECTED     4695887  
unix  3      [ ]         STREAM     CONNECTED     8647     
unix  3      [ ]         STREAM     CONNECTED     8652     /run/dbus/system_bus_socket
unix  2      [ ]         DGRAM      CONNECTED     8551     
unix  3      [ ]         STREAM     CONNECTED     4142     
unix  3      [ ]         STREAM     CONNECTED     4432     
unix  3      [ ]         STREAM     CONNECTED     4352     /run/systemd/journal/stdout
unix  3      [ ]         DGRAM      CONNECTED     3478     /run/systemd/notify
unix  3      [ ]         STREAM     CONNECTED     4695886  
unix  2      [ ]         DGRAM      CONNECTED     8270     
unix  3      [ ]         STREAM     CONNECTED     10570    
unix  3      [ ]         STREAM     CONNECTED     4695889  
unix  2      [ ]         DGRAM      CONNECTED     8646     
unix  3      [ ]         STREAM     CONNECTED     10889    
unix  3      [ ]         STREAM     CONNECTED     8255     
unix  2      [ ]         DGRAM      CONNECTED     8521     
unix  2      [ ]         DGRAM                    3502     /run/systemd/journal/syslog
unix  10     [ ]         DGRAM      CONNECTED     3506     /run/systemd/journal/dev-log
unix  3      [ ]         STREAM     CONNECTED     8543     /run/systemd/journal/stdout
unix  8      [ ]         DGRAM      CONNECTED     3508     /run/systemd/journal/socket
unix  3      [ ]         STREAM     CONNECTED     8507     
unix  3      [ ]         STREAM     CONNECTED     9920     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     8650     /run/dbus/system_bus_socket
unix  3      [ ]         STREAM     CONNECTED     8256     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     4695885  
unix  3      [ ]         STREAM     CONNECTED     8509     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     4695888  
unix  2      [ ]         DGRAM      CONNECTED     3815     
unix  3      [ ]         STREAM     CONNECTED     10571    
unix  3      [ ]         STREAM     CONNECTED     4433     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     8416     
unix  3      [ ]         STREAM     CONNECTED     8651     /run/dbus/system_bus_socket
unix  3      [ ]         STREAM     CONNECTED     4695891  
unix  3      [ ]         STREAM     CONNECTED     8420     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     8541     
unix  3      [ ]         STREAM     CONNECTED     4694800  
unix  3      [ ]         STREAM     CONNECTED     10255    
unix  3      [ ]         STREAM     CONNECTED     4694797  
unix  2      [ ]         DGRAM      CONNECTED     10523    
unix  3      [ ]         STREAM     CONNECTED     10348    /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     8695     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     10535    /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     10345    
unix  2      [ ]         DGRAM      CONNECTED     10620    
unix  3      [ ]         STREAM     CONNECTED     4694799  
unix  2      [ ]         DGRAM      CONNECTED     10475    
unix  3      [ ]         STREAM     CONNECTED     9791     
unix  3      [ ]         STREAM     CONNECTED     4694801  
unix  3      [ ]         STREAM     CONNECTED     10533    
unix  3      [ ]         STREAM     CONNECTED     4694798  
unix  3      [ ]         STREAM     CONNECTED     9170     
unix  3      [ ]         STREAM     CONNECTED     8836     
unix  3      [ ]         STREAM     CONNECTED     8840     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     10582    
unix  3      [ ]         STREAM     CONNECTED     4694803  
unix  3      [ ]         STREAM     CONNECTED     8847     /run/dbus/system_bus_socket
unix  3      [ ]         STREAM     CONNECTED     4694802  
unix  3      [ ]         STREAM     CONNECTED     9171     /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     8694     
unix  3      [ ]         STREAM     CONNECTED     10256    /run/systemd/journal/stdout
unix  3      [ ]         STREAM     CONNECTED     8838     @a58564fd9bbcd1b0/bus/systemd/bus-api-system
unix  3      [ ]         STREAM     CONNECTED     8369     @8c16d6645d7d582f/bus/systemd-network/bus-api-network
unix  2      [ ]         DGRAM                    13744    @/usr/local/qcloud/YunJing/conf/ydrpc_3@
unix  3      [ ]         STREAM     CONNECTED     8370     @4d311898a53b2ecd/bus/systemd-resolve/bus-api-resolve
unix  3      [ ]         STREAM     CONNECTED     4694332  @28462715f50dfd29/bus/systemd/bus-system
unix  3      [ ]         STREAM     CONNECTED     8574     @f2c6bb1c11ef4e79/bus/systemd-logind/system
[whisper@starry-sky projects]$ #其中tcp udp开头的是网络套接字, unix开头的是域间套接字, 用于本地进程间通信
[whisper@starry-sky projects]$ #如果只想查看处于监听状态的服务加上"-l"选项
[whisper@starry-sky projects]$ netstat -l
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 0.0.0.0:http-alt        0.0.0.0:*               LISTEN     
tcp        0      0 _localdnsproxy:domain   0.0.0.0:*               LISTEN     
tcp        0      0 localhost:38095         0.0.0.0:*               LISTEN     
tcp        0      0 _localdnsstub:domain    0.0.0.0:*               LISTEN     
tcp6       0      0 [::]:ssh                [::]:*                  LISTEN     
udp        0      0 _localdnsproxy:domain   0.0.0.0:*                          
udp        0      0 _localdnsstub:domain    0.0.0.0:*                          
udp        0      0 my-ubuntu:bootpc        0.0.0.0:*                          
udp        0      0 localhost:323           0.0.0.0:*                          
udp6       0      0 ip6-localhost:323       [::]:*                             
raw6       0      0 [::]:ipv6-icmp          [::]:*                  7          
Active UNIX domain sockets (only servers)
Proto RefCnt Flags       Type       State         I-Node   Path
unix  2      [ ACC ]     STREAM     LISTENING     8361     /run/acpid.socket
unix  2      [ ACC ]     STREAM     LISTENING     4694331  /run/user/1003/systemd/private
unix  2      [ ACC ]     STREAM     LISTENING     8367     /run/dbus/system_bus_socket
unix  2      [ ACC ]     STREAM     LISTENING     4694339  /run/user/1003/bus
unix  2      [ ACC ]     STREAM     LISTENING     13746    /usr/local/qcloud/YunJing/conf/ydrpc_1
unix  2      [ ACC ]     STREAM     LISTENING     8371     /run/lxd-installer.socket
unix  2      [ ACC ]     STREAM     LISTENING     8373     /run/snapd.socket
unix  2      [ ACC ]     STREAM     LISTENING     8375     /run/snapd-snap.socket
unix  2      [ ACC ]     STREAM     LISTENING     4694340  /run/user/1003/gnupg/S.dirmngr
unix  2      [ ACC ]     STREAM     LISTENING     4694342  /run/user/1003/gnupg/S.gpg-agent.browser
unix  2      [ ACC ]     STREAM     LISTENING     8384     /run/uuidd/request
unix  2      [ ACC ]     STREAM     LISTENING     4694344  /run/user/1003/gnupg/S.gpg-agent.extra
unix  2      [ ACC ]     STREAM     LISTENING     4694349  /run/user/1003/gnupg/S.gpg-agent
unix  2      [ ACC ]     STREAM     LISTENING     4694351  /run/user/1003/gnupg/S.keyboxd
unix  2      [ ACC ]     STREAM     LISTENING     4694353  /run/user/1003/pk-debconf-socket
unix  2      [ ACC ]     STREAM     LISTENING     9818     /home/whisper/.local/share/code-server/code-server-ipc.sock
unix  2      [ ACC ]     STREAM     LISTENING     4694355  /run/user/1003/snapd-session-agent.socket
unix  2      [ ACC ]     STREAM     LISTENING     4694378  /run/user/1003/gnupg/S.gpg-agent.ssh
unix  2      [ ACC ]     STREAM     LISTENING     4694839  /run/user/1003/vscode-ipc-d16af049-0f39-46e6-8d27-358005af28d0.sock
unix  2      [ ACC ]     STREAM     LISTENING     4695912  /run/user/1003/vscode-ipc-f5a1baaf-541f-4ef6-b59d-d8e7e677d594.sock
unix  2      [ ACC ]     STREAM     LISTENING     4696125  /run/user/1003/vscode-git-188cd3ed6b.sock
unix  2      [ ACC ]     STREAM     LISTENING     3481     /run/systemd/private
unix  2      [ ACC ]     STREAM     LISTENING     3483     /run/systemd/userdb/io.systemd.DynamicUser
unix  2      [ ACC ]     STREAM     LISTENING     3484     /run/systemd/io.systemd.ManagedOOM
unix  2      [ ACC ]     STREAM     LISTENING     3499     /run/lvm/lvmpolld.socket
unix  2      [ ACC ]     STREAM     LISTENING     3504     /run/systemd/fsck.progress
unix  2      [ ACC ]     STREAM     LISTENING     3510     /run/systemd/journal/stdout
unix  2      [ ACC ]     SEQPACKET  LISTENING     3514     /run/udev/control
unix  2      [ ACC ]     STREAM     LISTENING     6190     /run/systemd/resolve/io.systemd.Resolve
unix  2      [ ACC ]     STREAM     LISTENING     6191     /run/systemd/resolve/io.systemd.Resolve.Monitor
unix  2      [ ACC ]     STREAM     LISTENING     3736     /run/systemd/journal/io.systemd.journal
unix  2      [ ACC ]     STREAM     LISTENING     4249     /run/systemd/io.systemd.sysext
unix  2      [ ACC ]     STREAM     LISTENING     3501     @/org/kernel/linux/storage/multipathd
unix  2      [ ACC ]     STREAM     LISTENING     8368     @ISCSIADM_ABSTRACT_NAMESPACE
[whisper@starry-sky projects]$ # 只想查看与TCP有关的监听服务
[whisper@starry-sky projects]$ netstat -tl
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 0.0.0.0:http-alt        0.0.0.0:*               LISTEN     
tcp        0      0 _localdnsproxy:domain   0.0.0.0:*               LISTEN     
tcp        0      0 localhost:38095         0.0.0.0:*               LISTEN     
tcp        0      0 _localdnsstub:domain    0.0.0.0:*               LISTEN     
tcp6       0      0 [::]:ssh                [::]:*                  LISTEN     
[whisper@starry-sky projects]$ netstat -t
Active Internet connections (w/o servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 localhost:41398         localhost:38095         ESTABLISHED
tcp        0      0 localhost:41406         localhost:38095         ESTABLISHED
tcp        0      0 localhost:38095         localhost:41406         ESTABLISHED
tcp        0     16 localhost:38095         localhost:41398         ESTABLISHED
tcp        0      0 my-ubuntu:48716         169.254.0.55:5574       ESTABLISHED
tcp        0      0 my-ubuntu:38830         169.254.0.138:8186      ESTABLISHED
tcp        0      0 my-ubuntu:48714         169.254.0.55:5574       ESTABLISHED
tcp6       0    224 my-ubuntu:ssh           112.26.31.132:2827      ESTABLISHED
[whisper@starry-sky projects]$ # 不带上"-l"选项将不展示监听服务, 而只显示其他状态的
[whisper@starry-sky projects]$ # "-a"选项显示所有状态
[whisper@starry-sky projects]$ netstat -ta
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 0.0.0.0:http-alt        0.0.0.0:*               LISTEN     
tcp        0      0 _localdnsproxy:domain   0.0.0.0:*               LISTEN     
tcp        0      0 localhost:38095         0.0.0.0:*               LISTEN     
tcp        0      0 _localdnsstub:domain    0.0.0.0:*               LISTEN     
tcp        0      0 localhost:41398         localhost:38095         ESTABLISHED
tcp        0      0 localhost:41406         localhost:38095         ESTABLISHED
tcp        0      0 localhost:38095         localhost:41406         ESTABLISHED
tcp        0     15 localhost:38095         localhost:41398         ESTABLISHED
tcp        0      0 my-ubuntu:48716         169.254.0.55:5574       ESTABLISHED
tcp        0      0 my-ubuntu:38830         169.254.0.138:8186      ESTABLISHED
tcp        0      0 my-ubuntu:48714         169.254.0.55:5574       ESTABLISHED
tcp6       0      0 [::]:ssh                [::]:*                  LISTEN     
tcp6       0    212 my-ubuntu:ssh           112.26.31.132:2827      ESTABLISHED
[whisper@starry-sky projects]$ # 另外我们发现有些服务后面没有显示端口号, 可以带上"-n"显示
[whisper@starry-sky projects]$ netstat -tan
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 0.0.0.0:8080            0.0.0.0:*               LISTEN     
tcp        0      0 127.0.0.54:53           0.0.0.0:*               LISTEN     
tcp        0      0 127.0.0.1:38095         0.0.0.0:*               LISTEN     
tcp        0      0 127.0.0.53:53           0.0.0.0:*               LISTEN     
tcp        0      0 127.0.0.1:41398         127.0.0.1:38095         ESTABLISHED
tcp        0      0 127.0.0.1:41406         127.0.0.1:38095         ESTABLISHED
tcp        0      0 127.0.0.1:38095         127.0.0.1:41406         ESTABLISHED
tcp        0     15 127.0.0.1:38095         127.0.0.1:41398         ESTABLISHED
tcp        0      0 10.0.12.6:48716         169.254.0.55:5574       ESTABLISHED
tcp        0      0 10.0.12.6:38830         169.254.0.138:8186      ESTABLISHED
tcp        0      0 10.0.12.6:48714         169.254.0.55:5574       ESTABLISHED
tcp6       0      0 :::22                   :::*                    LISTEN     
tcp6       0    112 10.0.12.6:22            112.26.31.132:2827      ESTABLISHED
[whisper@starry-sky projects]$ # 选项"p"显示对应进程信息
[whisper@starry-sky projects]$ netstat -tp
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (w/o servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 my-ubuntu:49124         169.254.0.4:http        TIME_WAIT   -                   
tcp        0      0 localhost:41398         localhost:38095         ESTABLISHED -                   
tcp        0      0 localhost:41406         localhost:38095         ESTABLISHED -                   
tcp        0      0 localhost:38095         localhost:41406         ESTABLISHED 850042/node         
tcp        0      0 localhost:38095         localhost:41398         ESTABLISHED 849977/node         
tcp        0      0 my-ubuntu:48716         169.254.0.55:5574       ESTABLISHED -                   
tcp        0      0 my-ubuntu:38830         169.254.0.138:8186      ESTABLISHED -                   
tcp        0      0 my-ubuntu:48714         169.254.0.55:5574       ESTABLISHED -                   
tcp6       0    280 my-ubuntu:ssh           112.26.31.132:2827      ESTABLISHED -                   
[whisper@starry-sky projects]$ # 有些看不到, 因为权限不够
[whisper@starry-sky projects]$ sudo netstat -tp
[sudo] password for whisper: 
Active Internet connections (w/o servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0     19 localhost:41398         localhost:38095         ESTABLISHED 849926/sshd: whispe 
tcp        0      0 localhost:41406         localhost:38095         ESTABLISHED 849926/sshd: whispe 
tcp        0      0 localhost:38095         localhost:41406         ESTABLISHED 850042/node         
tcp        0      0 localhost:38095         localhost:41398         ESTABLISHED 849977/node         
tcp        0      0 my-ubuntu:48716         169.254.0.55:5574       ESTABLISHED 2393/YDService      
tcp        0      0 my-ubuntu:60862         169.254.0.55:http-alt   TIME_WAIT   -                   
tcp        0      0 my-ubuntu:38830         169.254.0.138:8186      ESTABLISHED 1036/tat_agent      
tcp        0      0 my-ubuntu:48714         169.254.0.55:5574       ESTABLISHED 2393/YDService      
tcp6       0     96 my-ubuntu:ssh           112.26.31.132:2827      ESTABLISHED 849629/sshd: whispe 
[whisper@starry-sky projects]$ 
```

另外还有一些其它状态查询的指令, 比如`iostat`

```shell
[whisper@starry-sky projects]$ iostat
Linux 6.8.0-51-generic (my-ubuntu)      04/11/2025      _x86_64_        (2 CPU)

avg-cpu:  %user   %nice %system %iowait  %steal   %idle
           0.41    0.01    0.77    0.14    0.00   98.66

Device             tps    kB_read/s    kB_wrtn/s    kB_dscd/s    kB_read    kB_wrtn    kB_dscd
loop0             0.00         0.00         0.00         0.00         14          0          0
sr0               0.00         0.05         0.00         0.00       8434          0          0
vda               5.53         6.72        58.71         0.00    1135185    9917409          0


[whisper@starry-sky projects]$ 
```

对于某些守护进程来说, 用`ps`查起来很麻烦, 此时就可以使用`pidof`直接查

```shell
[whisper@starry-sky Network]$ sudo ps ajx | head -1 && ps ajx | grep sshd | grep -v grep
   PPID     PID    PGID     SID TTY        TPGID STAT   UID   TIME COMMAND
      1    1372    1372    1372 ?             -1 Ss       0   0:00 sshd: /usr/sbin/sshd -D [listener] 0 of 10-100 startups
   1372  849629  849629  849629 ?             -1 Ss       0   0:00 sshd: whisper [priv]
 849629  849926  849629  849629 ?             -1 S     1003   0:01 sshd: whisper@notty
[whisper@starry-sky Network]$ sudo ps ajx | head -1 && ps ajx | grep sshd | grep -v grep | awk '{print $2}'
   PPID     PID    PGID     SID TTY        TPGID STAT   UID   TIME COMMAND
1372
849629
849926
[whisper@starry-sky Network]$ # 有时候, 要把一堆服务都停止
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ps ajx | head -1 && ps ajx | grep tcpserver | grep -v grep |awk '{print $2}' 
   PPID     PID    PGID     SID TTY        TPGID STAT   UID   TIME COMMAND
884286
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ps ajx | head -1 && ps ajx | grep tcpserver | grep -v grep |awk '{print $2}' 
   PPID     PID    PGID     SID TTY        TPGID STAT   UID   TIME COMMAND
884286
884835
884856
884866
884875
[whisper@starry-sky Network]$ #由于它们是守护进程, 所以要用kill -9 停止 
[whisper@starry-sky Network]$ # xargs指令可以把标准输入转化为命令行形式
[whisper@starry-sky Network]$ ps ajx | head -1 && ps ajx | grep tcpserver | grep -v grep |awk '{print $2}' | xargs kill -9
   PPID     PID    PGID     SID TTY        TPGID STAT   UID   TIME COMMAND
[whisper@starry-sky Network]$ ps ajx | head -1 && ps ajx | grep tcpserver | grep -v grep |awk '{print $2}' 
   PPID     PID    PGID     SID TTY        TPGID STAT   UID   TIME COMMAND
[whisper@starry-sky Network]$ # 此时就可以使用pidof
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ./TCP/tcpserver -8888
[whisper@starry-sky Network]$ ps ajx | head -1 && ps ajx | grep tcpserver | grep -v grep |awk '{print $2}' 
   PPID     PID    PGID     SID TTY        TPGID STAT   UID   TIME COMMAND
887170
887195
887204
887219
[whisper@starry-sky Network]$ pidof tcpserver
887219 887204 887195 887170
[whisper@starry-sky Network]$ pidof tcpserver | xargs kill -9

```

### UDP

在讲解传输层和网络层的的相关协议时, 我们要围绕的话题是**报头和负载该如何分离**, **怎样决定有效载荷应该交付给更上层协议中的哪一个**

我们先看看UDP的报文是什么样的

![image-20250411163844645](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250411163844778.png)

UDP怎么分离报头和负载的呢? 其实很简单, 它采用的是定长模式, UDP报文前八字节就是它的报头, 剩下的就是负载, 怎么交给更上层呢? 在报头中, 我们可以看到有一个"目的端口号", 通过这个"目的端口号"就能找到与之绑定的`socket`文件, 就能把收到的负载写进去, 进程再通过`socket`获取负载.

两个端口号我们就不说了, "16位UDP长度"表示了整个报文的长度,减八字节就是负载长度, "16位UDP校验和"可以对正文进行校验, 如果校验出错, 这份报文就会被直接丢弃.

UDP传输的过程类似于寄信 

- 无连接: 知道对端的IP和端口号就直接进行传输, 不需要建立连接;  
- 不可靠: 没有确认机制, 没有重传机制; 如果因为网络故障该段无法发到对方, UDP协议层也不会给应用层返回任何错误信息;
- 面向数据报: 不能够灵活的控制读写数据的次数和数量;  

"面向数据包"的特点是你发几次, 对方就收几次, UDP报文之间为独立关系,   另外, UDP没有真正意义上的 发送缓冲区. 调用sendto会直接交给内核, 由内核将数据传给网络层协议进行后续的传输动作;  UDP具有接收缓冲区. 但是这个接收缓冲区不能保证收到的UDP报的顺序和发送UDP报的顺序一致, 可能一个报文明明是后发的, 但路由选择比较好, 反而先被收到, 我们把这种情况叫做报文发生了乱序, 对于这种情况, 乱序只能通过我们自己在应用层解决; 如果缓冲区满了, 可能是收到的报文数量太多, 再到达的UDP数据就会被丢弃; 

另外要注意的一点是, UDP的长度字段只有16位, 所以一个UDP报文最多时64KB, 所以如果要发超过64KB大小的数据, 你要想办法把它们拆分成小于64KB的小份, 这些小份可能要依据应用层的种种操作标记一下顺序, 然后再一份份发过去.

UDP十分简单, 所以它适用于对速度很有要求的场景, 比如, 网络直播, 打游戏什么的,  由于UDP不可靠, 所以可能出现丢包情况, 体现就是视频突然模糊了一下.

和应用层搞序列化和反序列化不同, 应用层是因为它有时经常要改动, 所以序列化和反序列化就能适配更多的场景, 但内核并不搞序列化和反序列化, 它真的直接用结构体, 具体的来说, 它实际上是把报头中的成员以位段的形式存在结构体中

```c
struct UdpHeader
{
    unsigned int src_port : 16;
    unsigned int dst_port : 16;
    unsigned int length : 16;
    unsigned int check_code : 16;
};
```

UDP虽然没有缓冲区, 但还是要被系统进行管理的, 对UDP报文进行管理的结构被称为`sk_buff`, 它也承担着把报文和负载结合起来的功能

````c
struct sk_buff
{
    char* start;
    char* end;
    char* pos;
    int typr;
    ....
    struct sk_buff* next;
};
````

然后我们可以从内存找一片空间作为UDP报文存放的场所

![image-20250411181213150](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250411181213329.png)

丢弃报文的对应动作就是把`struct sk_buff*`给释放.

### TCP

UDP我们随便说说就行了, 关键是TCP.我们会认真说道说道的.

TCP全称为 "传输控制协议(Transmission Control Protocol"). 人如其名, 要对数据的传输进行一个详细的控制;  我们先简要理解一下, TCP在控制什么东西

我们可以画一根线, 线上代表的应用层, 我们可以在上面写些服务, 比如之前我们自定义协议写的网络计算器, 线下面我们就只看传输层, 虽然我们说TCP有可靠性, 所以用`write, send`, 可以认为字节流就进网络了, 但实际上, 并不是这样, `read, recv, write, send`它们的实际作用只是把用户缓冲区的内容拷到TCP的发送缓冲区, 或者把TCP接收缓冲区的内容拷贝到应用层的接收缓冲区, 比如我们之前定的`buffer`, `string`什么东西的, 应用层的报文, (一般叫做请求或者响应), 只是来到了TCP的发送缓冲区, 至于它什么时候发送, 发送多少, 怎么发送, 出错了怎么办? 这些都不用我们用户操心, 而是由TCP自己控制, 而且, 服务端和客户端用的都是TCP, 大家都是对等的, 对于另一台机器来说, 也是这样

![image-20250412220403590](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250412220403714.png)

另外, 我们其实也能感受到, 类似的话我们在文件里似乎也说过.   我们说每个文件都有自己的缓冲区, 我们使用`read, write`, 只是把数据从用户层缓冲区移到了内核层缓冲区, 至于内核层缓冲区中的数据, 什么时候写到文件里, 怎么写到文件里, 和磁盘怎么协商 都由系统自己决定, 用户不用关心, 今天我们学习网络, 就相当于把磁盘换成网卡, 这里再说一下, TCP的缓冲区实际上用的就是socket文件的缓冲区.

所以我们对于网络就可以这样理解, 应用层只进行数据处理, 比如什么加密解密, 计算, 查数据库什么的, 数据的发送则由系统决定. 不过我们之前谈文件的时候, 似乎没有谈过出错的情况, 那是之前的文件读写都是在本地, 出错概率很小, 但网络是长距离传输, 所以出错概率很是挺大的, 我们也需要特别说说.      网络发送的实质就是把我TCP发送缓冲区里的数据通过网络拷贝到对面的TCP接收缓冲区.

有时候在调用`read, write, recv, send`这类接口时, 可能会出现阻塞的情况, 出现这种情况的原因就是, TCP的缓冲区满了或者空了, 不能再往里面写数据或者读数据, 所以系统把进程就给挂起了, 这就是等待其它计算机资源就位.

另外, 由于发送接收缓冲区彼此独立, 所以可以对同一个socket一边读, 一边写, 这就叫做全双工.

另外, 我们还需要说的一点是, 这个TCP的两个缓冲区, 其实也是内存中某种空间, 我们知道, Linux一切皆文件, 所以TCP的这两个缓冲区实际上就是socket文件的缓冲区, 对于文件来说, 它的实际拥有者是系统本身, 普通进程对于文件只有使用权, 系统这个进程它也有自己的地址空间, 这个地址空间用的也是虚拟地址, 其中就包含着系统中被打开文件的缓冲区, 总而言之, TCP中的缓冲区是内存中的一处空间

另外, 我们也知道, 系统为了对内存这种十分关键的计算机资源进行管理, 将物理内存分为一个个页框进行管理, 每个页框的大小都是4KB, 页框都有与之对应的`struct page`, 系统把这些`struct page`集中放到某个特别的位置, 实际上就形成了一个数组,  虚拟内存通过页全局目录, 页目录, 页表这种树状结构, 就能通过下标指向`struct page`数组中的某个特定元素, 从而将虚拟地址映射到物理地址上, 然后由于物理地址有页框这种概念, 所以虚拟地址也有页框的概念, 文件缓冲区处于系统这个进程的地址空间中, 所以它也有页框的概念, 因此我们就可以对缓冲区进行一块一块的划分, 从而让整个缓冲区有数组的感觉, 这段话什么意思呢? 就是说, 通过某个数字, 是可以地定位缓冲区的某个具体位置, 这个数字其实就是下标, 至于这个数组它究竟长什么样, 我们先不考虑.

下面, 我们看看TCP协议的段格式

![image-20250413155330367](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250413155330455.png)

我们在应用层, 一般喜欢把报文叫做"请求, 响应", 而在传输层, 我们一般把报文叫做"报文段", 所以上面说是"段格式",  "IP"层一般叫做"数据报", 链路就是"数据帧"了, 这在之前的过程中, 我们也说过. 

我们首先要解答的问题是, 如果我收到一份TCP报文段, 该如何把报头和负载分离. 

我们先来看看TCP的报文段, 它可以分为三部分, 一是前20字节的标准报头, 标准报头的意思是, 只要是一个报文段, 那都会有, 接着被标记为"选项"的那部分就是"非标准报头", 用得很少, 我们就不谈它了, 接下来就是数据或者说负载了. 

首先我们看一下"16位源端口"和"16位目的端口", 这个很简单, 就是描述通信双方的端口号的, 没有端口号就找不到`socket`文件, 找不到`socket`就找不到进行网络通信的两个进程, 进程都找不到, 怎么进行服务呢?

如果想要将报头和负载相分离, 首先你要确定整份报文段的大小, 这个大小TCP协议并不提供, 而是由IP协议提供, 我们收到的报文段是IP协议交上来的, IP协议会告诉我们这个报文段究竟有多大. 接着, 我们只需要确定报头的长度就行了, 标准报头的长度是固定的20字节, 接下来我们只要确定非标准报头的大小, 这是借助于4位首部长度来确认的, 顾名思义, 4位首都长度就是一个4比特位的数据, 描述的是报头的总长度, 需要注意的是, 它为了节省空间, 并不是直接用一字节的单位进行描述的, 毕竟4比特位最大15, 都不够20, 而是用4字节做单位的,  所以它能表示的最长长度是15 * 4 = 60字节. 当然, 选项部分可能不够4字节, 对于这种情况, 它会补齐, 它有内存对齐策略, 所以报头都是4的倍数, 这样对于一个没有选项的报文段, 4位首部长度就是5, 也就是4位首部长度的默认值`0101`, 

因此, 报头和负载的分离过程是这样的, 首先TCP协议先读20个字节, 把其中的4位首部长度拿出来, 确定报头的长度, 接着把剩下的报头, 也就是选项那部分再读出来, 剩下的就是数据, 会被放到缓冲区里面.   当然, 由于它用的是位段形式, 所以可能是直接把前20字节解释成对应的结构体`struct TcpHeader`, 然后获得标准报头中的种种数据. 这样, 我们就解决了报头负载分离问题.

接下来我们先离开报文段格式, 先去讲讲其它东西. 但在说其它东西之前, 我们先要特别说一点, 那就是应用层的两个进程在基于TCP协议进行网络通信的时候, 发送的都是一份绝对包含标准报头的, 完整的报文段, 收到的报文段也绝对有前20字节, 下面我们可能为了图方便, 会省略说什么传"标志位", "序号"什么东西, 但你一定要知道, 我传的不仅仅是那一个标记位, 一串序列号, 而是整个标准报头, 只不过这个报头上的标记位, 序列号被设置罢了, 看其他相关书籍的时候, 也要记住这一点.

下面我们谈一下TCP保证可靠性的一种措施, 叫做"流量控制".

尽管正如我们之前所说, TCP它有缓冲区, 这个缓冲区实际上socket文件的缓冲区, 这样, 应用层的进程, 它开了几个基面向字节流的socket文件, 它就有几对接收, 发送缓冲区, 但这缓冲区毕竟是有大小的, 会不会发生这样的一种情况, 那就是, 报文段确实顺利到达对面了, 也成功进行了报头和负载的分离, 也找到了与目的端口号绑定的socket文件, 但当我想把负载放进去的时候, 发现, 缓冲区快满了, 放不下了, 此时我怎么办呢? 像UDP那样扔掉吗? 那样就丢包了 

其实对于TCP来说, 根本不会有上述的情况发生, 因为这其实是不可靠的表现, TCP会通过一些手段或者措施, 让发送方在发送之前就了解到接收方的缓冲区容纳能力, 如果无法容纳, 发送方就会等一会儿发, 或者把数据拆成更小的份, 这一小份接收方还能放得下, 先把这一小份发过去.  我们把这种, 依据接收方的数据容纳能力, 发送方主动调节发送速率, 或者发送的数据大小的这种措施叫做"流量控制".        有人可能说, 我知道TCP还有一个保持可靠性的措施, 叫做"丢包重传", 那能不能也不要什么"流量控制", 我发送方就闭着眼传, 对方缓冲区放不下, 那就重传呗, 这确实是一种可行的方案, 但很明显, 这种方案有些自讨苦吃了, 你明明知道对方装不下了, 还传过去, 你这不是故意要重传吗? 另外, TCP协议是系统的一部分, 系统要尽可能让计算机中的各类资源得到充分的利用, 网络也是一种资源, 也需要高利用率, 你明知道对面放不下, 为什么还要浪费网络资源, 千里迢迢把报文段传过去呢? 所以这种方案虽然可行, 但不能真的用.

本来, 按照逻辑来说, 我应该去说说到底是通过什么"手段或者措施"来实现"流量控制"的, 但我们先再换一个话题, 再引入一个概念, 否则解释起来不好理解.

TCP还有一个确保可靠性的机制, 叫做"确认应答".									

我们先引入一个生活场景, 你到了一个很远的地方, 安全的到达了, 而你的家人很牵挂你, 你为了让他们放心, 就打了一个电话, 报了平安. 这其实就是一种确认应答, 家人不知道你有没有安全到达, 你打了个电话让他们知道你安全到达了.

同样的, 发送方发了一封报文段, 它怎么知道接收方有没有收到呢? 答案是, 接收方收到这份报文段之后, 会立刻向发送方再发一封报文段, 发送方收到这份报文段, 就知道, 对方收到我刚刚发的报文段了, 那我这边就可以把发送缓冲区里面的对应数据删掉了, 如果发送端过了一段时间, 一段计算机能察觉出, 但我们察觉不出的时间, 还收不到确认应答, 那就认为对方没有收到, 那我就有可能进行重传操作.

上面我们没有明确进行身份划分, 那现在我们再把应用层的身份引入进来, 重述一遍, 现在我们有`client`和`server`, 当`client`向`server`发送一封报文段后, 即使`server`的应用层并没有向自己的传输层发任何数据, TCP协议也要自己造一个空的, 没有负载的, 只有标准报头的报文段, 给`client`的传输层发过去, 这样`client`的传输层才知道自己刚刚发的负载对面成功收到了, 它才会放心的把这份负载从发送缓冲区中清除, 并且, 在传输层, 双方的地位是对等的, 并不是传输层的那种`server`被动与`client`通信的那种情况, 所以如果`server`向`client`发送一封有实际负载的报文段, `client`的传输层在收到之后, 也必须向`server`的传输层发一封确认应答报文段.

下面我们可以说说"流量控制"到底是通过什么"手段或者措施"来实现的了.

很明显, "流量控制"的前提是发送方(`Sender`)要知道接收方(`Receiver`)缓冲区的存储状况, 知道还剩下多少空间, 那这到底是怎么知道的呢? 其实很简单, 这就是依靠"确认应答"实现的, 让我们把时间稍微往前拨一点, 在之前, Sender就已经给Receiver发送过了一份报文段, 作为收到该报文段的回应, Receiver向Sender发回了一份"确认应答", 这份"确认应答"有着完整的标准报头, 而标准报头中就说明了Receiver的接收缓冲区剩余空间, 所以现在Sender就知道了对面剩下的空间大小.

那到底是标准报头中的哪个字段描述了缓冲区的剩余大小呢? 答案就是"16位窗口大小", 不管你是谁, TCP协议在构造报头的时候都会先瞅一眼自己本端接收缓冲区的剩余大小, 然后写到16位窗口大小里, 发过去, 对端就知道了自己剩余的接收缓冲区大小, 从而实现端对端的相互流量控制. 

对了, 传输层是对等的, 所以常用的称呼是发送方, 接收方, 本端, 对端, 服务端和客户端是应用层的说法, 我们这里除非涉及到具体的应用场景, 否则都不会用.

接下来我们说说"确认应答"的细节, 其中包括"32位序号"和"32位确认序号"这两个位段

首先我们需要认识到的是TCP协议不会对确认应答进行确认应答, 这也好理解, 真要这样做的话, 那就停不下来了, 双方一直在确认应答.  比如, 现在有两个机器, 我们将它们记为"A", "B",  现在"A"向"B"发送一份消息, "B"在收到后, 为了告知"A"自己收到了这份消息, "B"向"A"发了一封确认应答, 现在"A"收到了这份确认应答, 它就知道, 刚刚发的消息对面收到了, 也就是说, 从"A"到"B"的这个方向, 可靠性是确定的, 但现在"A"为了让"B"知道自己收到了这份确认应答, 它向"B"发送了一份确认应答, 以保证"B"到"A"这个方向上的可靠性, "B"在收到这份确认报文就知道, 自己刚刚发的确认报文对面是收到的, 但对面现在并不知道我有没有收到它发的确认应答, 所以我应该再给它发一份确认应答, 表明自己收到了"A"发的确认应答....................

这样的话, 就会没完没了, 尽管当初"A"为了确保"B"知道自己收到了他发的确认应答, 而对其发送了确认应答, 这个保证"B"到"A"方向可靠性的本意是好的, 但是没有实际意义, 所以对于纯粹的确认应答, 所谓纯粹就是完全只有报头, 没有负载的那种, 我们收到之后, 就不用再向对面发送确认应答了.

不过由于"A"不因为收到"B"的确认应答, 而向"B" 再发确认应答, 所以, 我们就只能确保"A"到"B"方向的可靠性, 却无法确保"B"到"A"方向的可靠性, 故而我们说没有完全可靠的通信协议, 

这里要注意, 我并没有说"B"到"A"方向可靠性一直得不到保证, 而只是说, 单论  "A"收到"B"的确认应答这个场景下, "B"到"A"的方向可靠性得不到保证,但"A""B"两台机器是一直在互动的, 当"B"向"A"发送有实质内容的信息时, 确认应答就能保证"B"到"A"方向的可靠性, 但保证不了"A"到"B"方向的可靠性

![image-20250414130245703](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250414130245802.png)

![image-20250414130653706](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250414130653791.png)

尽管就单个场景来说, 无法确保两个方向上的可靠性, 但多个场景合起来, 就能确保两个方向上的确认应答.    也就是说, 局部上只能保证一个方向上的可靠性, 但整体上两个方向上的可靠性都能保证.

当然可能存在一种小概率事件, 那就是本端确实收到了信息, 也向对端发送了确认应答, 但对端没有收到, 所以对端就认为本端没有收到消息, 从而进行报文重传, 但这种情况也不需要太担心, 首先, 这是小概率事件, 其次, 本端有能力察觉出这是已经接受过的数据, 我们就不展开讲了.

我们上面谈到了一个词, "纯粹的确认应答", 有没有不纯粹的呢? 当然是有的, 并且这实际上是更常见的情况. 这种情况实际上是一个效率优化, 比如, 如果用的都是"纯粹的确认应答", 我们平常的对话就是这样的, 

A: "晚上一起去吃饭吗?"
B: "收到"
B: "去"
A: "收到"
A: "吃什么"
B: "收到"
B: "吃盖浇饭"
A: "收到"

这明显不太对, 所以实际上更多的场景是, 我们不单纯发一个确认应答, 而是传输层先等一小会儿, 看应用层有没有把消息的处理结果发下来, 那样我就把这个处理结果放到确认应答的负载里, 或者在发送缓冲区找点数据, 放到确认应答负载里, 总之, 尽量不要光发一个确认应答, 对于这种有数据的确认应答, 那对面收到之后, 就需要进行确认应答了, 这个确认应答答的不是报头里记录的确认应答, 而是报文里面的负载. 

另外, 我们上面的这种通信, 是串行的, 收到确认应答再发下一个数据, 这样效率就会非常低下. 所以在实际通信中, 我们用的方案都是并行通信, 本端同时发送多次消息, 这样, 在理论上, 对端也要发送多个确认应答.

![image-20250414133837551](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250414133837641.png)

不过, 这样就引发了一个问题, 尽管上图中的四个报文段, 我们依据发送时的顺序编号为"a,b,c,d",   但对面收到的顺序可不一定是"a,b,c,d", 因为网络状况是在时刻发生变化的, 所以可能恰好"a"发的时候最短的那个路由走不过去, 所以它选择了一个更长的路由, 别的报文则走了最短的那个路由.  在UDP里面, 我们也说过这种现象叫做 "乱序", "乱序"是不可靠的表现, 所以TCP也需要通过一些机制, 把乱序给修正回来.如果不修正, 应用层收到的就是无法被解析的数据

因此, 我们就需要给每个报文进行编号, 把编号写到报头里, 对面收到的时候再依据编号判断是否乱序, 乱序了再修正回来, 这个编号在报文段格式里, 就是两个序号, "32位序号"和"32位确认序号", 下面, 我们就探讨一些这两个序号.

我们之前说过, TCP缓冲区它是一格一格的, 或者我们可以认为, 它就是一个`char`类型的大数组, 里面的数据, 由于是直接从应用层缓冲区里面拷过来的, 所以都是符合应用层顺序要求的, 并且, 由于由于这是一个字符数组, 所以其中的每个字符都有与之对应的数组下标, 当我们要把其中的数据作为TCP报文负载发出去的时候, 就把负载中最后一个字符的数组下标作为"32位序号". 

![image-20250414141259396](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250414141259496.png)

当对面收到数据之后, 就可以依据序号把数据填到缓冲区里, 从而在不经意间完成顺序的修正.

另外, 那个"确认序号", 顾名思义就是表示确认应答和之前发送的报文之间的对应关系的, 它在数值上, 是与之相对报文的"确认序号"再加上一, 比如, 上面的那张图, 如果报文的序号是"1000, 2000, 3000, 4000", 那么与之对应的确认应答就是"1001, 2001, 3001, 4001"

![image-20250414142801886](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250414142801962.png)

"确认序号"除了上面表示数据与确认应答映射关系的意思外, 还有一种意思, 那就是表示"确认序号之前的数据,  我已经全部收到了!", 比如, 对与上图中, 其实可以只发"4001"这一个确认应答. 由于因为"1000, 2000, 3000, 4000"都在"4001"前面, 所以只靠这一个确认应答, 我就可以知道之前发的数据对面都收到了, 还有一种意思是, 下次发送请从确认序号开始进行发送, 比如我这里收到了"4001"确认应答, 那就意味着, 下一次数据从"4001"这个位置开始,  上面这张图的序号是乱填的, 不要在意具体数值.

有一个面试题, 是这样的, 为什么不能把序号和确认序号合并呢? 答案很简单, 因为它们两个根本不是一个概念, 序号说的是"我发送的数据结尾是什么", 确认序号说的是"我收到了你的第几个数据", 它们根本不是一个东西, 无法合并在一起.

比如我们举个实例, 之前我们说过一个"非纯粹的确认应答", 其实它有专门的名字, 叫做"捎带应答", "捎带应答"同时存在着两种不同的信息: "我发了什么", "我收到了什么", 它们完全不能合并在一起.

接下来我们去看协议段格式里面的六个标记位, 就是这部分

![image-20250414153616891](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250414153617019.png)

这些标记位是干什么的呢? 实际上就是帮助TCP协议实现多态调用, TCP协议是用C写的, 但这并不意味着C里面没有面向对象, 继承多态的概念, 实际上, 对于系统来说, 用C实现的面向对象, 继承多态非常常见, . 只是方式不同于 C++ 或 Java 语言，而是通过结构体、函数指针、状态机等方式间接实现。

在使用TCP协议进行网络通信的整个生命周期中, 会经历多个过程, 在不同的过程中, 报文所承担的职责不同, 对应的行为也就不同, 比如我们曾经略微提过, TCP有"三次握手"建立连接, "四次挥手"断开连接的机制, 只有在连接完成连接并且未启动关闭机制时可以进行通信, 此时的报文起到的就是信息传递的功能, 而在连接建立, 连接断开的过程中, 报文并不进行任何应用层方面的信息传递, 而是在进行信息传递的准备和善后操作, 对方如何知道收到的这份报文是处于什么状态呢? 那就需要查看它的六个标记位.

我们先看标记位**ACK**, 它用来描述确认序号是否有效, 或者说, 它描述了这个报文是否具有确认应答的功能. 不过一般来说, 在正常通信过程中, 它一般都是被置为一, 因为在正常通信中, 捎带应答是主流, 光一个应答反而少. 一般来说, 在三次握手结束后, 这个ACK一般都是一了.

标记位**SYN**, 表示请求建立连接, 我们把携带SNY标识的称为"同步报文段". 在应用层的cs模式中, 一个服务端会对应多个客户段, 因此服务器的TCP协议会同时和多个客户端的TCP协议相互之间进行通信, 此时客户端的TCP报文段, 有的是正常通信, 有的是刚连上, 请求三次握手, 还有请求断开连接的, 服务端TCP该如何进行分辨呢? 那就是通过这些标记位, 当服务端TCP收到一个SYN标记为一的报文段, 就知道, 对面的那个客户端请求进行三次握手, 客户端TCP想与我建立连接

标记位**FIN**, 表示通信完毕, 告诉对端, 本端要关闭了. 

标记位**PSH**, 催促对端赶快把接收缓冲区清出一片空间. 这个催促信息只会传递到对端的TCP协议, 不会传到应用层, 平常, TCP协议可能会先在自己的接收缓冲区里先屯多点数据, 然后一次性交给应用层, 这样效率更高, 但如果某个机器的TCP协议收到了含有PSH的报文段, 它就不会再屯了, 能往应用层推送多少就推多少, 就细节上来说, 它会改变对端`read`或者`recv`读取数据的程度. 当然, 如果应用层不调`read, recv`那系统也没办法,不过这也意味着应用层设计有问题, 或者我们也可以从生产消费者模型的角度出发, 接收方接收缓冲区就是一份临界空间, 发送方TCP协议往里面扔数据, 接收方应用层从里面读数据, 如果临界区满了怎么办, 那就让发送方TCP协议的写端口阻塞, 这样其实就有多执行流数据同步的那种感觉,  所以`write, send`它们有时候阻塞. 当然`read, recv`也可能读不出来东西也阻塞. 如果真的发生了这种场景, 要么是发送方轮询接收方的窗口大小, 要么是接收方窗口清出来之后主动联系发送方, 告知它可以继续进行正常通信了. 否则就一直在阻塞.   另外如果`PSH`太久, 发送方可能认为连接出异常从而主动断开连接. 刚刚我们说的场景是发送方发送缓冲区快满了, 所以催促, 实际上, 另一种场景是, 发送方的应用层已经把要传的数据都传过去了, 所以在传最后一个数据时, 可以在`send`中的后续参数(`send`还有多参数版)进行设置, 应用层指定把发送报文段设置成PSH, 这样对端收到就会把数据尽快往上传, 从而让对端应用层更早收到全部数据, 这是发送方缓冲区没有快满的场景, 是应用层主动要求设置成PSH的, 对于这种场景, 可能携带PSH的报文段中也有数据.

标记位**RST**, 表示通信双方就连接状况的认知不一致, 请求重新建立连接. 一般用在握手失败的场景.我们知道TCP有所谓"三次握手", "四次挥手"的机制, 三次握手保证了后续正常通信的可靠性, 但握手本身也可能失败, 而且可能有各种各样的原因, 可能因为网络中断、目标主机未开启服务，或是双方系统内部状态异常等各种原因导致失败。, 反正出错情况非常多, 由于太多了, 所以当TCP连接建立失败后, 不会尝试修复连接, 而是像我们电脑出问题后, 重启那样, 重新建立连接, 把状态变成最初的状态, 所以携带这种标记位的称为"复位报文段".   "认知不一致"我们会在探讨"TCP三次握手的意义"时详谈.    另外, "连接"这种东西, 在系统里也是有对应的描述对象的, 毕竟连接也是一种计算机资源, 系统也要对其进行管理, 但管理是有成本的, 毕竟要有对应的内存空间需要给"连接结构体"放的, 另外这些结构体可能也有很多, 所以也需要以某种数据结构的方式对其进行统一管理, 由于cs模式下的客户端和服务端对于三次握手成功的判断时机不同, 所以就会出现一个认为握手成功, 另一个认为握手不成功的奇怪现象发生, 此时就是认知不一致了, 需要RST, RST也会对双方的这些相关的"连接结构体"进行释放, 然后在之后的重新连接里重新实例化.

下面我们粗谈一下三次握手

![image-20250415160118777](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250415160118993.png)

当客户端应用层调用`connect`时, 会给服务器发送一份SYN呗标记了的报文段, 当然, 这份报文段也描述了客户端的原始缓冲区大小和序号, 相当于在进行协商了,  

在基本场景下, 服务端应该响应客户端的一切连接建立请求, 所以它在成功接收到来自客户端的报文段后, 其实`struct sock`这种抽象描述和管理连接的核心对象已经被实例化, 服务端进入了预连接状态, 并向客户端发回了一份ACK和SYN被标记的报文段, 其中亦含有服务端`strcut sock`中对于将要建立的缓冲区初始大小(窗口大小), 和初始序列号. 

客户端收到来自服务端的响应, 保证了自客户端到服务端的可靠性, 再次发送ACK, 帮助服务端判断服务端到客户端的可靠性, 此时`connect`就正常返回, 客户端认为连接已经建立成功, 

服务端收到来自客户端的ACK, 保证了服务端到客户端的可靠性, 依据之前的`struct sock`中的相关配置, 真正实例化出进行网络通信所需要的各类资源.

需要注意的是, 中间是网络, 网络通信需要时间, 所以这里不是水平的, 斜着想表示的就是它是一个具有时间跨度的过程.

另外, 我们还需要注意到, 在三次握手的最后, 有一个没有应答的ACK, 这其实就是客户端在赌能连接成功, 默认连接成功, 所以它在发出这份ACK之后就认为连接已经建立成功了, 就真的会创建连接实体, `struct sock`是对连接的抽象描述, 但不是连接本体.

而对于服务端来说, 只有真正收到最后的ACK, 它才会认为连接建立成功, 才会创建真正的连接实体.因而客户端和服务端对于连接成功的判断时机并不相同. 

当最后一个ACK丢失后, 客户端后面就正式进入常规通信状态了, 就可能会进行正常的, 含有数据负载的报文段, 但此时客户端并没有建立成功, 此时它如果收到这份用于正常通信的报文段就会意识到, 客户端和自己对于连接成功的认知是有偏差的, 就会对服务端发送含有RST的报文段, 要求重新建立连接.服务端也会把之前的`struct sock`进行释放. 我们这里说的"认知不一致", 主要是说网络故障, 

下面我们说**URG**, 用于描述紧急指针是否有效, 需要应用层的配合.在一般情况下, 报文段中的数据都会依据序列号直接或者间接映射到接收缓冲区的某块位置上, 从而保证了数据的有序性, 而当一个报文段的URG标志位被标记后, 数据还是按照序列号有序地填充到缓冲区中, 但TCP会通过某些机制, 比如向服务端发信号, 让服务端立刻读到被紧急指针指向的字节(TCP对紧急数据的大小做了限制, 只有一字节), 从而进行立刻响应, 到达"紧急传输"的效果, 响应内容就是   服务端可能也传一个标记了URG的报文, 然后发给客户端, 客户端就也可以立刻收到响应的处理结果. 这个紧急指针实际上就是描述那一字节的"紧急数据"在负载中的偏移量. 值得注意的是, 现代操作系统有些已经舍弃了紧急指针功能, 因为其机制复杂, 可靠性低, 如果非要有紧急传输功能, 也是直接做到应用层里的, 不用TCP紧急指针机制.注意它这个"紧急"不是通过改变缓冲区字节顺序的方式实现的, 而是通过进程即时响应机制实现的, 它是逻辑上的插队, 不是物理上的插队.	

URG一般是用在什么样的情况呢? 比如, 服务端服务异常, 可能这个服务很吃资源, 所以有时候服务老是卡, 但我(服务的开发或者维护者)发现似乎不是网络原因, 我想快速确认一下服务端的状况, 到底是在吃什么资源, 就可以通过这个紧急指针, 因为它本身已经卡了, 应用层的应急传输用不了, 但紧急指针是即时响应的, 所以就可以快速与服务端建立紧急通信, 通过约定好的状态码, 大致确认当前服务的情况.  不过我们也说过, 紧急指针现代操作系统有些已经不兼容了, 应该只能在老软件上能看到, 而且, 即时是这种场景, 在应用层仍有更好的替代方案.不过我们就不说了.(实际上我也不知道, 应用软件一般没有这个紧急机制, 一般都是控制软件才有), 实在不行, 你可以在应用层专门搭个不负责实际业务的控制线程, 专门进行管理, 状态检测, 异常汇报, 模式转换什么的, 你可以模仿系统的思路, 系统不就喜欢管理吗, 实际业务是系统指挥具体计算机组件实现的.

接下里我们看一看TCP保证可靠性的另一个机制, 超时重传

其实之前我们应该谈过, 对于一份数据来说, 如果发送者在一段时间内收不到对应的ACK, 它就认为, 刚刚发的报文丢包了, 当然, 也有可能是对端收到数据了, 但它发的ACK丢了, 总之, 只要发送端收不到ACK, 它就认为对端没有收到数据, 它就会把这份报文重新发送, 如果对端真的没收到, 那对端收到就发ACK, 如果对端其实已经收到报文段了, 也可以基于序号判断出这已经收到过了, 它也会传ACK.   对于这里的"一段时间", 因为网络的状况时刻在变化, 所以这个等待的间隔时间也是在动态改变的, 但这个动态改变倒也没有多复杂, 初始时, 这段时间是"500ms", 等了"500ms", 没收到ACK, 那为了确保可靠性, 只能做最坏的打算, 就是丢了, 所以它会进行重发, 然后再等"1000ms"(500ms * 2), 还没收到ACK, 它就会再重传, 再等"1500ms"(500ms * 3), 重发......... 当重发一定次数, 发送者会认为连接异常, 就会自动断开连接.              当然, 这些细节应用层的用户不需要考虑, 也没精力考虑, 所以TCP是传输控制协议, 它控制传输的细节问题.

接下来我们看看"三次握手"的周边问题.

在三次握手成功之后, server和client就开始进行正常通信, 用户层通过调用`recv, read, write, send`等系统接口, 让数据在应用层缓冲区和传输层缓冲区进行流通. 

![image-20250416160853383](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250416160853455.png)

之后是四次挥手

![image-20250416161723593](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250416161723669.png)

接下来我们探讨一下为什么要进行三次握手.

实际上也可以说是四次握手, 因为我们看到, 中间的报文实际上是`SYN`和`ACK`合在一起的, 当然四次挥手也可以进行合并, 变成三次挥手.  至于为什么三次握手为什么书上都是把`SYN`和`ACK`合在一起是因为, 在cs模式的基础场景下, 服务端应该响应来自客户端一切连接建立请求, 所以服务器会对于连接请求直接同意, 所以`SYN`和`ACK`就合到了一起.       而对于四次握手来说, 服务端可能还有数据没有发完, 所以不一定会立刻也断开连接, 而是先把数据发完后再断开连接, 所以服务端的`FIN`并不是在收到对方的`FIN`就立刻发送, 所以`FIN`和`ACK`就不一定会合在一起.

我们将从两个角度来说明为什么进行三次握手, 

一方面, 三次握手可以至少一次确保客户端到服务端的可靠性, 以及服务端到客户端的可靠性, 保障了双向的可靠性, 才可以进行正常通信. 

另一方面, 三次握手可以减轻服务端的负担, 如果说上面的原因是从网络层面说, 这个原因就是从系统层面来进行讨论. 在之前的说明中, 我们曾模糊的说过一点, 只有系统确认连接成功的时候才会建立连接实体. 而连接实体的创建,维护和销毁是有成本的, 在cs模式下, 服务端始终是最后一个确认连接成功的, 这意味着, 如果中途连接异常, 由于服务端是cs中最后才创建连接实体的, 所以服务端就会认为连接失败, 从而不创建连接实体, 更不会销毁连接实体, 所以就减轻了服务端的压力, 要知道, 一个客户端崩了不会影响其它客户端, 但如果服务端崩了, 那将对所有客户端造成影响, 所以在进行服务设计的时候, 我们应该尽可能减小服务端的资源压力. 要把异常情况风险交给客户端承受.

比如, 我们现在假设, 现在只要一次握手, 只要服务端收到了来自客户端的`SYN`, 就认为连接成功. 我们不考虑通信方面的可靠性, 在这种场景下, 由于服务端一收到`SYN`就认为连接成功, 就会创建连接实体, 便会面临一种问题: 有时候, 服务端的连接请求会陡然增大(不管是无意还是恶意), 此时服务端就会在极短时间内创建大量的连接实体, 就很容易造成服务端连接资源的短缺, 这就像有些国家因为美债加息破产了, 它不是因为国家真没钱了而破产, 而是因为周转资金链断裂了, 没缓过来劲而破产.这种服务端短时间面临大量`SYN`请求的场景被称为"SYN洪水", 这种场景对三次握手也较有挑战.

当使用两次握手时, 即客户端发SYN, 服务端发ACK, 服务端发ACK的时候建立连接实体, 客户端收到ACK的时候建立连接实体, 这种场景, 我们可以这样来说, 假如你是服务端, 谁是客户端呢? 某些房地产商是客户端, 今天服务端发出了ACK, 于是就相当于对方做了约定, 我把资源已经建立好了, 你赶快来连吧. 就像你把钱交给了那些房地产商, 他们说, 行, 我马上就拿你的钱去建房子, 结果, 不管它是无意, 还是故意, 客户端收到ACK之后又不想连接了, 又关掉了, 但我的钱切切实实给了房地产商, 我服务端也已经建立了连接实体, 但你却不遵守约定, 就会让人很不好.   当然我们也不能这么恶意的想, 或许客户端真的不知道怎么就出问题了. 违背了我们的约定.

当握手是三次时, 正如前文所说, 服务端是最后兑现承诺的, 当连接异常时, 服务端就不会创建连接实体, 最多只是有对于连接的抽象描述, `struct sock`, 只是几个空的对象, 抽象描述相当于是设计图纸, 并没有真建房子, 所以占有的资源比连接实体就小很多.当然, 对于三次握手来说, 上面的情况, 如"SYN"洪水, 仍然会有威胁, 但我们可以在服务端前面建立一个中间层进行流量控制, 

当握手次数大于三次时, 对于偶数次, 服务端不是最后建立连接的, 对于奇数次, 和三次一样, 能够做到, 谁发起连接, 谁承担失败后果的这种效果, 那为什么不搞五次七次呢? 因为它尽管对于网络攻击有更强的适应性, 但性价比降下来了, 没有根本上避免问题, 而且更加复杂了, 有这精力, 不如在服务端前面再建一个流量控制层. 

再多说一点, 有一种网络攻击, 就是基于`SYN`洪水的, 客户端被人恶意控制, 这些人控制了很多机器, 并让这些机器在一个特定的时间点对服务端进行集中访问, 这种机器就被称为"肉机".

接下来说四次挥手, 首先我们先要说"断开连接"意味着什么, 意味着没有数据给对方发送了, 但不意味着我不读数据了,     在上图中, 客户端先发送了`FIN`, 服务端回应了`ACK`, 客户端收到`ACK`后就关闭的写端, 但此时读端并没有关闭, 客户端仍可以从读端读到服务端发来的数据, 此时连接就像变成了单向的, 可以简化客户端的运行逻辑, 因为现在已经没有写端了, 只需要守着读端就行了,    可能服务端有很多数据要发, 所以服务端这里先不关闭, 而是继续把数据发过去, 直到数据发完了, 服务端再调用`close`, 再向客户端发`	FIN`, 客户端再发`ACK`, 当服务端收到`ACK`时, 因为大家都关了写端, 所以连接才会关闭, 双方进入无连接状态.

下面我们通过实际实验来观察TCP协议通信中的相关状态. 参与实验的是两台Linux机器, 其中Centos 作服务端, Ubuntu 作客户端, 所用的代码是我们刚开始使用TCP的那份代码

这次我们的重点是传输层, 所以要把代码改一下, 客户端不进行改动, 服务端对改了两个地方, 首先是默认连接队列的大小, 由以前的`5`, 改成了`1`, 另外, 我们先不`accept`, 所以`accept_connection`最前面加了死循环, 还有为方便观察, 守护进程也去除了, 日志改回了屏幕输出模式

![image-20250417200350587](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417200350721.png)

![image-20250417200417196](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417200417357.png)

![image-20250417201101539](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417201101710.png)

先只启动服务端可以看到, 监听套接字已经进入了监听状态

![image-20250417211821654](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417211821786.png)

启动客户端我们就能看到相应的网络状态

![image-20250417212059289](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417212059478.png)

我们以客户端随机绑定的端口为准, 云服务器的IP地址是经过云服务器厂商虚拟化的, 所以IP不一定能对得上, 我们看到, 服务端和客户端的状态都是`ESTABLISHED`, 也就是两端都认为连接建立成功的, 尽管应用层还没有调用`accept`, 所以我们要得出的重要结论是

连接建立成功和上层有没有完成`accept`没有关系, 三次握手是双方操作系统自动完成的. 

现在我们再启动两个客户端

![image-20250417212216445](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417212216554.png)

![image-20250417212340717](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417212340831.png)

我们看到, 这次服务端和客户端就连接是否成功产生了分歧, 这是偶然现象, 服务端没有收到最后的那次`ACK`吗?

![image-20250415160118777](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250415160118993.png)

现在我们把第二个客户端关闭

![image-20250417212931497](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417212931632.png)

挥手画的缺些状态, 看这张的挥手状态

![image-20250417213029155](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417213029249.png)

由于客户端退出, 所以连接亦被关闭, 向服务端发出了`FIN`. 服务端也做出了回应, 不过由于服务端我们人为地卡住了, 所以它没有调`close`, 所以`51364`在客户端没有进入`TIME_WAIT`, 或者是直接关闭, 服务端则是`CLOSE_WAIT`, 另外我们还可以看到, 之前`SYN_RECV`的那个连接已经不存在了. 

接下来我们要说的是为什么第三次连接建立失败了, 这和`listen`的第二个参数, 也就是之前我们改的宏`DEFAULT_BACKLOG`有关. 之前我们把它改成了`1`, 这意味着, 在应用层没有把连接映射到套接字文件之前, 服务端只能存`1 + 1`个连接.

我们看到`accept`返回的是一个文件描述符, 它的实际作用就是让某个连接与一个内存级的套接字文件建立映射关系, 从而通过把连接转换成内存文件的这种方式 , 把连接的维护转变为对文件的维护, 但如果应用层一直不调用`accept`, 那传输层就需要将这些已经建立成功的连接保存下来, 它把这些连接存到哪里呢? TCP协议会在传输层建立一个全连接队列, 把那些已经建立好的连接放到这个队列中, 此时这个队列中就相当于一份临界资源, TCP协议是生产者, 应用层是消费者, `accept`的作用就是把连接从这个全连接队列中拿出来, 与某个文件进行关联, 这样连接就以文件的形式进行维护, 就不需要TCP在传输层进行维护了.

![image-20250417215342751](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250417215342914.png)

当临界空间满了之后, 即使客户端收到了来自它的, 三次握手的`ACK`, 也会因为全连接队列满了, 而丢弃这个`ACK`, 因此在服务端眼里, 这个连接仍旧是`SYN_RCVD`的.(`SYN_RCVD` 是 TCP 协议标准中的术语, `SYN_RECV` 是 Linux 内核中对这个状态的代码命名方式, 它们是同一个东西)

我们接下来再试一次

![image-20250418125054460](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418125054580.png)

![image-20250418125201361](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418125201467.png)

稍过一会儿, 我们就可以发现,之前的`45996`在服务端已经消失了, 被释放了.

![image-20250418125514483](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418125514596.png)

像`45996`这种, 处于`SYN_RECV`状态的连接被称为"半连接", 与全连接类似, TCP协议也会对半连接进行暂时的存储, 称为"半连接队列", 当一个半连接超过一段时间之后还没有变成全连接, 还是半连接, 那么它就会被系统释放, 所以服务端就没有它了. "半连接队列"的长度和`DEFAULT_BACKLOG`略有关系, 但不多, 主要还是系统自己决定. 

并且我们还可以看到, 现在服务端已经没有这个连接了, 而客户端还认为连接存在, 此时客户端向服务端发送数据, 服务端就会发送`RST`双方进行重连, (也有可能是我们应用层的重连机制起效了, 而不是TCP协议层在重连, 这里不深究了, 深究也很简单, 把客户端改简单点, 不要在应用层启用重连机制)不过因为完全连接队列仍旧是满的, 所以还是无法建立成功, 服务端依旧会显示`SYN_RECV`

![image-20250418133428335](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418133428455.png)

在上面的过程中, 我们意识到, 一个连接先会进入半连接队列, 然后再进入全连接队列, 最后才是到达应用层, 这里就有一种分级管理的感觉, 另外我们需要知道的是, 如果应用层因为资源吃紧, 迟迟不把全连接拿上去, 而又有恶意连接占着全连接队列或者半连接队列中的位置, 那其它连接就连半连接队列都进不去, 从而造成丢失, 这个就是TCP三次握手的`SYN`洪水场景. 

下面我们说一个面试题: 为什么`listen`的第二个参数不能太长, 也不能没有

不能太长其实很好解释, 本来应用层就有可能资源吃紧而不能把连接从全连接队列里拿上去, 结果在资源已经吃紧的情况下, 还维持一个很长的全连接队列, 连接还一直放在队列里面, 就会造成资源的进一步浪费, 从而加重资源压力, 而且由于这些连接没有拿到应用层, 提供不了服务, 还空占着位置, 所以连接队列不能太长

但也不能太短, 你不能说没有, 这个队列就相当于某种缓冲区, 当应用层提供了对应的服务, 把连接关闭后, 就能立刻从全连接队列里再获取多个连接, 如果没有全连接队列, 应用层突然闲下来时就必须等着客户端来连, 也就是说它有个间隙不能为外界提供服务, 不能提供服务就不能创造价值, 所以该有还是要有的.或者我们拿生活中的一个现象举例, 有些饭馆可能很火, 现在后厨很忙, 暂时提供不了服务, 那可以先把客户安排到桌子上, 让用户先等一会, 而不是直接告诉客户, 换一家馆子.

至于具体要设置成多少, 那要看机器的具体性能, 对我用的这个入门型云服务器来, 设置成`12, 16`, 这个级别的数字就完全够了.

前面我们实验的是三次握手, 下面我们试验一下四次挥手, 首先我们还是要再改一下代码, 把连接拿上去. 也就是取消`accept_connection`的`while(1)`, 并且为了看一下`CLOSE_WAIT`状态, 我们不关闭`class task`里面的`sockfd`

![image-20250418144113128](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418144113676.png)

我们先正常连上

![image-20250418144803981](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418144804088.png)

然后把客户端断掉, 由于我们手动阻止了服务端的`close`, 所以服务端还没有向客户端发送`FIN`, 所以四次握手没有完整进行, 主动断开的那一方是`FIN_WAIT2`, 被动断开的那一方是`CLOSE_WAIT`

![image-20250418144846156](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418144846262.png)

再过一会儿我们发现主动断开的那一方连接已经消失了, 而被动断开的那一方连接仍旧存在, 也就是说被动断开的那一方会把连接存更长的时间

![image-20250418145328114](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418145328227.png)

接下来我们主要说说`FIN_WAIT`这个状态, 也就是主动断开的那一方, 在收到对面`FIN`之后, 所进入的状态, 

这次我们让服务端做主动断开的那一方, 为了让服务端主动关闭, 我们把服务内容全部注释, 就留下来一个`close`

![image-20250418152028580](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418152029134.png)

在服务端主动关闭之后, 客户端发回了`ACK`, 服务端的连接就变成了`FIN_WAIT2`

![image-20250418153520568](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418153520680.png)

接下来我们关闭客户端, 由于客户端被关闭了, 相当于文件被关闭了, 所以客户端这边也会发送`FIN`, 服务端收到之后, 发回`ACK` 状态变为`FIN_WAIT`

![image-20250418153545725](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418153545837.png)

并且在一二十秒之后, 再次查询, 仍能查到`8444`这个连接, 也就是说, 这个状态下的连接还会保存一段时间, 

总结一下上面的现象:   主动关闭连接的一方, 在完成四次握手之后就会变成`FIN_WAIT`状态, 并保持一段时间. 

这个`FIN_WAIT`和端口地址复用有关, 为此, 我们先把代码中地址端口复用设置注释掉.

![image-20250418155152236](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418155152810.png)

在这之后, 我们再来一次

![image-20250418155322878](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418155322981.png)

![image-20250418155343380](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418155343492.png)

此时我们把服务端终止, 再启动就会报错

![image-20250418155819688](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418155819806.png)

在服务端关闭之后, 尽管应用层已经没了, 但在其之前绑定的端口上仍旧还有连接, 此时重新启动服务端, 由于用的是相同的端口, 所以这个新启动的服务进程就会和之前未被完全关闭的连接冲突, 从而绑定失败.  尽管它是是地址在被使用, 但实际上就是纯粹的端口原因, 和IP没有关系.

在面临大量流量时, 服务端的崩溃是一种常见现象, 比如双十一京东它们的服务端, 此时就需要快速地将服务端重启, 如果端口地址没有设置成复用, 那就要等一段时间才能重启, 要知道, 现在是大量流量, 这不能重启的一段时间内, 将会造成很大的损失. 所以对于服务端来说, 一定要设置成端口地址复用.

这个一段时间平台不同各有不同, 大概是三十到六十秒.

```c
int opt = 1;
// 防止服务端异常退出后出现不能立即重启的情况    复用地址  和    端口      讲原理的时候再细说
setsockopt(_listeningSockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
```

系统新的话可以只有`SO_REUSEADDR`这一个选项, 默认地址端口都复用.

下面我们细说一下`TIME_WAIT`状态

TCP协议规定,主动关闭连接的一方要处于`TIME_WAIT`状态,等待两个`MSL`(最大报文段存在时间)的时间后才能回到`CLOSED`状态 , 也就是完全关闭.   `MSL`就是一份报文段从一段到另一端的最长传送时间,         为什么会有这种机制呢?   这涉及到很多原因,   比如, 主动断开的那一方是在向对方发了`ACK`之后才进入`TIME_WAIT`的, 但这个`ACK`有可能丢包, 此时另一端就会再发`FIN`, 本端为了能够响应重发的这份`FIN`, 那就需要还存在一段时间     

![image-20250418163718978](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418163719093.png)

不过即使网络一直不好, `ACK`一直丢包, 被动端也会认为连接异常自动关闭, 但这种关闭不按照正常情况来, 我们能通过让主动端等一会儿的方式正常关闭当然就用正常关闭.

另外一个原因是回收历史报文, 在双方网络通信过程中, 可能会存在一些报文, 这些报文长时间在网络中滞留, 时间长到可能发送方已经认为传送失败而重传多次的那种程度, 毕竟我们说等待时间都是以秒为单位的, 正常通信肯定不会以秒为单位, 这又不是地月通信, 地月之间的距离大概是1.2光秒, 这些历史残留报文可能本身已经没有用了, 因为接收方可能早已收到了它的重传版本, 但毕竟是打算给接受者的东西, 接受者既然可以收到, 那就应该收到, 即使收到也没有用, 可能收到的行为就是直接丢弃, 但如果能收到还是要收到的, 为什么非要纠结没有用也要回收呢? 这主要是怕这些历史数据影响到下一次新的相同IP和端口的连接, 由于端口IP相同, 所以这些历史数据会干扰到新连接的通信, 尽管干扰程度并不大, 因为每次连接使用的初始序号是随机的, 所以新连接可以依据序号判断出这不是新连接的报文, 从而也将其舍弃.

这里要特别注意一下, 超时重传的那个最大传送时长(毫秒级)和最大存在时长(秒级)不是一个东西, 不要弄混了.   

我们可以使用`cat /proc/sys/net/ipv4/tcp_fin_timeout  `查看系统中`MSL`的长度

```shell
[wind@starry-sky TCP]$ cat /proc/sys/net/ipv4/tcp_fin_timeout
60
[wind@starry-sky TCP]$

[whisper@starry-sky TCP]$ cat /proc/sys/net/ipv4/tcp_fin_timeout
60
[whisper@starry-sky TCP]$ 
```

想改动直接改配置文件就行了.                        TCP协议协议规定`MSL`是`120`秒, 但具体还要看系统

下面我们稍微说一下流量重传, 接下来就去看TCP的滑动窗口了

![image-20250418175426750](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418175426909.png)

这个其实我们曾经也说过, 这里稍微总结一番. 在两台机器最开始握手的时候, 双方会交换彼此的报头, 此时双方其实就已经获知了对方的窗口大小, 甚至在握手阶段, 还可以通过选项协商缓冲区的单位, 默认情况下, 窗口大小是16位, 那就是65535字节(约64KB), 但可以通过选项把把它变大, 选项中有一个名为"窗口扩大因子"(记为`M`)的八位数字, 也就是范围是`0`到`14`, 实际的窗口大小等于窗口大小的字段值再乘上$2^M$ 所以窗口的最大大小是$65535 * 2^{14} = 1073725440字节$, 约为1GB.

在三次握手的最后一个`ACK`, 其实已经可以进行捎带应答了, 可以顺便携带上应用层的数据, 

当有一端窗口真的满了, 那另一端就不能再发有数据的报文了, 此时, 另一端有两种方法获知对方有没有把窗口清理出来, 一是它自己每过一段时间发一份没有负载的报文, 另一端就会对这个报文进行应答, 其中就包含着当前窗口的剩余大小, 二是, 当窗口有剩余空间时, `B`也会立刻向`A`发一份报文, 从而让对方获知当前剩余的窗口大小, 不过第二种方法只做一次, 如果丢失了就丢失了, 所以就需要第一种方法不断询问, 防止不知道窗口的更新. 

流量控制既有可靠性的考虑(你不能发一个数据我放不下的报文段), 也有效率考虑(我放不下, 那报文传过来就没有意义, 反而浪费了网络资源)

------------------

下面我们说一个大话题, 那就是TCP对于自己缓冲区的管理方式, 称之为"滑动窗口"

我们曾经了解过TCP最基本的通信方式: 那就是双方使用串行通信模式, 发送方不能连续地发送报文, 必须先发一份, 然后在收到`ACK`应答之后, 再发第二份.

![image-20250418184833339](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418184833452.png)

但很明显, 这样的通信方式效率是很低的, 为此, 我们就需要并行的发送模式

![image-20250418185209597](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418185209680.png)

但若要实现这种并行发送模式, 就需要对发送和接收缓冲区进行管理, 从而支持这种一次发送多份报文的能力.

对于发送缓冲区来说, 由于在收到`ACK`之前都不知道对方是否收到报文, 所以就需要把那些没有收到应答的报文中的数据暂时保存起来,并且由于它是并行发的, 所以就会有多份这样的数据.

我们可以先简单地把发送缓冲区分为四个部分, 已发送已确认(可被覆写), 已发送未确认, 待发送, 其它

![image-20250418191204643](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418191204713.png)

已发送已确认, 那就意味着对方收到了其中的数据, 对于这片区域来说, 就可以直接被用户新传下来的数据覆写, 或者说, 这个区域中的数据实际上已经被视为无效的, 删除了, 移除的. 

已发送带待确认, 可以细分成两种数据, 一种是可以发的, 另一种是已经发的, 但没有收到应答的区域, 对于这些区域来说, 其中的数据就相当于被保护着, 当其中那些已经发的数据收到了对方的应答, 它们就会被纳入"已发送已确认"的范畴. 这个区域就是我们所说的"滑动窗口"

![image-20250418192041956](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418192042025.png)

滑动窗口是可以直接向对方发的数据, 所以它的大小, 实际上就是对方接收窗口的大小.

滑动窗口的划分是通过下标来进行说明的, 我们知道缓冲区可以视为一个字符数组, 所以里面的每个字节都有其下标, 所以我们就可以用两个下标(指针)来描述滑动窗口的范围.

![image-20250418192800660](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250418192800733.png)

将来如果对方的接受能力变大了, 那就可以把`win_end`往后移, 这也是一种滑动.

另外我们还需要了解的是, 网卡不能一次性发太大的数据, 所以即使滑动窗口的大小处于对方窗口的可容纳范围之内, 也不能一块打包做成一个报文发过去, 而是要拆成一份一份的数据, 用一次发送多份报文的方式把数据给对方发过去.

上面的概念有些太杂了, 在这里我们再次叙述一下. 

为了有效的解决TCP网络串行通信效率低下的问题. 

![image-20250419151654724](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419151654846.png)

我们引入了并行发送这种方式

![image-20250419151733251](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419151733350.png)

我们知道, 应用层的报文实际上被存放在TCP的发送缓冲区中, 我们通过指针对发送缓冲区进行区域划分, 可以将其分为,

![image-20250419152032237](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419152032308.png)

已经收到应答的, 可被覆写的区域(图中的1001下标之前), 可以直接立刻发送或者说已经发送过但没有收到应答的滑动窗口(1001到5001), 还不能直接发送数据的区域(5001到某个位置), 没有用户数据的"空"区域(接着"某个位置"到末尾)

当`1001`到`2001`这段区域收到了应答, 那滑动窗口的起始指针就会向右移动到`2001`, 这样`1001`到`2001`就相当于变成了可被覆写的区域.  从理论上说, 滑动窗口越大, 就意味着发送端可以一次发送更多份的报文(链路层不支持一次发送一份很大的报文, 所以我们一次只能把滑动窗口中的数据分成多份, 一份一份发), 

滑动窗口表示可以直接向对端发送的数据范围, 直接是因为滑动窗口会通过对端报文中的窗口大小, 判断出对端的最大数据负载能力, 依据对方的实际能力来决定滑动窗口的大小, 因此, 滑动窗口的所有数据, 在一次拆成多份发给对端, 假设对端能够收到所有报文, 那这些报文中的数据加在一起, 也不会超过对端接收缓冲区的剩余空间, 所以我们说, 可以"直接"发送.     就目前而言, 我们可以认为, 滑动窗口的大小就是对端的剩余窗口大小.

下面我们探讨第一个话题:   滑动窗口是怎么应对`ACK`丢包的

![image-20250419161744872](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419161744957.png)

![image-20250419161825724](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419161825824.png)

当对方实际接收到全部报文, 但对应的`ACK`应答丢包了, TCP会作何反应?

其实很简单, 在上图中, 尽管`A`未收到确认序号未`6001`的`ACK`, 但它收到了确认序号`8001`的`ACK`, 我们之前说过, 确认序号如果是`X`, 那就表示`X`前面的序号对端都收到了, 所以对于这种情况, 滑动窗口的起始指针, 就是直接移到收到的最后一个`ACK`的确认序号, 即`8001`的位置.(先不考虑滑动窗口的末尾指针移动)

![image-20250419164130857](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419164130938.png)

如果是`8001`的这个`ACK`丢了, 那没办法, `A`只能认为`B`收到了`7001`前面的数据, 此时对于`7001~8000`这段数据可能就会使用超时重传了.

![image-20250419162929652](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419162929742.png)

![image-20250419164244800](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419164244912.png)

接下来我们看看真的数据丢了会怎么样

![image-20250419163457366](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419163457457.png)

在这里, `5001~6000`的数据丢了, 此时由于确认序号取得是最大的, 所以后面的`ACK`, 确认序号都是`5001`

对于这种情况, 滑动窗口起始指针就会移到`5001`

![image-20250419164418139](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419164418207.png)

对于这种出现多个(三个及其以上)相同确认序号的`ACK`, `A`就会进入启动"快重传", `A`可以很明显地感知到, 一定是`5001~6000`这段数据出问题了, 此时它就不会再等超过最大传送时间再进行重传, 而是立刻先把`5001~6000`这段数据重新发送, 对于这份新发的数据来说, 由于`B`已经有了`6001~7000, 7001~8000`这两份数据, 所以会直接回应确认序号`8001`

![image-20250419165652613](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419165652711.png)

这还有一份更老的图

![image-20250419165812385](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419165812503.png)

"快重传"有前置条件, 那就是收到3个及其以上同样的确认应答, 它可以提升效率, 但如果报文数据都不够三个, 此时就只能靠"超时重传"了, 能用"快重传"就用"快重传", 实在用不了, 那也有"超时重传" 

之前我们都假定滑动窗口的末尾指针不动, 其实对应的就是`B`应用层一直不读数据, 接下来我们要考虑一下对端数据的接受能力.

之前我们说, 现在可以认为滑动窗口的大小就是对端接收缓冲区的剩余空间大小, 所以在最开始的时候, 滑动窗口的大小就是对端剩余空间的大小, 而对端应用层又不读数据, 所以每发送成功一份报文, 对端剩余空间就小一份, 最终当滑动窗口大小归零时, 其实就意味着对端的剩余空间归零了. (起始指针向右移, 末尾指针不动)

如果对端应用层读数据很快, 那么, `A`就可以通过`ACK`中的窗口大小来获知对端窗口变大了, 所以本端的窗口也会变大, (起始指针和末尾指针都想后移动), 当然也存在这种可能, 那就是`A`发四份报文, `B`读出两份报文, 对于这种情况, 起始末尾指针还是都向右移, 但起始指针的偏移量将会比末尾指针大, 所以滑动窗口还是缩小了, 或者发四份, 读四份, 此时还是都向右移, 且偏移量一致.

下面我们给个简单的计算公式来简单概括两个指针的寻址方式

```c
int start = 根据确认序号和缓冲区下标的映射关系, 得出, 不考虑映射关系的话, 就是确认序号;
int end = start + 对端报头中的窗口大小;
```

不过还要注意一点, 那个`end`不能溢出, 比如对于下面这张图来说, 发出这三份数据, 也受到了对应的`ACK`, 所以起始指针变成`8001`, 但`ACK`中的窗口显示是四单位的大小, 那`end`不能移到`12001`, 而只能移到`11001`

![image-20250419164418139](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419164418207.png)

之前我们曾经说过"流量控制", 我们需要知道的是, "流量控制"正是由"滑动窗口"实现的, 流量控制是窗口大小的外在表现, 当对方容纳能力很大时, 就可以一次性发送很多份报文, 当对方快满了, 那就少发一点. 

还有一点, 那就是缓冲区在逻辑上实际上是成环的, 指针或者说下标的计算都是会取模的, 所以在逻辑上缓冲区是首尾相连的, 因此最开始, 在涂色的时候, 我们把"空"(真的没有用户层数据), 和"已经收到应答可以删除的"这两个部分都画成了一个颜色, 并统一称呼为"可以被自由覆写". `13001`其实就是`1`.

最后我们要说的是, `statr`的那个计算公式, 我们说什么"映射关系", 这是因为序号和缓冲区的下标其实不是一个概念, 在TCP最开始三次握手的时候, 出于防范`TIME_WAIT`那种的历史数据, 两端随机生成一个初始序号, 比如`A`生成的是`1234`, `B`是`4321`, 那么选择其中最小的那个序号, 这样真正使用的初始序号就是`1234`,    然后缓冲区的真正数组下标其实应该等于序号再减去`1234`, 而在上面的图中, 为了方便, 我们认为初始序号就是`0`, 所以直接对应, 但实际上, 它们是相对映射的

------------------

下面我们说延迟应答和阻塞控制, 主要说阻塞控制

在上面关于TCP滑动窗口的讨论, 我们曾说过这样一句话: " 那就是`A`发四份报文, `B`读出两份报文, 对于这种情况, 起始末尾指针还是都向右移, 但起始指针的偏移量将会比末尾指针大", 这里面其实就暗含着TCP的延迟应答机制.

我们知道, 发送方一次发送更多的数据, 并发的程度就更高, 通信的效率就更高, 但发送方的数据发送能力是被滑动窗口控制的, 或者这样说, 接收方的接受能力决定了发送方滑动窗口的大小, 进而决定了发送方的数据发送能力, 所以`B`主机在给`A`发回`ACK`的时候, 如果可以的话, 应给给`A`发一个更大的窗口, 所以当`B`收到数据时, 它不会马上发`ACK`, 而是先等一会儿, 当然这一会儿不至于触发对面的超时重传, 为什么要等呢? 就是希望应用层在这段时间之内把接收缓冲区中的数据读上去, 这样窗口就变大了, 此时`ACK`上的窗口就可以填的更大.    当然, 如果这段时间内应用层不取数据, 那就没办法, 窗口并没有变大.那还是填之前的大小. 对于上面的这段话, 那说的就是在延迟应答这段时间内应用层又读上去了两份报文, 所以本来窗口可能并没有这么大, 但由于应用层读上了数据, 所以窗口大小就可以再加上这两份的大小.

延迟应答对于应用层的启示就是, 要尽快地把数据读上来, 从而更新出更大的窗口.

延迟应答有两方面的限制
首先是在数量限制, 每隔N(2)个就要应答一次, 避免对端迟迟不能更新滑动窗口, 什么意思呢? 就是对端可能正准备一次发送多个, 现在已经发了两个了, 马上就要发更多个, 此时就不要延迟了, 立马回一个`ACK`, 让对端立刻更新滑动窗口大小, 防止它因为没有更新滑动窗口卡住, 而发不出本来应该能发出的后续报文.
然后是时间限制, 超过一定时间(200ms), 就要立刻发一个`ACK`, 防止触发对方的超时重传.

所以延迟应答虽然主要是为了提高效率, 但如果使用时机不对, 反而会拖垮效率.

下面我们略微小节一下

![image-20250419185743236](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419185743347.png)

校验和没什么好说的, 那是数学的事, 序列号既有按序到达的功能, 也有去重的功能, 确认应答在TCP中无处不在, 支持着可靠性和控制信息的传递, 超时重传没什么好讲的, 连接管理就是握手挥手. 流量控制防止发出对方放不下的报文, 效率方面, 那就有滑动窗口, 快速重传, 捎带应答, 延迟应答, 

上面的这些东西都是作用在通信双方主机上的, 下面我们要说的"拥塞控制", 作用的可不是通信双方的主机, 而是所有用TCP的主机, 或者说网络本身.

我们说拥塞控制控制的是网络, TCP双方通信, 无非出两方面的问题, 一是通信双方主机的问题, 二是网络的问题, 那既然拥塞控制面向的是网络, 那该如何确认出问题的是网络, 从而启动阻塞控制的呢?

这个其实也很通俗易懂, 如果我发的报文, 只有一小半, 比如两成左右的是对方收到的, 而且, 每次发, 都是这样, 那就说明, 网络本身出问题了, 或者这样说, 大学的期末考试, 百分之二的挂科率和百分之九十八的挂科率不是同一个概念, 如果百分之九十八都挂了, 那就应该是学校的问题. TCP也是这样, 如果通信的时候发生了少量的丢包, 那可能是通信双方主机的偶发事件, 如果发生了大量丢包, 那就可能是网络本身有问题.

网络问题又可以继续划分, 比如硬件出问题了, 那这没办法, 只能靠硬件工程师去修, 又或者网络中的数据太多了, 引发了网络阻塞, 这个说不清楚, 既可以是软件问题,, 也可以是硬件问题.

如果通信双方出现了大量的数据丢包问题(可能根本收不到应答), TCP就会认为网络出现了问题(称之为"网络阻塞"), 从而启用阻塞控制.

对于我们软件来说, 当出现网络阻塞时, 要做的就是降低数据的发送量, 不要加重阻塞的程度. 因此对于这种情况, 由于大量报文都丢包了, 所以理论上应该进行大量的超时重传, 但实际上, 在阻塞控制的作用下, TCP不会再把这些丢包的数据一次性又大量重发.因为这样会让网络中的数据量又增多, 从而加重阻塞程度, 进而形成恶性循环.

我们说, TCP是协议, 大家都用TCP协议, 都遵循对应的规则, 如果一个TCP在报文大量丢包的情况下会大量重发, 那么, 所有的TCP都会这样, 尽管一个TCP对于网络的影响微乎其微, 但所有TCP都这样干, 影响就会非常大了.

所以对于数据大量丢包的情况, 每个TCP都应该少发一点, 大家都应该少发一点, 这样累加起来, 就会少发很多数据. 并且这样会形成一个宏观效果:   网络越阻塞, 收到波及的机器就越多, 启动阻塞控制的TCP就越多, 网络中的数据就会越少., 只有个别主机认为网络阻塞, 那就只有个别主机进行阻塞控制, 其它主机仍然进行正常通信.

上面说的是不该做什么, 下面我们说该做什么, 也就是拥塞控制的具体操作.

TCP引入"慢启动"机制, 先发少量的数据, 探探路, 摸清当前的网络拥堵状态, 失败就重传这一个, 成功就再多发一点.

![image-20250419195428373](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419195428497.png)

如果一直顺利的话, 就把数据量呈指数性的增长.   

对于阻塞控制来说, 会引入一个概念, 名为"阻塞窗口", 阻塞窗口最开始被设定为一(1个单位的最大理论负载,  链路层每次只能发送一定长度的数据帧, 大小为`1500 `字节, 除去必须要的IP报头(20字节)和TCP报头(20字节), 还剩下`1460`字节, 此处的一表示`1460`字节), 每次收到一个ACK应答, 拥塞窗口加1, 以上面的图举例, (上图为了好画, 它的一用的是1000字节), 第一次它只发一个报文, 在收到ACK之后, 阻塞窗口就变成了2,  而在第二次, 它发送了两份报文, 收到两份ACK, 阻塞窗口变成`2+2=4`, 第三次, 发送四份报文, 全部收到`ACK`, 那阻塞窗口就变成了`4+4=8`, 所以, 对于每次来说, 阻塞窗口是指数性增加的

每次发送数据包的时候, 将拥塞窗口和接收端主机反馈的窗口大小做比较, 取较小的值作为实际发送的窗口, 此处涉及到了很多窗口, 滑动窗口, 反馈窗口, 拥塞窗口, 实际用的是滑动窗口, 我们之前说的滑动窗口实际上是在不考虑阻塞窗口, 仅考虑反馈窗口(描述对方主机的接受能力),   阻塞窗口描述的是网络承载能力.

想要大量发送数据, 有两个必要条件, 一是对端能放的下(反馈窗口), 二是网络承受得起(拥塞窗口), 所以实际的滑动窗口大小使用的是它俩的最小值.至此有

```c
int end = (start + min(对端承载能力, 网络承载能力)) % mod(缓冲区大小, 环形逻辑)
```

上面说, 拥塞窗口是指数性增加的, 诚然, 最开始基数比较小, 所以它增长的比较慢, 因此称之为"慢启动", 但随着基数不断增大, 它就会增长地越来越快,   但很明显, 拥塞窗口描述的是网络承载能力, 超过它的大小, 就很有可能引发网络阻塞, 但很明显, 网络资源即使好, 即使很健康, 也是有极限的, 所以阻塞窗口不会一直指数性的增加, 这样不符合它的实际意义, 而是到达某个值(称之为"慢启动阈值"), 就会变成线性增长, 去试探网络的最大承受能力(网络是动态变化的, 所以要试一试), 试探到了(到了这个值网络阻塞), 那就会对阈值进行下调, 并且重新进行慢启动, 这样它就会越来越难以接近到阻塞节点(到达这个点网络就阻塞),  这仅仅是从拥塞窗口的角度来考虑的, 实际上, 滑动窗口取的是反馈和拥塞最小值, 所以有可能滑动窗口一直达不到阻塞节点.

![image-20250419204615823](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250419204615933.png)

慢启动阈值初始时各有不同, 但下调都是刚刚网络拥塞的一半. ("一半"是基于统计学的角度得来的, 在工程实践中, 发现这种调法效果好, 当然也有理论依据)

少量的丢包, 我们仅仅是触发超时重传; 大量的丢包, 我们就认为网络拥塞 .    当TCP通信开始后, 网络吞吐量会逐渐上升; 随着网络发生拥堵, 吞吐量会立刻下降拥塞控制, 归根结底是TCP协议想尽可能快的把数据传输给对方, 但是又要避免给网络造成太大压力的折中方案

## 面向字节流

首先我们先说一下面向字节流的重要特征, 那就是从缓冲区里读数据的时候, 不需要在意它是怎么被写进缓冲区的, 或者说写数据的时候, 也不用在意它会怎么被读进去. 比如说, 

- 写100个字节数据时, 可以调用一次write写100个字节, 也可以调用100次write, 每次写一个字节;  
- 读100个字节数据时, 也完全不需要考虑写的时候是怎么写的, 既可以一次read 100个字节, 也可以一次read一个字节, 重复100次;  

TCP是面向字节流的, UDP是面向数据报的, 为此我们可以拿UDP来比较一下, 我们知道UDP协议中有一个长度字段, 它描述了整个UDP报文段的大小, 而又由于UDP报头的长度是确定的,  所以就可以得到确定的数据大小

![image-20250411163844645](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250411163844778.png)

对于UDP来说, 要么就收到一个整的报文, 要么就收不到报文, 不存在中间状态.

对于面向字节流该如何理解, 首先它确实是一种"流", 比如应用层把自己的网络报文交给传输层,(主要是说TCP), 在TCP眼里, 没有什么报文不报文的概念, 就只是很多字节的数据, 它会把字节数据包到自己的数据段中, 交给IP, IP在通过一些中间结构递给对方的传输层, 对端传输层也只认为自己收到了一堆字节数据, 然后对端应用层会读数据, 这样数据就有了流向, 所以是流.

因为我们的传输层它只知道字节数据, 所以对于这些字节数据该如何区分, 那就要靠应用层的协议了, 比如, 在我们的自定义协议中, 我们就使用了特殊字符`'\n'`来进行报文分割, 报文里面也有对应的分割符, 有专门的接口可以查看收到的字节数据能不能构成一个完整报文, 如果可以构成, 那就把它从应用层缓冲区给截出去, 进行专门的解析, 还有HTTP的空行, 用来分割报头和数据. 

当表示层根据报头截出一个长度足够的字节数据之后, 

![image-20250421191712869](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421191713587.png)

就会交给应用层, 由我们的`init`进行解析

![image-20250421191144672](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421191145563.png)

TCP是传输控制协议, 它可能会把用户层数据拆开来发, 所以不需要知道数据的大小, 知道也是没有意义的. 等会儿我们说连接和文件的关系时, 还会深入来说. 

我们之前说, 什么把数据交给`send`, 就发出去了, 其实这是一种错误的说法, `send`只是把应用层报文移交给系统, 系统会以自己的方式, 把这些数据一层一层地加报头, 最终发到网络中, 至于这个发送的细节, 用户不用操心, 都由系统决定.

## 粘包问题  

就像我们所说的那样, TCP眼里只有字节数据, 没有报文概念, 这就使得多个报文之间是连起来的, 但很明显, 对于应用层来说, 我们只能把报文一个一个地处理, 所以我们要把连在一起的报文给截出来一个完整的报文, 以进行下一步, 如反序列化之类的处理. 关于粘包, 我们可以拿一些日常场景来理解, 比如, 过年时, 吃包子, 但包子连在一起, 你去吃, 就会把"整张"包子拿起来, 那就不好操作, 是吧.   解决粘包问题 就需要应用层设计相应的协议, 比如, 在我们的自定义协议中, 我们在应用层报文上加上了数据长度这种自描述字段, 对其进行解析时, 就可以依据这个长度取出足够长度的字节数据. (注: 不够一个报文也属于粘包问题)

为了把连在一起的报文给解出一个独立的报文, 我们有三种具体做法. 

- 定长报文, 我的报文长度是确定的, 对端只需要依据这个确定长度一个个读就行
- 使用特殊字符, 比如HTTP的那种空行, 我们的自定义协议中也有对应的思想
- 使用自描述字段, 比如还是HTTP和我们的自定义协议, 它们的报头中都含有负载数据的长度, 我们可以通过定长的报头, 比如模仿TCP报头那样, 先把报头拿出来, 从而获知自描述字段, 或者通过特殊字符, 如空行, 把报头拿出来, 查看其中的自描述字段. 也就是说, 这三种混着用.

## TCP异常情况

- 进程终止
  对于系统来说, 它没有什么异常终止, 正常终止的概念, 所以不管是哪种形式的终止, 对于系统来说, 只意味着要回收进程资源而已, 我们之前说过, 连接是以文件的形式供应用层使用的, 如果一个文件已经没人用了, 系统就会把它关闭, 这种关闭和我们调用`close`类似, 所以当进程终止后, 系统就会关闭连接文件, 文件关闭了, 就会启动四次挥手流程, 正常的断开TCP连接
- 机器关机/重启
  对于正常的关机重启, 系统会自动关闭系统内的进程, 所以这就回到了进程终止的情况, TCP还是正常断开连接
- 机器断电/网线断开
  那这自然是无法进行正常的连接断开的, 机器断电系统都没了, 怎么挥手? 网线断开, 物理层没了, 那怎么能通信, 对于这种情况, 双方会意识到连接异常, 自动关闭, 如果网线接上去的间隔时间够短, 那可能会触发TCP的RST, 进行重连, 另外, TCP有保活机制, 如果服务端发现客户端较长时间不发数据, 服务端就会发报文确认情况, 收不到应答就认为连接异常, 于是关闭

## 一道面试题

问: 如何实现UDP可靠传输

首先确认一下使用场景, 能不能直接改成TCP, 不能就依据使用场景, 你不必把TCP的可靠性措施全部搬到应用层, 那样可能开销有些大, 毕竟他坚持使用UDP, 所以可以有选择的套用TCP可靠性措施

## 连接与文件

我们知道, `struct task_struct`中定义了一个`struct files_struct *`的指针, 该指针指向的对象中会含有一个`struct file*`的指针数组, 而`struct file`其实就是我们所说的文件描述附表, `struct file`里面有一个操作方法集指针`const struct file_operations  *`, 这个操作方法集里面就有一堆函数指针, 为应用层提供对应的操作方法, 实际上就是用指针实现多态, 把更底层的操作方法都封装成文件

![image-20250421212650732](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421212651437.png)

`struct file`里面还有一个`struct address_space  *`, 这就是指向文件缓冲区的指针.

![image-20250421214148089](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421214148245.png)

然后我们再看看连接那边

连接的数据结构是`struct socket`

![image-20250421214317428](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421214317674.png)

然后我们还可以看到, `struct file`里面有一个`void* private_data*`, 这个指针指的就是被文件虚拟化的那些更底层的结构, 比如`struct socket`, 而`struct socket`也有`struct file*`指回去, 所以它们就可以双向寻找对方

然后我们再看看`struct socket`里面的别的东西, 比如`wait_queue_head_t`, 它是对`struct __wait_queue_head`的类型重命名, 这个等待队列里面就有`struct list_head task_list`, `struct task_struct`里面就有`struct list_head`, 这样就可以让那些`socket`资源不到位的进程排队了, 或者说阻塞了.

`struct socket`里面还有`struct proto_ops *ops`, 描述了协议的方法, 会被`file_operations  *`包装成文件的读写方法

![image-20250421215541385](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421215541765.png)

`struct socket`里面还有`struct sock*`, `struct sock`是连接的底层结构体, 你可以把它理解成连接的基类, 下面我们看看它的派生`struct tcp_sock, struct udp_sock`, 这两个派生类里面都封装了`struct sock`, `struct socket`指向的`struct sock`并不是纯`struct sock`, 而是在派生类`struct tcp_sock, struct udp_sock`里面的`struct sock`, 另外, 你还记得创建套接字时对其中的`family`初始化的`SOCK_STREAM, SOCK_DGRAM` , 系统就可以借助于这个字段判断这到底是在`struct tcp_sock`里面的`struct sock`还是在`struct udp_sock`里面的`struct sock`, 或者说, 是一种多态标志位.分得清自己是谁之后我们就可以通过指针强转来访问派生类的其它内容了. 另外我们还能看到`struct proto    *sk_prot_creator`, 这是`sock`的读写方法, 会被`struct proto_ops *ops`包装, 

然后我们可以从`struct sock`里面找到系统内的通用报文索引, `struct sk_buff_head`, 可分为接收和读写两个结构

![image-20250421224625163](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421224626180.png)

![image-20250421231352133](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421231352261.png)

还有`struct sk_buff_head`,  它们是对报文进行管理的结构, 你知道的, 系统里会有很多报文, 要对他们进行管理, 我们看`struct sk_buff_head`的内部定义, 可以发现这是对`struct sk_buff`的双向链表, 另外它可以认为是生产消费者模型中的那个临界资源, 所以它也有锁.

`struct sk_buff`描述的就是一个个的报文

![image-20250421234019190](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421234020004.png)

关于`sk_buff`可以看这篇[文章](https://blog.csdn.net/qq_38107043/article/details/124160693?fromshare=blogdetail&sharetype=blogdetail&sharerId=124160693&sharerefer=PC&sharesource=venti0411&sharefrom=from_link)

![image-20250421234936977](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421234937320.png)

然后`struct sk_buff`里面有这四个指针

![image-20250421235213708](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421235214044.png)

![image-20250421235233156](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250421235233242.png)

这是什么意思呢? 就是说, 不管你是传输层, 网络层, 链路层, 都是系统的一部分, 只要你这个报文进了系统, 都会被`struct sk_buff`索引, 

假设内存起始地址为 `0x1000`，分配了 128 字节连续缓冲区，头部预留空间如下：

| 协议层                | 假设头部大小 |
| --------------------- | ------------ |
| 链路层（Ethernet）    | 14 字节      |
| 网络层（IP）          | 20 字节      |
| 传输层（TCP）         | 20 字节      |
| 应用层数据（"Hello"） | 5 字节       |

内存布局如下所示（逻辑地址，不考虑实际字节对齐）：

```
head
 ↓
+---------+--------+--------+------------------+--------+
| Ethernet |   IP    |  TCP   | "Hello" (5字节) | padding |
+---------+--------+--------+------------------+--------+
↑         ↑        ↑        ↑                 ↑
0x1000    0x100E   0x1022   0x1036            tail
                     ↑
                    data
```

---

每一层对 `skb` 的操作如下：

传输层（TCP）：
- 初始 `data = 0x1036`，只包含 `"Hello"`
- 调用 `skb_push(skb, 20)`，`data = 0x101C`
- TCP 头写入 `[0x101C, 0x1036)`
- `tail` 保持在 `0x103B`

网络层（IP）：
- 调用 `skb_push(skb, 20)`，`data = 0x1008`
- IP 头写入 `[0x1008, 0x101C)`
- `tail` 不变

链路层（Ethernet）：
- 调用 `skb_push(skb, 14)`，`data = 0x0FF4`
- Ethernet 头写入 `[0x0FF4, 0x1008)`
- `tail` 仍为 `0x103B`

---

最终的内存结构如下：

```
         head          data             tail          end
          ↓             ↓                ↓             ↓
          +-------------+----------------+-------------+
          | 预留空间     |协议头(ETH/IP/TCP)| 应用数据     | padding |
          +-------------+----------------+-------------+
```

---

总结：

- `sk_buff` 中的 `head` 是缓冲区的起点，`end` 是缓冲区末尾，这两者在整个生命周期中**不会改变**。
- `data` 指针总是指向当前这一层协议应处理的首地址，**随着协议栈向下推进（从传输层到链路层），通过 `skb_push()` 向前移动**。
- `tail` 指向当前缓冲区中最后一个有效字节的下一个位置，即总数据的“末尾”。
- 每一层只需填写它的协议头，无需拷贝整个数据，**上层数据就位于下层数据之后，共享同一段内存**。
- 报文数据（如用户数据 `"Hello"`）始终在同一位置，没有移动、没有复制，**实现真正的“零拷贝”**。
- skb 在协议栈中传递的本质，是协议头逐层“向前添加”，数据内容保持不动，**层与层之间的交接靠的是对 `skb->data` 的指针变化，而不是数据本身的位置变化**。

## 网络层

在说网络层之前, 我们先简略通过一个生活实例来表述网路层和传输层的作用, 传输层提供的是策略, 而网络层及其以下的则是真正负责传送的那批人,

传输层 就像是一个物流公司，负责决定如何把货物（数据）从起点送到终点。它不仅关注如何确保货物顺利送达，还会有策略，比如选择运输方式、确保运输安全等。例如，传输层会决定用 TCP 还是 UDP 这种“运输方式”，并处理数据的完整性、顺序等问题。
网络层 则像是负责运输路线的工作人员。它的任务是确定货物的传输路径，从源地到目的地，经过哪些城市和道路，确保货物能够准确地到达。网络层使用的是 IP 地址来确定路由，就像是确定货物从哪个仓库出发，经过哪些转运站，最终到达客户手中。

网络层即使传输效率很高, 那也需要传输层的指挥. 再举个例子, 这个例子不太恰当, 有一个学生他数学非常好, 真的有实力考满分的那种, 但小概率会有一点疏忽, 差了一点, 这个学生不仅成绩好, 还是关系户, 他爸是教育局领导, 于是当该学生没考到满分时, 他爸就直接宣布考试作废, 重考, 直到他儿子考了满分. 在这个示例中, 这个学生就是传输层, 他本身有足够的能力去做事, 但要让事情做得特别好, 还需要策略(不满分重考)的支持.

网络层又叫IP层, 可见IP的重要性, 所以下面我们先来深入了解一下IP. 首先我们要知道IP分为两种, 私有IP和公网IP, 下面我们所说的, 如果不指名, 都默认是公网IP, 另外, IP地址可以分为两个部分, 目标网络和目标主机. 我们知道, 主机是存在于一个个子网中的, 如果一个在子网`A`中的主机要把信息交给在子网`B`中的主机, 那先别管什么, 先要把报文交到子网`B`里面, 然后才能交到另一个主机手里, 这个目标网络描述的就是子网, 目标主机描述的就是子网里的主机.

在报文从子网到另一个子网传送的过程中, 它采用的是一种递归式的策略, 会逐级到达更高的层, 够高后再走回更低的层. 这个递归怎么理解呢? 我们还是可以拿生活中的场景去举例, 毕竟我们是从应用层往下走的, 现在直接报那么多网络层的概念并不合适.

我们知道, 这个社会是分层的. 直接看社会有些太过复杂, 所以我们就看一个小的社会, 那就是大学. 在大学中, 有各种各样的学院, 什么我本人所在的计算机科学学院, 金融管理学院, 化学材料学院, 生命健康学院什么的, 每个学院都有自己的编号, 学校也有自己的编号, 为了很方便的区分一个学生到底是哪里的学生, 我们可以依据  大学编号, 学院编号, 专业编号, 班级编号, 学生编号的层级结构来一一编号,, 在这里我们就不搞这么复杂, 我们就认为, 一个学生的学号由两部分构成, 学院编号和学生号.

大学里有学生会, 我本人对学生会的架构不怎么了解, 因为我没有多余精力来操心学生会的东西, 但大致来说, 它应该是层状结构的, 最底下的是学院学生会, 然后是校学生会, 我们假设, 计算机学院的编号是443, 经管是427, 材料是464, 医学院是487, 每个学院都有各自的学生会, 每个学生会都有自己的学生会主席. 假设现在材料的学生捡到了一个学生卡, 学生号是42716, 此时这位学生该如何把学生卡还回去呢? 

第一种方案, 去人口密集的地方, 一个人一个人地问, 比如食堂, 随手栏一个人, 问一下, 你的学号是什么, 我这捡到一张学生卡, 看看是不是你的, 但很明显, 这种查找方式效率太过低下了, 

所以就有了方案二, 他是材料学生, 他本身是知道自己的学院编号的, 他知道这个42716一定不是我们院的, 所以我把这张卡交给材料学生会主席, 主席一看, 427是经管的, 所以他就去路由了, 他就在校学生会群里联系了经管的主席, 把卡交了过去. 经管的主席就找到了这位16的同学, 把它交还了原主. 

方案二为什么快呢? 因为它实际上是一种树结构, 这一层无法确定, 那就往上交, 总会有一层, 他大致可以判断出目的地应该是哪个子层, 然后就一层层地再往下走. 每一层都不需要知道全局的所有信息，只要知道自己负责的那一层、以及上一级或下一级的联系人就够了. 

我们先来看IP协议的报头

![image-20250423201909346](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250423201909432.png)

对于IP的报头来说, 我们不会像TCP那样说, 而是先把能讲的一次性说掉, 不能讲的到时候再说.

我们看到, IP的报头在宏观上和TCP是很相近的, 所以有时候我们叫"TCP/IP协议栈", 另外, 它和TCP也类似, 直接采用结构的方式进行报文传输,

还是老样子, 我们先学习报头和有效负载是如何分离的, IP又是怎么知道解出来的有效载荷交给传输层的哪个协议, 是TCP, 还是UDP?
我们看到, IP也有自己定长的标准报头, 它和TCP类似, 都是在`sk_buff`上原地解析报头, 因为IP标准报头是20字节, 所以先原地解析20字节, 然后我们看到, 它有一个4位首部长度, 这个4位首部长度和TCP的4位首都长度相同, 单位都是4字节, 这样的话, 固定的20字节就是5个单位, 所以这个4位首部长度的范围就是`5-15`, 可以依据4位首部长度来确定是否有非标准报头, 也就是选项部分, 
我们再回过头来看看4位版本, 这是什么意思呢? 当初我们的计算机工程师在设计IPv4的时候, 就意识到, 总会出现新一代的IP协议, 现在我们知道就是IPv6, 尽管我们说, 链路层应该是会把`sk_buff`交到网络层中的对应协议, 按道理来说, IPv4不可能收到IPv6的报文, 所以这个4位版本主要是为了增加容错度.加个保险, IPv6我们后面会稍微说一下
然后我们看看8位服务类型, 它有3位优先级字段(已弃用), 1位保留字段(保留的意思就是为以后可能要增加的新标记位占个位置, 现在还是没用, 必须置为0), 剩下的4位是可以被改动的, 这四位描述了路由器在给报文提供服务(其实就是报完转发)时的策略, : 最小延迟, 最大吞吐量, 最高可靠性, 最小成本, 一般来说, 这四种需求是冲突的, 只能选一种, 最小延迟就是快, 比如你坐飞机去某地, 最大吞吐量就是单位时间内传输更多的数据, 比如像坐火车, 承载量更大, 最高可靠性, 尽管IP不管可靠性, 但它可以尽可能选择最起码看上去更可靠的路径, 就像做经过官方认证的交通工具, 最小成本, 顾名思义, 选成本更小的.   
16位总长度, 顾名思义, 就是整份报文的长度, 它一方面可以确定负载有多大, 另一方面可以进行长度校验, 看看有没有丢长度.不过实际上, 直接看`sk_buff`也能确定长度.差不多也是一个保险的自描述字段

中间三个跳过去

8位生存时间, IP报文, 实际上所有报文都有点像炮灰, 单个报文丢了就真的丢了, 至于报文里的数据会使用传输层的可靠性策略重新传送, 有时候, 因为种种原因, 网络中会存在很多实际上已经迷路的报文, 总之到达不了目的地了, 此时, 它其实已经没有存在价值了, 还在网络中转发反而浪费资源, 所以就有一个8位生存时间, 描述了到达目的地的最大报文跳数. 一般是64. 每次经过一个路由,  就减一, 如果一个路由收到了8位生存时间已经为零的报文, 那就释放掉它, 不要让它瞎转悠.
8位协议也很好懂, 就像刚刚我们说的那样, 链路层它自已的报头应该含有将负载交给网络层哪个协议的描述字段, IP协议也是这样, 这个8位协议描述的其实就是该把解包后的数据分用给传输层的哪个协议
检验和顾名思义, 就是看报头有没有出错, 出错就丢失.
`srcIP`和`dstIP`想必不用说, 用来确定通信双方的主机的, 或者说, 是用来作为路由选择的依据的, 这个我们之前在网络基础一其实大致说过.

### 网段划分

我们之前说过, IP地址分为两个部分, 分别是网络号和主机号, 就像我们之前说的捡到学生卡的那个例子

![image-20250423213052790](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250423213052922.png)

我们先不管那个`/24`, 后面我们会说, 那是子网掩码, 但现在, 我们的关注点不在这里.

我们先补充四点, 都是关于路由器的

- 路由器本质也是网络中的一台主机, 也要配置ip地址
- 路由器凭什么能把一个子网中的数据转发到另一个子网中, 那就是因为它同时存在在两个子网中, 或者说至少同时连接着两个子网, 它在每个子网中都有对应的ip, 具体怎么连上两个网的我们软件不管, 可以认为它有多个网卡
- 路由器一般是子网中的第一台主机, 一般它的ip都是网络号.1
- 路由器可不止报文转发这一个功能, 还有很多其它的功能. 比如它可以构建一个子网(局域网), 一个经常被人忽视的路由器功能

关于上面的图, 这其实是两个局域网中的主机相互通信的场景, 里面的IP都是私有ip, 路由器为了管理子网中的ip资源, 往往会采用动态分配的策略, 当一个主机连接上路由器, 路由器就会为其分配一个私有ip, 主机离线了, 路由器会自动回收这个私有ip, 我们把这种技术称之为`DHCP`, 	

下面我们说说整个ip地址, 也就是不分私有公有的两种划分方式, 一种是旧的, 另一种是新的(对我们来说, 其实都很旧), 它们两个现在其实是在同时起作用的, 我们先说老的那个

在网络发展初期, 曾经提出一种依据比特位位置进行网络号和主机号划分的方案, 把所有IP 地址分为五类, 如下图所示

![image-20250423215925004](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250423215925087.png)

第一个比特位为零的, 称之为"A类网络", 其它的, 也就是第一个比特位一的, 被称为"非A类网络", "A类网络"的网络号位数为七, 所以"A类网络"一共有$2^7$个, 每个A类网络都可以容纳$2^{24}$个主机. 

在非A类网络中, 第二个比特位为零的, 称之为B类网络, 同理, 有$2^{14}$个B类网络, 每个B类网络中可以容纳$2^{16}$个主机

其它的都以此类推.

这种划分方式称之为"分类划分法", 但这种划分方法随着互联网的高速发展, 很快就出现了问题.

有很多组织都喜欢申请B类网络, 一是因为每个B类网络的主机号绝对够用, 二是因为B类网络本身有$2^{14}$个, 也是比较多的, 但因为大家都这样想, 所以B类网络反而很快就用尽了, 并且, 由于每个B类网络可容纳的主机数也是较多的, 所以B类网络在被真正使用时, 有很多网络号都没人用, 这就造成了很大的浪费.

这些网络具体怎么分配呢? 主要都是相关利益体经过协商进行分配的, 分配的主要依据是社会的发展情况以及实际需求, 比如中国的网络基础设施全球领先, 网民也多, 网络需求大, 那就多分配一点, 普通的发展中国家, 可能人多, 但基础设施不全, 就算给了太多ip它也用不起来, 所以就少给一点.

分类划分法已经适应不了现在的网络需求了, 因为现在的网络设备越来越多, ip已经不够用了, 分类划分法还容易造成浪费, 所以分类划分法尽管现在仍在使用, 但起辅助作用, 下面我们说的, 被称为"CIDR"的划分方法起主要作用.

"CIDR"在分类划分法的基础之上, 又引入了"子网掩码"这个概念

子网掩码有两种表示方式, 一是二进制的表示方式, 他也是一个无符号的32位整数, 左边都是1, 右边都是0, 形如`1111 1111 1000 0000 0000 0000 0000 0000`,  比如`255.255.255.0`(点分十进制风格, 255就是$2^8$, 也就是8个1), 当我们拿到一个ip, 比如`122.133.144.155`时, 就可以采用IP & 子网掩码的方式得到网络号, 比如在这里`122.133.144.155 & 255.255.255.0 = 122.133.144.0`所以`122.133.144.0`就是`122.133.144.155`以`255.255.255.0`为掩码的网络号, 此时我们就可以通过调节子网掩码中1的个数来转变某个ip对应的网络号. 多加几个1, 网络号就更多, 但主机号就变小, 少加几个1, 那就反过来了, 所以就可以用子网掩码动态调整网络号的主机号的范围.

子网掩码被内嵌到路由器当中, 我们只需要操心那个最开始的ip就行了, 路由器在转发报文中, 会通过子网掩码按位与ip得到网络号, 依据这个得到的网络号判断路由方向.

子网掩码的另一种表达方式就是`/24`这种形式, 这在之前的图上我们见过, 表示的就是前面有24个1, 也就是`255.255.255.0`

![image-20250424145921558](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250424145921666.png)

在示例一中, 由于它的网络号是`140.252.20.0`, 主机号是低8位,  所以子网地址范围就是`140.252.20.0~140.252.20.255`, 其中的`140.252.20.0 `和`140.252.20.255`有特殊用途, 所以可容纳的主机数是254.

在示例二中, 它的网络号是`140.252.20.64`, 主机号是低4位(掩码中1的数量可以随便定), 也就是`1+2+4+8 = 15`, 所以子网地址范围就是`140.252.20.64~140.252.20.79`, 可容纳的主机数是`16-2=14`.

将ip地址中的主机号部分全部设为0, 得到的就是网络号, 代表这个子网, 如`140.252.20.64, 140.252.20.0`, 将ip地址中的主机号部分全部设为1, 得到的就是广播地址, 用于给这个子网中的所有主机发送数据, 127.*的ip地址用于本地环回, 通常是127.0.0.1

子网掩码和分类划分是怎么结合起来的呢? 首先我们可能拿到了一个B网, 前面的, 由分类划分规定的前16位我们是动不了的, 但我们可以在后面的原本充当主机号的16位中做手脚, 把其中的一部分比如说其中的前12位作为我这一层的网络再划分, 这样我手上的一个B网, 就可以被拆成$2^{12}$个子网, 每个子网只能容纳14个主机, 这就相当于在我这一层对网络进行了更细的划分.

![image-20250424154736791](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250424154736883.png)

就现在来说, ip地址已经严重不足了, 为了应对这个问题, 我们有两种方法

一是缓兵之计, 提高ip地址的利用率, 比如动态分配ip, NAT技术(后面重点说), 以及刚刚我们说的子网掩码, 
二就是发展新一代的ip协议, 即IPv6.

IPv6技术最发达的就是中国, 在IPv6中, ip地址是128位, IPv6与IPv4完全不是一个协议, 不过因为IPv4是硬编码写到系统里面的, 所以不好改, 只能局部, 也就是中国支持, 中国IPv6强有多种原因, 一是我们基建很好, 我们有庞大的工程师团队, 二是我们的网络需求确实很大, 三是IPv4它毕竟不是中国的, 政府层面是强制要求相关产业支持IPv6的.

----------

为了缓解IP地址紧张的问题，可以使用NAT（网络地址转换）技术。它的基本思想是在公网之下建立多个私有网络（私网）。这些私网并不直接连接公网，而是通过路由器访问公网。私网中的主机使用私有IP地址，这些地址不能直接在公网中使用，因为不同私网中可能存在相同的IP地址，容易发生冲突。

- `10.0.0.0/8`：前8位为网络号，共约1677万个地址
- `172.16.0.0/12`：前12位为网络号，涵盖 `172.16.0.0` 到 `172.31.255.255`，约104万个地址
- `192.168.0.0/16`：前16位为网络号，共约6.5万个地址

剩下的ip则是公网ip.

对于我们来说, 平常用的都是私网ip, 在Windows, 我们可以使用`ipconfig`指令查看自己的网络信息, 在Linux, 使用`ifconfig`进行查看

```shell
[whisper@starry-sky ~]$ ifconfig
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 10.0.12.6  netmask 255.255.252.0  broadcast 10.0.15.255
        inet6 fe80::5054:ff:fe6a:5fbb  prefixlen 64  scopeid 0x20<link>
        ether 52:54:00:6a:5f:bb  txqueuelen 1000  (Ethernet)
        RX packets 16977939  bytes 3197360560 (3.1 GB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 15349472  bytes 1533171444 (1.5 GB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

[whisper@starry-sky ~]$






PS C:\Users\21066> ipconfig

Windows IP 配置

无线局域网适配器 WLAN:

   连接特定的 DNS 后缀 . . . . . . . :
   IPv6 地址 . . . . . . . . . . . . : 2409:8931:ab:265d:c18a:f8f6:4f72:4260
   临时 IPv6 地址. . . . . . . . . . : 2409:8931:ab:265d:fcf8:2a7d:fc68:82e5
   本地链接 IPv6 地址. . . . . . . . : fe80::d4a3:9643:d5f8:adcc%4
   IPv4 地址 . . . . . . . . . . . . : 192.168.43.222
   子网掩码  . . . . . . . . . . . . : 255.255.255.0
   默认网关. . . . . . . . . . . . . : fe80::6c6b:40ff:feb5:c05a%4
                                       192.168.43.1
```

如上, 这台Linux主机的内网ip是`10.0.12.6`, Windows主机的ip是`192.168.43.222`, Xshell的那个云服务器地址是公网ip.

### 理解运营商

对于我们来说, 如果要上网, 首先先要让运营商拉条网线到家里, 当然, 对于农村地区来说, 可能确实要主动通知一下运营商, 让他们拉根网线, 但对于城市地区, 光纤入户是默认的会做的事, 所以不用主动联系运营商.

光纤或者其他网络传输媒介, 它们本质上是在传输模拟信号, 光电信号那不就是模拟信号吗, 所谓模拟信号就是自然中本就有的那些信号, 但计算机用的可不是模拟信号, 而是数字信号, 所以就需要进行数字和模拟信号的转换, 收到的模拟信号要转成数字信号, 发出的数字信号要被转换成模拟信号, 这个工作, 就是用调制解调器, 也就是我们平常说的"光猫"来实现的. 网线先要连光猫上, 然后再从光猫中拉一条线连到路由器上.

路由器有两种账号密码, 一种是面向运营商的, 你要给运营商交钱, 然后才能获得这个账号密码, 这里的账号和密码就可以映射你的网络费用, 不欠费, 运营商才会允许这个路由器访问网络, 在这之后, 路由器就可以构建一个局域网, 就现在来说, 那主要是无线网.

这样, 用户就可以使用无线网连接网络了, 但不仅我这个用户可以连无线网, 邻居也可以, 所以还要一个面向用户的账号密码, 就是平常我们新连接无线网时输入的那个账号和密码. 所以家用路由器一头连的是家庭的局域网, 一头连运营商.

我们知道, 中国的互联网应用技术真的是遥遥领先于第二的那种, 我们有一大批的互联网公司, 有着其他国家不可想象的移动支付, 共享经济, 极其发达的电商平台, 但这些繁荣的背后, 离不开运营商所提供的网络基础设施支持.

没有运营商全中国地建机房, 搭信号塔, 光纤入户之类, 这些繁荣就无从谈起, 诚然, 运营商也有黑历史, 但它的功绩值得被肯定. 强大的网络基础设施支持, 庞大的用户基数, 一大批的互联网公司, 三者共同维持这这种繁荣局面, 从而创造出更多的价值, 利益.

基础设施的建设往往有着很长的回报周期, 一个信号塔, 可能搭了20年才能回本, 钱赚的太慢了, 所以运营商可能也不是很想投资建设基础设施, 但它们是国企, 让私企去搞基础设施建设确实强人所难, 所以这个职责只能落到国企上. 政府要求, 国企要大力发展基础设施建设, 而且在建设完毕后, 还要尽可能让更多的人去使用这些基础设施, 要让更多人去用, 就要降低利润率, 降网费, 搞优惠, 所以钱赚的就更慢了. 	

### 理解全球网络

谈到全球, 那自然要涉及到各个国家, 实际上,  对于国家层面的ip地址分配, 是非常复杂的, 但在这里, 为了简化模型, 我们就简单地认为, 因为现在大概有200多个国家, 这样的话, 在我们的模型中, 就认为ip的前八位是国家号, 美国是`0000 0001`, 中国是`0000 0010`, 俄罗斯是`0000 0011`, 德国是`0000 0100`, ......这些国家都有自己的国际路由器可以连上国际网.这样的话, 各个国家级别的子网掩码就是8

![image-20250424180137423](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250424180137621.png)

中国有34个省级行政区, 这样的话, 就可以依据行政区对`0000 0010`开头的ip进行再划分, 这样的话, 我们可以从剩下的24个位中取出6位作为省的区分, 这样省级的掩码就是`/14`, 它们也有各自的省级路由器

![image-20250424182140993](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250424182141157.png)

每个省下面都有自己的若干个市, 为了区分生省下面的各个市, 我们才拿出4个比特位用来区分, 这样市一级的掩码就是`/18`, 自然它们也有自己的市路由器

![image-20250424182817602](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250424182817769.png)

下面不继续划分了.现在假设西安市现在有一台使用公网的主机, ip地址是`0000 0010 0001 0000 0100 0000 0000 0101`,  `2.16.64.5`, 假设现在有一台美国主机, 是`1.8.128.5`, 现在, 美国的这台主机想访问西安的这台主机, 由于`2.16.64.5`不是美国ip, 所以美国的国际路由器就会把报文扔到国际网中, 中国国际路由器通过子网掩码算出来这是中国的ip, 所以就会把它转发到中国内网中, 各省的路由器也对其进行了检测, 其中陕西省的省路由器发现这是我这个省的, 所以就会把报文转发到陕西省里的子网, 然后西安市的路由器通过把这个ip和自己的子网掩码进行了按位与, 发现是自己这个市的, 于是就会转发到市里, 然后就找到了西安的这台主机.

从报文踏进中国的国际路由器, 这份报文就被运营商的基础设施进行了一次次的转发, 最终到达具体的公网上.

如果是从西安往南京发, 那到达省一级就会被江苏省的省路由器转发.

上面我们说的都是公网ip, 实际上, 对于我们来说, 我们用的都是私网ip, 那私网和公网是怎么连起来的呢? 

![image-20250424193302113](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250424193302227.png)

在这张图中, 我们所处的位置就是最末尾, 家用路由器下的那些`192.168.1.x`, 如果现在我们要访问公网上的一台主机`122.77.241.3`

现在如果`srcIP: 192.168.1.201`要访问`dstIP: 122.77.241.3`, 首先我们的本地主机会意识到这个`dstIP`肯定不是我所处于的子网中的, 所以就会发给家用路由器, 家用路由器一看, 这也一定不是我所在的子网, 所以就把它又转发给了运营商路由器, 我们看到这个运营商路由器真的连着公网的, 所以运营商路由器就把报文转发到公网中, 最终被`122.77.241.3`接收到, 这样的话, 服务端就会给我们构建一份响应报文, 但我们发现, 出问题了, 因为如果真的像刚刚所说的这样直接转发的话, 服务端收到的可是一个私有ip, 所以这个响应报文就回不来了, 要知道, 即使是不同的私网, 也可能出现相同的私网ip

如何解决这个问题呢? NAT技术解决的就是这个问题. 我们看到, 运营商的入口路由器(那个连着多个私网和一个公网的运营商服务器就是一种入口路由器), 它是有公网ip的, 报文最开始被构建出来的时候是`src: 192.168.1.201, dst: 122.77.241.3`, 到了家用路由器这里, 它就会把这份报文收到网络层上, 进行解包, 并重新加了网络层报头, 或者说是更改了之前的那份网络层报头, 改成了`src: 10.1.1.2, dst: 122.77.241.3`, 然后再转交给运营商的入口路由器, 运营商的入口路由器也会修改报头, 变成`src: 122.77.2412.4  dst: 122.77.241.3`, 此时报文的源ip就变成了公网ip, 此时服务端的响应报文就会被发到运营商入口路由器, 至于后面怎么回来, 我们等会儿说, 这也是NAT的事.      

你可以认为刚刚我们说的那个用公网的西安主机就相当于上面的运营商入口路由器.因为到地方, 公网ip就不够划分了, 所以下面就要用私网. 

这种模式我们称之为  公网 + 私网 = 互联网

另外, 我们也可以看到, 在家庭级别, 我们用的是`192.168.0.0`这种级别的私网ip, 在更高的层次, 用的就是`172.16.0.0` 到 `172.31.255.255`或者`10.0.0.0`这种可以容纳更多主机的私网ip了. 

现在我们更细的看一下, 对于上图中左上角的那个家庭服务器来说, 它有两个`ip`, 这两个`ip`分别处于不同的层次上, `192.168.1.1/24`, 代表它是`192.168.1.0`这个网络的第一台主机, 管理着这个子网, 它的另一个ip是`10.1.1.2/24`, 处于`10.1.1.0`这个子网中, 是其中的一个普通的主机, 我们把面向更高层的ip称为"WAN口ip", 而把面向更底层网络的那个ip叫做"LAN口ip", 这样它们就形成了一个分层的网络结构, 网络末梢用的都是私网, 到达运营商的入口路由器被修改成公网ip, 就能在公网中进行传送.

![image-20250425132125360](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250425132125737.png)

刚刚我们是从用户的角度来看网络的, 对于服务方, 来说, 也就是那些互联网公司来说, 首先它们肯定是要有自己的公网ip的, 问题只是有多少而已, 并且由于服务量可能会很大, 它们也会搭建自己的机房, 自己的内网, 也有自己的直接使用公网ip的入口路由器, 当用户访问它们时, 入口路由器就会把请求映射到机房内一个具体的机器上, 从而提供相应的服务.

### 路由

上面的过程其实就是路由, 网络通信的报文在一个个主机之间不停跳转, 最终来到目标主机的过程.

![image-20250425140127284](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250425140127451.png)

下面我们要说的就是, 路由器(不要把路由器想的太过狭窄, 是个主机都能做路由器, 比如电脑连接手机热点的时候, 手机其实就是路由器, 或者对家庭进行网络配置时, 也可以完全不要路由器, 直接拿一个废旧电脑充当软路由, 这样还能实现对家庭网络设备的代理服务),     路由器是怎么决定路由方向的呢? 实际上, 每个路由器上都会维护一张路由表, 通过这张路由表来决定来到这台主机的报文, 应该转发到哪里.

我们可以使用`route`指令来查看本机的路由表(对于主机上层发来的报文, 这个主机本身的网络层也要对其进行路由, 判断这个报文应该给路由器还是直接给子网中的一台主机)

```shell
[whisper@starry-sky ~]$ route
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
default         _gateway        0.0.0.0         UG    100    0        0 eth0
10.0.12.0       0.0.0.0         255.255.252.0   U     100    0        0 eth0
_gateway        0.0.0.0         255.255.255.255 UH    100    0        0 eth0
183.60.82.98    _gateway        255.255.255.255 UGH   100    0        0 eth0
183.60.83.19    _gateway        255.255.255.255 UGH   100    0        0 eth0
```

Windows用`route PRINT`

```shell
PS C:\Users\21066> route PRINT
===========================================================================
接口列表
  8...ce 47 40 b1 34 a6 ......Microsoft Wi-Fi Direct Virtual Adapter
 15...ee 47 40 b1 34 a6 ......Microsoft Wi-Fi Direct Virtual Adapter #2
 13...00 50 56 c0 00 01 ......VMware Virtual Ethernet Adapter for VMnet1
 18...00 ff bf 34 5f 93 ......Netease UU TAP-Win32 Adapter V9.21
 20...00 50 56 c0 00 08 ......VMware Virtual Ethernet Adapter for VMnet8
  4...cc 47 40 b1 34 a6 ......Realtek RTL8852BE WiFi 6 802.11ax PCIe Adapter
 10...1c 83 41 ca 23 14 ......Realtek PCIe GbE Family Controller
  1...........................Software Loopback Interface 1
===========================================================================

IPv4 路由表
===========================================================================
活动路由:
网络目标        网络掩码          网关       接口   跃点数
          0.0.0.0          0.0.0.0   172.20.203.254    172.20.203.47     45
        127.0.0.0        255.0.0.0            在链路上         127.0.0.1    331
        127.0.0.1  255.255.255.255            在链路上         127.0.0.1    331
  127.255.255.255  255.255.255.255            在链路上         127.0.0.1    331
     172.20.202.0    255.255.254.0            在链路上     172.20.203.47    301
    172.20.203.47  255.255.255.255            在链路上     172.20.203.47    301
   172.20.203.255  255.255.255.255            在链路上     172.20.203.47    301
    192.168.150.0    255.255.255.0            在链路上     192.168.150.1    291
    192.168.150.1  255.255.255.255            在链路上     192.168.150.1    291
  192.168.150.255  255.255.255.255            在链路上     192.168.150.1    291
    192.168.248.0    255.255.255.0            在链路上     192.168.248.1    291
    192.168.248.1  255.255.255.255            在链路上     192.168.248.1    291
  192.168.248.255  255.255.255.255            在链路上     192.168.248.1    291
        224.0.0.0        240.0.0.0            在链路上         127.0.0.1    331
        224.0.0.0        240.0.0.0            在链路上     192.168.248.1    291
        224.0.0.0        240.0.0.0            在链路上     192.168.150.1    291
        224.0.0.0        240.0.0.0            在链路上     172.20.203.47    301
  255.255.255.255  255.255.255.255            在链路上         127.0.0.1    331
  255.255.255.255  255.255.255.255            在链路上     192.168.248.1    291
  255.255.255.255  255.255.255.255            在链路上     192.168.150.1    291
  255.255.255.255  255.255.255.255            在链路上     172.20.203.47    301
===========================================================================
永久路由:
  无

PS C:\Users\21066>
```

当一台主机的网络层收到一份报文时, 不管是从本机上层收到的, 还是从链路层传过来的, 网络层都会按照子网掩码从长(1的个数多)到短的顺序, 得到这份报文的网络号(言外之意就是有多个匹配成功的, 选择1更多的那个), 并把这个网络号和路由表中的目标网络(Destination)进行比对, 比对成功, 仅从发送的角度来说, 他就会把报文的ip地址先改成自己的,  然后依据对应的网关(Gateway, 网关就是下一台主机), 通过一些手段(链路层我们会说), 拿到下一个主机的MAC地址, 我们说过 子网是用MAC地址进行分辨的,  然后把报文和下一条的Mac地址交给链路层, 让它发给下一个网关的主机, 如果一直匹配不到, 它就会使用默认网关, 所谓默认网关, 其实就是连接更上层网络的那个主机, 默认网关的意思就是我不知你该去哪里, 你应该去问问更高的网络层             另外我们看到路由表里面也有子网掩码, 这是什么意思呢?   意思就是说就是如果一个报文ip通过掩码A得到网络号, 那在进行匹配时, 只会对使用A掩码的项进行匹配, 而忽略采用其它掩码的目标网络      

最后还有一个`eth0`, 其实就是对物理层网卡的标识, 意思是用`eth0`这张网卡发出去.      另外我们看到网关有时候是全零, 它的意思是目标ip就在这个子网中, 你直接发到子网中就行了, 不需要给下一个路由器.

查路由表(问路)有三种结果, 分别是

- 我什么都不知道, 不要问我(现实中会有, 但网络中出现这种情况一般来说意味着出故障了, 报文被丢弃, 当然也有可能遇到墙了)
- 我虽然不知道, 但我知道谁应该知道(去问问本地熟人,  去更高层网络的路由器(默认网关))
- 我知道, 给你具体路径(目的ip对应的目标网络就在路由表上, 相当于已经离得很近了, 甚至我这个路由器直接连接目标ip)

下面我们看一个被专门配置过的路由表, 适合作为实例进行演示

```shell
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
192.168.10.0    *               255.255.252.0   U     0      0        0 eth0
192.168.56.0    *               255.255.252.0   U     0      0        0 eth1
127.0.0.0       *               255.0.0.0       U     0      0        0 lo
default         192.168.10.1    0.0.0.0         UG    0      0        0 eth0
```

这里的`*`就是上面的`0.0.0.0`, 我们可以看到, 这是一个明显的局域网场景, 这台主机同时连着两个子网`192.168.10.0, 192.168.56.0 `, 标记位`U`表示直接相连, 不需要跳转, 直接把报文扔进子网就行了, 此外我们还看到了本地环回`lo`, 并且可以确认它是有两个网卡的

当该主机的网络层收到一封报文, 目标ip是`192.168.56.3`, 他先用`255.255.252.0`得到了网络号`192.168.56.0`, 在第二个项匹配成功, 就直接发到对应的子网中

如果目标ip是`202.10.1.2`, 发现没有匹配的, 就发给默认网关.

## 数据链路层

与传输层相同, 传输层并不是把报文直接发到网络上, 而是把报文先交给网络层, 对于网络层来说, 它也不是直接把报文发到网络中, 而是交给数据链路层, 数据链路层实际上已经到了网卡驱动层的那个地方了, 之后就要真的被发送出去了.

我们之前说过, 链路层不能一次性发送太大的数据, 所以这就要求更上一层, 也就是网络层, 每次只能向下交付一定大小的报文. 我们可以通过`ifconfig`来查看链路层负载的最大长度, 简称为`MTU`

```shell
[wind@starry-sky ~/linux-study]$ ifconfig
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.31.235.81  netmask 255.255.240.0  broadcast 172.31.239.255
        inet6 fe80::216:3eff:fe1c:d78d  prefixlen 64  scopeid 0x20<link>
        ether 00:16:3e:1c:d7:8d  txqueuelen 1000  (Ethernet)
        RX packets 265113  bytes 52307447 (52.3 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 184139  bytes 23650278 (23.6 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

```

我们看到, 这台主机的链路层最大负载长度是`1500`, 链路层负载, 也就是应用层报文 + 传输层报头  + 网络层报头的长度和.  如果传输层给网络层交付了一个很大的报文(TCP在三次握手时会通过非标准报头的选项字段间接商讨每次传送的报文大小, 从而尽量满足链路层的要求, 但对于UDP来说, 因为没有连接, 所以很容易出现这种情况, UDP理论最长长度是$2^{16}$, 即65536字节,  对于1500来说, 是很大的数字),     网络层就需要对负载进行"分片"操作, 简要的说, 就是对传输层报文进行拆分, 分成多次来进行发送.     链路层是直接操作硬件的, 它亲自拆分数据不合适, 并且链路层要随着具体的局域网环境进行协议更换, 如果是链路层拆分, 它就需要频繁地进行拆分合并, 因为报文需要网络层的路由, 如果是网络层进行拆分, 只有源主机网络层需要拆分, 目标主机网络层需要合并,   中间的网络层可以直接进行转发, 不考虑它们是不是完整的.

为此, 我们需要重回网络层, 来说明ip协议的"分片"操作, 以及未说明的, 与"分片"有关的IP报头字段, 就是之前我们省略的那一行

![image-20250425161645342](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250425161645563.png)

首先我们需要需要强调一点, ip协议的分片操作是把传输层报文分成多个份, 每份都再添加一个ip协议报头, ip协议报头的16位标识符表示传输层报文的编号, 也就是说, 如果若干个ip报文的16位标识符相同, 意味着其中的负载来自于同一份传输层报文, 它们需要被合并.

3位标志中的第一位是保留位, 必须为零, 第二位表示报文在传输过程过程中是否允许二次分片, 中间机器的`MTU`可能会小于源和目的主机的`MTU`, 此时, 为了让数据继续传递下去, 中间机器的ip协议就会提取之前ip报文中的负载, 并再次对其进行分片, 第二位为1表示禁止中间机器二次分片, 如果出现上述情况直接丢弃报文, 向源主机发回一份错误报文, 为0表示允许二次分片, 或者说多次分片.  第三位用来判断这个ip报文中装的负载是不是最后一个传输层小份, 是的话, 就置为1, 不是, 则置为0. 

13位片偏移表示ip报文中负载对于最开始的那份完整的传输层报文来说, 偏移量是多少, 可以用来进行报文次序的判断. 比如, 现在传输层给ip交付了一份4440字节的报文, ip协议可以将其分为三份1480字节的小份, 这样每个小份加上ip自己的20字节报头恰好就是1500, 对于第一份ip报文来说, 由于其中的负载就是传输层报文的开头部分, 所以片偏移量就是0, 而对于第二份ip报文来说, 由于装的负载是传输层报文的中间部分, 所以片偏移量就是1480, 而对于第三份ip报文来说, 由于装的是传输层报文的末尾, 所以片偏移量就是2960

对于目标主机来说, 如果它收到了多份ip报文, 结果发现它们具有相同的标识, 那就说明, 它们是被分片的, 如果暂时只收到一个ip报文, 对于第二份来说, 可以通过片偏移量和第三位标记, 确定它是分片报文, 如果是第三份的话, 可以通过片偏移量确认它是分片报文, 对于第一份来说, 可以通过第三位标记确定它是分片报文.    也就是说, 如果没有分片, ip报文的片偏移量应该为零, 第三位标记应该为1, 二者同时达到, 才是未分片ip报文.

当目标主机通过片偏移量大小是否合理, 以及是否存在第三位标识为1的, 就可以判断出是否收到了所有的分片报文, 如果丢失任意一个, ip协议都会将全部分片报文统统丢弃. 这里我们看到, 分片是只要丢一个, 那就相当于全丢, 所以实际上我们不建议分片, 为此我们最好用TCP, 如果要用UDP, 那就让应用层自己分片.

好的, 现在我们可以回到链路层了, 首先我们需要知道, 链路层解决的是, 直接相连的主机之间, 进行数据通信的问题.  为此我们需要重谈一个概念, 那就是Mac地址.每个网卡都有一个Mac地址, 仍然是`ifconfig`

```shell
[wind@starry-sky ~/linux-study]$ ifconfig
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.31.235.81  netmask 255.255.240.0  broadcast 172.31.239.255
        inet6 fe80::216:3eff:fe1c:d78d  prefixlen 64  scopeid 0x20<link>
        ether 00:16:3e:1c:d7:8d  txqueuelen 1000  (Ethernet)
        RX packets 265113  bytes 52307447 (52.3 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 184139  bytes 23650278 (23.6 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

```

`ether`后面的`00:16:3e:1c:d7:8d`即为Mac地址. Mac地址的作用就是, 区分同一个局域网下的各个主机.

以太网帧(以太网协议报文)格式如下

![image-20250425175305885](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250425175306041.png)

绝大多数情况下, 链路层协议的负载都是ip报文, 当然后面我们也会说`ARP, RARP`这种协议.  

首先还是那两个老问题, Mac帧如何做到解包和封装, 如何进行分用

其实很简单, 在上图中, 我们看到, Mac帧采用的是定长报头方案, 所以可以很容易地通过`sk_push`封装, 或者解包, 至于分用, 我们也可以看到, 它有个类型字段, 所以就可以依据该字段来判断应该把负载交给更上层的什么协议. 

局域网的通信原理, 其实我们在网络基础就已经说过了.而且还是比较详细的. 

局域网中的每台主机都和局域网直接相连

![image-20250311195311448](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311195311538.png)

在经过上面的学习之后，我们知道，局域网的通信虽然并非全程加密，但应用层的数据一般都会被加密，其它协议层则通常是明文的。因此，当其中一台主机发出报文时，除了目标主机之外（这里暂不考虑跨局域网），其它主机虽然都能感知到有报文在传输，但在链路层捕捉并解包之后，会发现这份报文并不是发给自己的，便会将其丢弃。        各个主机虽然可以知道“这份报文存在”，但因为加密发生在应用层，它们无法直接获取报文中的实际内容。或者说，虽然存在被监听的可能性，但一般并不值得专门去破解这些加密内容。

如果两个主机同时发送数据, 便会发生数据碰撞问题, 那若是发生数据碰撞, 它们能够意识到自己发的报文失效了吗, 当然是可以的, 这两个主机同样会捕捉局域网中的报文, 包括自己发出的, 当它捕捉不到, 那便意味着Mac帧失效了, 此时两台主机就会执行碰撞避免算法, 也就是稍微等待随机时间, 然后再对其进行重新发送.  从系统层面, 我们可以认为局域网就是多台主机的临界资源.

当进行跨局域网通信时，本机会查询路由表，根据目标 IP 判断该地址是否在本局域网内。如果不在，就会将报文发送给默认网关，也就是路由器。路由器收到这份报文后，它的链路层发现该报文的目的 MAC 地址是自己，于是接收并解包，然后将负载交给网络层。网络层会再次根据路由表判断下一个跳的主机是谁，再让链路层重新封装，并发送给下一个主机。

我们注意到，在整个过程中，IP 层的源 IP 和目的 IP 保持不变，但MAC 层的源 MAC 和目的 MAC 会不断变化。这正体现了链路层“点对点通信”的特点：每次发送的 MAC 报文只负责本机到下一跳之间的通信，就像一次次的接力传递。

不过，这里我们先略过一个问题：我们如何知道“下一跳主机”的 MAC 地址？我知道它的 IP 地址没错，可局域网通信依赖的是 MAC 地址。这时就需要引入 ARP 和 RARP —— 它们正是为了解决“IP 到 MAC 的映射问题”而设计的，我们将在后面详细讲解。

另外我们还要说, 随着局域网中的主机个数增多, 数据碰撞的概率就会越来越大, 甚至完全无法正常工作, 此时我们就需要引入交换机这种网络设备. 交换机可以划分碰撞域, 

![image-20250311204645010](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250311204645083.png)

左边发生碰撞, 没关系, 交换机会识别到, 不把左边的报文转到右边, 没发生碰撞, 那依据Mac帧的目的Mac地址判断是否是右边的, 是, 那就转发过去, 不是, 就不转发, 这样, 原先的一个局域网就被交换机划分成了两个相对独立的部分.

在链路层的通信中, 每次报文的发送还是短一些比较好, 因为报文短, 发到网上就快, 占用局域网的时间就少, 数据碰撞概率就低, 因此, 我们对链路层的负载长度进行了限定, 一般来说, 就是1500字节.

链路层最长负载是1500, 这意味着, 网络层的最长负载是1480, 传输层的最长负载是1460,(简称是`MSS`) 当然, 这是不考虑非标准报头, 也就是选项部分的情况下, 对于TCP来说, 在三次握手时, 会通过TCP的选项, 交换通信双方的`MSS`, 并选择其中最小的那个作为通信时的`MSS`, 在这之后, 即使滑动窗口很大, 网络很好, 拥塞窗口大, 对方窗口也很大, 我也不会一次把滑动窗口的数据全发出去, 而是把滑动窗口拆成多份, 每份最大就是通信时双方约定的那个`MSS`. 

现在我们回来, 去解决之前提出的问题: "我这个主机是如何靠下一个主机的ip地址得到下一个主机的Mac地址, 从而将ip报文重新封装到链路层, 进而进行局域网通的呢?"

这就需要依靠`ARP`协议, 该协议工作在链路层上端, 离网络层更近, 但还是在链路层的事, 那就是依据目标主机的ip地址, 找到目标机器的Mac地址. 还是为了局域网中主机的相互通信.

还是这张图, 我们假设源主机就是图中的主机A, 目标主机就是图中的主机B, 现在报文已经传到了B所在子网的入口路由器, 如图现在, 我们假设, 入口路由器的LAN口ip, 或者说子网ip是`172.20.1.1`, 它面向子网的那个网卡的Mac地址是`6A:3D:92:BE:18:4F`, 主机B的ip是`172.20.1.2`, Mac地址是`08:00:20:74:CE:EC`, 入口路由器现在已经依据路由表知道这份报文的目标ip就在这个子网里, 现在他只需要拿到B的Mac地址就可以构建Mac帧, 向子网中发送数据了.

![image-20250426180444034](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426180444257.png)

就基本原理来说, 其实很简单, 入口路由器会对整个子网进行广播, 询问谁的ip地址是`172.20.1.2`, 并请求对应的主机将自己的Mac地址发过来.

![image-20250426180929648](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426180929761.png)

下面, 我们就这个原理, 模拟一遍这个过程. 我们先看一个`ARP`的请求/应答报文格式

![image-20250426185258594](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426185258715.png)

首先, 我们先确定一下`ARP`的归属问题, `ARP`协议是链路层的上层协议, 在ip协议之下, 在以太网协议之上.   然后我们谈一谈其中的字段, 首先第一个字段, 硬件类型, 描述下层所采用的链路层传输协议, 在这里, 指的是以太网协议,  用`1`表示,           协议类型, 指的是网络层所采用的具体地址形式, 在这里是IPv4, 用`0x0800`表示, 硬件地址长度描述链路层地址的长度, 这里采用的是Mac地址, 长度为6字节, 填`6`,      协议地址长度, 描述网络层地址的长度, 对于IPv4来说, 是4字节, 填`4`,            op字段, 描述报文种类, 填`1`表示这是一个`ARP`请求, 填`0`表示这是一个`ARP`应答. 

前面这些字段大多时候都是固定的, 主要看后面的, 发送端以太网地址那就是本机的Mac地址, 发送端IP地址就是本机的IP地址, 目的以太网地址, 我们现在并不知道目的以太网地址, 所以可以全部填为`1`, 表示广播, 目的IP地址自然就是报文的那个目的IP.

首先, 入口路由器先构造一份`ARP`报文

![image-20250426200153951](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426200154076.png)

然后再把这份报文交给链路层, 更详细的说, 是以太网协议.

![image-20250426202355381](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426202355480.png)

![image-20250426201153298](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426201153421.png)

![image-20250426201843103](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426201843201.png)

`ARP`的类型表示是`0806` , 所以帧类型是`0806`,  注意中间的那个报文格式是缺了链路层的校验和`CRC`的

此时入口路由器的链路层把这份Mac帧发到局域网中, 其它主机的链路层会对其进行解包, 发现Mac地址全是1, 这意味着这份Mac是份广播, 又因为栈类型是`0X0806`所以把解包出来的报文交给了自己的`ARP`协议, 

就我们现在的逻辑来说, 别的主机`ARP`协议在收到一份`ARP`报文后, 可以直接看一下目的ip地址, 发现不是自己, 所以就会把报文丢弃, 但实际上, 并不是这样, 我们是站在全局视角来看的, 但对于`ARP`协议来说, 它有可能也刚刚发了一份`ARP`请求, 它现在收到一份`ARP`报文, 可能就是对它之前`ARP`请求的应答, 也有可能是别的主机对自己的`ARP`请求, 所以实际逻辑中, `ARP`收到一个报文会首先看`op`字段, 然后再做其他决策, 在通过`op`知道它是一个请求之后, 他才会去看目标ip地址

对于局域网中除了B之外的其他主机来说, 在比对完之后, 发现,目的ip不是自己, 所以就会将其丢弃, 而对于主机B来说, 会发现这就是我的ip地址, 所以就会构造一个`ARP`应答报文.

![image-20250426204558573](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426204558698.png)

此时发送方就是主机B, 它知道自己的Mac地址, 发送方的Mac地址就是主机B的Mac地址

由于知道`ARP`请求者的Mac地址, 所以响应就不会广播了, 而是直接发送

![image-20250426205418958](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250426205419050.png)

在发出去之后, 入口路由器的链路层就捕捉到了这个报文, 发现这个报文就是发给自己的, 然后依据帧类型, 把解出来的报文交给了`ARP`协议, `ARP`还是先看`op`字段, 发现是`2`, 于是看了发送者Mac地址, 就得到了主机B的Mac地址

从上面的过程中, 我们看到对于`ARP`请求, 看的是目的ip, 对于`ARP`的响应, 看的是发送ip, 或者这样说, `ARP`是依靠`op`调用不同的逻辑的.

另外, 对于通过`ARP`得到的Mac地址会在本机中暂存一会, 我们可以通过`arp -a`来查看本机中暂时存储的`arp`信息, 缓存时间一般是几十分钟.

```shell
# Linux
[whisper@starry-sky ~]$ arp -a
_gateway (10.0.12.1) at fe:ee:5e:4d:d7:bb [ether] on eth0

# Windows
PS C:\Users\21066> arp -a

接口: 172.20.203.47 --- 0x4
  Internet 地址         物理地址              类型
  172.20.202.27         5e-01-76-f2-d9-77     动态
  172.20.202.36         10-5f-ad-33-af-c7     动态
  172.20.202.112        04-8c-9a-e7-d4-92     动态
  172.20.202.133        10-6f-d9-66-04-37     动态
  172.20.202.251        7a-38-26-83-db-2f     动态
  172.20.203.106        ae-4f-6a-01-54-64     动态
  172.20.203.107        1c-ce-51-c9-8b-47     动态
  172.20.203.137        1e-b6-be-0e-85-2a     动态
  172.20.203.167        b4-b6-86-53-32-f4     动态
  172.20.203.169        32-a4-d3-f5-4e-fe     动态
  172.20.203.254        10-51-72-0f-f8-0a     动态
  172.20.203.255        ff-ff-ff-ff-ff-ff     静态
  224.0.0.2             01-00-5e-00-00-02     静态
  224.0.0.22            01-00-5e-00-00-16     静态
  224.0.0.251           01-00-5e-00-00-fb     静态
  224.0.0.252           01-00-5e-00-00-fc     静态
  224.0.0.253           01-00-5e-00-00-fd     静态
  238.238.238.238       01-00-5e-6e-ee-ee     静态
  239.192.152.143       01-00-5e-40-98-8f     静态
  239.255.255.250       01-00-5e-7f-ff-fa     静态
  255.255.255.255       ff-ff-ff-ff-ff-ff     静态

接口: 192.168.248.1 --- 0xd
  Internet 地址         物理地址              类型
  192.168.248.254       00-50-56-fe-b5-c9     动态
  192.168.248.255       ff-ff-ff-ff-ff-ff     静态
  224.0.0.2             01-00-5e-00-00-02     静态
  224.0.0.22            01-00-5e-00-00-16     静态
  224.0.0.251           01-00-5e-00-00-fb     静态
  224.0.0.252           01-00-5e-00-00-fc     静态
  224.0.0.253           01-00-5e-00-00-fd     静态
  238.238.238.238       01-00-5e-6e-ee-ee     静态
  239.192.152.143       01-00-5e-40-98-8f     静态
  239.255.255.250       01-00-5e-7f-ff-fa     静态
  255.255.255.255       ff-ff-ff-ff-ff-ff     静态

接口: 192.168.150.1 --- 0x14
  Internet 地址         物理地址              类型
  192.168.150.254       00-50-56-fb-9b-e2     动态
  192.168.150.255       ff-ff-ff-ff-ff-ff     静态
  224.0.0.2             01-00-5e-00-00-02     静态
  224.0.0.22            01-00-5e-00-00-16     静态
  224.0.0.251           01-00-5e-00-00-fb     静态
  224.0.0.252           01-00-5e-00-00-fc     静态
  224.0.0.253           01-00-5e-00-00-fd     静态
  238.238.238.238       01-00-5e-6e-ee-ee     静态
  239.192.152.143       01-00-5e-40-98-8f     静态
  239.255.255.250       01-00-5e-7f-ff-fa     静态
  255.255.255.255       ff-ff-ff-ff-ff-ff     静态
```

Mac地址是依据硬件而定的, 所以容易因为硬件的改变而发生变化, 因此为了提高效率, 它会缓存一会儿, 但也只是一会儿. 缓存失效后, 会重新进行`ARP`, 并且, 如果有多个`ARP`报文, 会采用最新的, 对请求和应答都是这样.    `ARP`在局域网直接相连的任意两台主机都可以做, 这样就解决了局域网中主机之间Mac地址的寻址问题, 从而让它们可以相互通信.

`ARP`这种采用最新的策略, 本意可能是为了适应硬件网卡的更新, 但也会引发一些安全问题, 比如现在局域网中有很多主机, 我们把路由器记为`R`, 把其中的两台普通主机记为`A, B`, 由于`ARP`的请求采用广播形式, 所以当`A`请求`R`的Mac时, `B`也会收到, 并且可以获取`A`的Mac和ip, 这样, `B`可以对`A`发送多个`ARP`应答, 覆盖掉原来`R`的应答, 此时`A`就会认为`B`是路由器, 就会把报文转交给`B`, 如果`B`丢弃这些报文, 就相当于把`A`给断网了, 这种网络攻击称之为"ARP欺骗", 如果`B`不丢弃, 而转发给`R`, `R`就会认为请求来源是`B`, 所以不管收到什么, 都会转交给`B`, 然后`B`再转交给`A`, 此时`B`就成了中间人, 这是基于`ARP`的成为中间人方式,    或者, `B`可以在`A`广播之后, 一边对`R`发送多份`ARP`请求, 这些请求中ip仍是A的ip, 但Mac是B的Mac, 另一边, 再对`A`发送多份`ARP`响应, 这些响应中ip是R的ip, 但Mac是B的Mac, 此时, B的透明度就更高了, 实现了双向欺骗.

网络上是有ARP欺骗工具的, 可以进行相关的实验, 看一下效果. 推荐使用自己的主机做路由器, 进行小规模欺骗.

顺口提一下, `RARP`是用Mac找ip

## 其它协议或技术

### DNS

之前说过, 我们日常访问某个网站时, 用的并不是`ip`, 而是域名, DNS就是一种从域名映射到ip的技术.

比如我们可以`ping`一下, 拿到某个网站的`ip`, 然后用这个ip访问它, 不过出于安全考虑,, 不一定真能连上.

```shell
PS C:\Users\21066> ping www.baidu.com

正在 Ping www.a.shifen.com [36.152.44.132] 具有 32 字节的数据:
来自 36.152.44.132 的回复: 字节=32 时间=16ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=18ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=15ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=18ms TTL=52

36.152.44.132 的 Ping 统计信息:
    数据包: 已发送 = 4，已接收 = 4，丢失 = 0 (0% 丢失)，
往返行程的估计时间(以毫秒为单位):
    最短 = 15ms，最长 = 18ms，平均 = 16ms
PS C:\Users\21066>
```

就现在来说, 域名解析大概是这样的, 当用户在浏览器里输入域名后, 浏览器会先向DNS服务商进行询问, 得到对应的ip, 然后再用这个ip对网站进行访问. 另外, 系统会内置一份`hosts`文件, 里面可能会记录一些域名与ip的映射关系, 为什么说是可能呢? 因为现在的过程都是先看一下本地的`hosts`文件, 本地找不到映射关系后再去问DNS服务商(就是本地优先级高于DNS服务商), 但询问到的结果并不会存到`hosts`文件里, 所以`hosts`可能一直都很干净. 所以现在主要起作用的还是DNS服务商运行, 本地`hosts`只用于自己的个性化配置.

以前可能主要是用`hosts`的, 但所有电脑的文件同步很麻烦, 所以后来都不用了, DNS服务商统一进行域名管理, 这样就不会有什么文件太老, 映射关系变了, 服务端改域名ip, 用户不能及时更新之类的问题.

域名解析其实很重要的, 但一般来说都不需要我们直接操心.

域名分为两个部分, 比如对于`www.baidu.com  `, `com`是一级域名, 表示这是一个商业性质的企业域名, 除此之外, 还有`net`(网络服务商), `org`(非盈利组织), 等其它同级的域名, `baidu`是二级域名, 表示具体的公司, 组织.  `www`是习惯性的书写, 完全可以不写.

### 简述网络过程

这其实是道经典的面试题, 一般是从用户输入`url`开始, 然后让你回答之后会发生什么.很考验对于网络的理解 对于这类问题, 可以分三个步骤: 

- 先说http(把整个网络协议栈穿透一遍)
- 说一下https(加密原理)
- 其它细节(与面试官问答的形式进行)

其实, 本文完全可以回答这个问题, 到目前为止, 本文已经有十一万一千多词, 后面我们还有多路转接, 所以这里先略过, 复习的时候我们再回来写. 对于我们来说, 要做的是缩句不是扩句.

### ICMP协议

首先需要说明的是, 这是一个网络层协议, 用的是原始套接字, 有些面试官会故意套人, 问ICMP协议或者基于ICMP协议的`ping`指令用的是什么端口, 要注意的是, 这是网络层, 没有端口的说法.

我们知道, ip协议并不保障可靠性, 所以在新搭建好一个网络后, 无法使用ip协议进行网络测试, 因为ip报文传送失败什么都不会有, 没有反馈就不好判断网络状态, ICMP正是为此而生的, ICMP在逻辑上就像是IP基础上加了一个错误应答机制, 或者说是在IP协议之上但仍在网络层的协议, `ping`用的就是这个协议

![image-20250427163443289](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250427163443404.png)

ICMP先构造一份报文, 然后交给IP协议, 之后就是贯穿协议栈, 来到目标主机, 或者传送失败的那个主机, 再回应一个ICMP应答

![image-20250427163717300](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250427163717370.png)

![image-20250427163840960](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250427163841070.png)

不同的类型映射不同的反馈情况. 让我们再回来看一下`ping`

```shell
PS C:\Users\21066> ping www.baidu.com

正在 Ping www.a.shifen.com [36.152.44.132] 具有 32 字节的数据:
来自 36.152.44.132 的回复: 字节=32 时间=16ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=18ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=15ms TTL=52
来自 36.152.44.132 的回复: 字节=32 时间=18ms TTL=52

36.152.44.132 的 Ping 统计信息:
    数据包: 已发送 = 4，已接收 = 4，丢失 = 0 (0% 丢失)，
往返行程的估计时间(以毫秒为单位):
    最短 = 15ms，最长 = 18ms，平均 = 16ms
PS C:\Users\21066>
```

里面记录了响应时间, 最小生存时间`TTL`之类的信息, 它这个`TTL`是一个个试出来的, 就是让`ip`报文的`TTL`从`1`开始, 一个一个试.

基于ICMP的指令还有`traceroute`, 可以打印整个传送过程经过的路由器

```shell
[wind@starry-sky ~]$ # Ubuntu 安装指令 sudo apt install traceroute
[wind@starry-sky ~]$ traceroute baidu.com
traceroute to baidu.com (110.242.68.66), 30 hops max, 60 byte packets
 1  10.12.168.166 (10.12.168.166)  0.448 ms 10.12.208.166 (10.12.208.166)  0.524 ms  0.538 ms
 2  * 11.73.35.118 (11.73.35.118)  5.800 ms 10.12.176.65 (10.12.176.65)  6.750 ms
 3  11.223.161.129 (11.223.161.129)  0.549 ms 11.223.161.137 (11.223.161.137)  0.484 ms 10.255.101.125 (10.255.101.125)  14.729 ms
 4  11.94.128.126 (11.94.128.126)  2.197 ms 11.94.129.26 (11.94.129.26)  11.838 ms 11.94.128.138 (11.94.128.138)  1.713 ms
 5  10.102.42.57 (10.102.42.57)  2.516 ms 10.102.41.161 (10.102.41.161)  2.330 ms 10.102.41.101 (10.102.41.101)  2.317 ms
 6  124.160.190.225 (124.160.190.225)  2.571 ms  2.431 ms  2.435 ms
 7  124.160.83.1 (124.160.83.1)  4.082 ms 124.160.189.101 (124.160.189.101)  2.815 ms 124.160.83.81 (124.160.83.81)  2.767 ms
 8  * * *
 9  110.242.66.178 (110.242.66.178)  26.178 ms  25.724 ms 110.242.66.190 (110.242.66.190)  28.109 ms
10  221.194.45.134 (221.194.45.134)  27.246 ms  25.343 ms  25.311 ms
11  * * *
12  * * *
13  * * *
14  * * *
15  * * *
16  * * *
17  * * *
18  * * *
19  * * *
20  * * *
21  * * *
22  * * *
23  * * *
24  * * *
25  * * *
26  * * *
27  * * *
28  * * *
29  * * *
30  * * *
[wind@starry-sky ~]$ 
```

### NAT

之前我们已经说过NAT怎么出去了, 现在我们说说怎么回来.

我们知道, 现在的IPv4地址已经严重不足了, 为了缓解这个问题, 对于网络末梢来说, 其实是一个个局域网, 这些局域网里的主机用的都是私有ip, 因为不同的局域网中可以出现相同的私有ip, 所以就相当于同一个ip被人同时使用, 从而起到了缓解作用.

但私有ip的这个可重复特性也为网络通信带来了新的问题.因为私有ip可以重复, 丧失了ip原有的唯一性, 所以一个使用私有ip的用户不能和使用公网或者说使用公网ip的服务端入口路由器直接通信. 

之前我们也说过, 在用户请求报文往外发送时, 每经过一个路由器, 该路由器的ip层都会对ip报文的源ip进行修改, 改成自己的, 确保响应报文来到下一跳后能够找回来, 然后再发给下一跳, 这样最终, 在出运营商的入口路由器之后, 报文中的ip就变成了运营商入口路由器的公网ip.

![image-20250424193302113](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250424193302227.png)

![image-20250427171332897](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250427171333029.png)

等到回来的时候, 首先因为最后的ip请求报文的源ip是运营商入口路由器的公网ip, 所以响应报文就会来到运营商入口路由器.

现在我们要说的是, 每级路由器再替换用户请求ip的过程中, 并不是仅仅只替换了一个ip, 还有可能替换端口, 并自己维持一个原先源ip端口和修改后的源ip端口的映射结构.

![image-20250427172423595](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250427172423727.png)

在上面的这张图中, 我们省略了其它中间路由器, 用户和运营商入口路由器直接相连, 用户发送的请求报文, 最开始`src: 10.0.0.10:1025  dst: 163.221.120.9:80`, 再经过入口路由器后就被改成了`src: 202.244.174.37:1025  dst: 163.221.120.9:80`, 并维护着对应的NAT转换表.  在回来的时候,  它就可以通过端口的不同找回原先的用户ip端口或者它下一级路由器的ip端口, 每一级都是这样, 最后就可以一级一级地回来.

NAT也是有缺陷的, 一是它太过关键, 容错率比较低, 出错会造成很大范围的影响, 另一方面, 外面的公网主机是不能直接连接私网主机的, 只能私网主机先主动通信, 才能连接.

现在我们看看更复杂的场景, 两个私有设备如何通信, 比如像QQ这样的社交软件. 用户用的都是私网ip, 这怎么通信呢? 其实也不难, QQ启动时会自动连接腾讯的服务器, 腾讯服务器会把之前别人发给你的消息存起来, 你一登录QQ, 因为它会自动连接, 所以就能把数据发回来. 此时腾讯的这台服务器就相当于一个代理服务器.并且一般来说, 我们的电子设备会一直联网, 所以也能做到即时通信, 可能客户端每过一次都会自动连接服务端, 确认一下情况, 或者干脆使用一个长连接, 一直连着.

这种让两台使用私有ip的主机相互通信的技术就叫做内网穿透, 都需要借助一个使用公网ip的中间主机进行中继. 内网穿透也有对应的工具, 比如`frp`, 分为中间主机装的`frps`和私有ip主机装的`frpc`, 

代理服务器也不是一定要连公网, 比如对于某个学校来说, 它有自己的校园网, 为了对校园网进行管理, 学校就会专门的学校服务器, 当你连的是校园网时, 会首先弹出一个界面, 让你进行登录认证(实际上是对你进行私有ip分配).校园网中的所有请求, 都会先汇总到学校服务器那里, 然后学校服务器再进行一下NAT转换, 改成它自己的私有ip, 然后再发给上一级. 我们把这种服务器, 这种汇总用户请求的服务器, 或者说代理用户请求的服务器, 叫做正向代理服务器.

有时, 学校的服务器会处于安全考虑屏蔽掉某些请求, 比如ssh. 所以有时候用校园网连不上云服务器.  另外, 学校有时候也会把网络上的东西拷下来, 可能因为比较常用, 此时只借助于校园网就可以查看这些资源, 学校服务器的安全策略具有对内的, 也有对外的, 比如做内网穿透, 学校的服务器可能会直接拦截从其他地方来的请求.

另外还有反向代理服务器, 对于提供各种网络服务的那些公司来说, 它会有自己的机房, 里面有很多机器, 公司不会把这些机器全部连公网, 这样会造成一种情况, 一方面, 太多机器暴露出去了, 不安全, 也不好管理, 另一方面, 可能用户总连接那几台机器, 别的机器没人连, 有的机器都很忙, 有的机器却很闲, 效率就会很低, 所以公司会准备一个配置很高的服务器, 用户一律都是连这个服务器, 然后这个服务器再把请求均匀的分配给机房中的其他机器.  它自己作为双方通信的中继服务器, 或者如果机房里面的机器连的都是公网, 还可以直接让用户重定向.   反向代理服务器一般用来实现流量的负载均衡.

`frp`中的那个中继服务器是反向代理服务器, 服务端都是私网中的那个主机, 客户端可以是任何的主机, 连私网公网都可以.   像是一个简易的公司网络架构.

有时, 我们可以在具有特殊政策的地区, 比如香港, 租赁一台服务器, 当用户想访问谷歌这种某些境外网站时, 如果正常地通过运营商, 运营商就可以通过目的ip知道你要访问谷歌, 从而丢弃你的报文, 此时, 我们可以把报文先发给香港的那个代理服务器, 香港因为政策特殊, 所以可以直接连接外网, 这样就绕过了原先的运营商, 这样香港的那台代理服务器就可以替你访问谷歌, 并把响应再转发给你. 不过运营商也有对策, 它可能通过某些手段, 比如和特别地区的云服务器厂商合作, 或者自己感觉到流量异常, 从而也屏蔽掉香港的那台服务器的ip, 这样你就连不上香港的那台机器了. 也有对应的工具, 叫做`socks5`. B站似乎有相关的知识, 可以从中学习一下.

一般来说, 有专门的人士提供这种服务.

代理服务器的具体工作模式可以理解为隧道技术, 就目前来说, 我们可以认为隧道技术就是把真实使用的应用层报文包在另一个应用层报文里, 利用应用层的加密服务, 对原先报文的访问意图进行隐藏或者保护的一种技术.

![image-20250428203153297](https://raw.githubusercontent.com/ListenStarsWind/images/master/2025/20250428203153556.png)

将就看吧, 不是学美术的.

我们之前说过, 我们现在用的都是域名, 将域名解析成ip需要DNS运营商的支持, 当你访问`google.com`时, DNS服务商会拒绝应答, 不知道ip, 无法访问.
如果你知道ip, 运营商看到你的目的ip是谷歌, 直接舍弃报文, 访问不了
可以从某些渠道, 拿到某种软件, 它会在本地拦截我们对于某些域名的DNS服务报文, 然后它自己就有对应的ip, 就将DNS应答成功返回到浏览器, 浏览器获得谷歌的ip, 构造一个应用层请求报文, 那个软件将请求报文拦截, 把它作为负载包进了另一个看起来没什么特别的又一个应用层报文, 并加密, 原先的谷歌请求报文被加密, 看不到, 这个软件将这份"伪装报文"转交给运营商, 运营商通过目的ip, 发现是国内的一些区域, 看起来正常, 内容加密了它看不到, 转发到某些特别区域, 特别区域的服务器将伪装报文解包, 把原先的那个谷歌请求报文拿出来, 发给运营商, 这些特别区域有特殊政策, 运营商不拦截, 转发到国际网上, 谷歌服务器收到, 构建应答报文, 运营商收到, 目的ip是特殊区域, 不拦截, 特殊地区服务器收到应答报文, 将其放到另一个应用层报文的负载里, 加密, 转发给运营商, 运营商查看源ip, 发现是内地的, 进行转发, 本地获得伪装报文, 本地代理软件解包, 把谷歌应答拿出来, 交给浏览器, 本地访问到谷歌.

具体用什么应用层协议无所谓, 反正大家都有对应的加密层.

下面聊聊NAT和代理服务器的区别

从目的上来看, 代理服务器解决应用层的某些特别需求, NAT解决ip地址不足
从协议栈角度来看, NAT主要在网络层, 代理服务器则在应用层,
从使用范围来说, NAT在局域网的出口(不同层次的网络之间)部署, 代理服务器广域网, 局域网, 都有


# 完 