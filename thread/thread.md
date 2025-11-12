# thread

## 引言

在说线程的概念之前, 我们先简略回顾一下计算机的发展过程, 计算机本质上是为了满足我们的各种需求而存在的, 如何满足? 计算机具体来说是通过各种各样的程序来满足人们的需求的, 在最开始, 计算机是顺序执行程序的, 此时, 就出现一个问题,  就是计算机的资源利用率太低,  计算机使用的是电磁信号, 而我们使用的是化学信号, 这必然会导致一种情况, 就是用户的速度跟不上计算机的速度,  这意味着, 计算机需要频繁等待用户,  在等待的过程中, 计算机就只能空转,  这样就使得它的资源利用率过低.

对于用户来说, 他当然更希望能尽可能榨干计算机的资源, 毕竟, 即使单就人类群体本身来说, 一些人也总是希望榨干另一些人的资源,  计算机又和人类不是一个物种,  因此我们更要榨干它们. 

怎么榨呢?   很简单,   让计算可以并发运行多个程序, 这个程序在等待用户交互, 那就去运行另一个程序, 总之, 要尽可能让计算机时时刻刻都有活干,  我们称之为"进程",  而且进程之间还可以通过特殊渠道进行相互通信,  它们之间相互合作,  还能完成更加复杂的功能.(写到这里, 感觉和工业发展有些像, 最开始家庭工坊, 后来小型手工业工厂,  功能特化,  协同合作....)

当然, 进程也有缺点, 为了将多个进程进行管理, 我们需要将它们进行层层抽象,  变成一个又一个数据结构,  还要把这些数据结构有机地整合起来, 又为了保证进程间的独立性, 每个进程都需要有独立的一堆数据结构, 这样就造成一种困境:  为了满足用户需求,  我们需要创建进程,  为了管理进程,  计算机需要做一堆准备工作,   有时,  我们的需求其实很简单,   但为了创建满足需求的进程,  计算机需要进行很多额外工作,  这就又降低了计算机的工作效率.

怎么办呢?   我们还是用老办法, 再加一个中间层,  我们把进程的执行逻辑进行拆分, 把较为独立的执行分支变成一个个小进程,  由于这些小进程本质上是在进程的基础上运行的  所以它可以直接使用进程原先的绝大部分数据结构,  这样就不用创建绝大多数新的进程内核数据, 这样的话, 当某个进程在运行时, 如果这个小进程在阻塞, 那就可以换成另一个小进程,  而不必创建新的进程.  我们把这些小进程称之为"线程".   ("小进程"这种说法只是为了好理解而已, 但不能真的这么说)

线程还有一些优点,   之前, 为了保证进程的独立性,  我们只能借助于特定的方式去进行进程间通信,  这些特定方式也会影响工作效率,  但线程的通信就较为简单, 因为线程本质上是进程的一部分, 所以它们之间若要进行通信, 就不需要太多弯弯绕绕,  直接通信即可.   当然, 这也是有代价的, 这导致线程之间没有独立性, 一个线程崩溃会导致整个进程崩溃,  而且由于线程间有许多共享资源,  在它们并发执行的时候,  就容易发生资源竞争的问题,  这导致多线程的代码要有许多额外考虑, 因此代码难度也就更高. 

对于CPU来说, 线程和进程其实都是执行流, 或者这样说, CPU不在乎自己调度的究竟是线程还是进程, 在CPU的眼中, 线程和进程都是一堆代码和数据, 它们的代码描述如何处理数据, 从而达到什么样的效果. 而又由于线程是进程的向下细分, 所以我们可以说, "线程是操作系统调度的基本单位".  既然说了线程是什么东西, 那我们再说说进程是什么东西吧, 以前我们说, 进程是很多东西的集合,  比如进程控制块, 页表, 进程地址空间....这些内核数据, 还有满足实际需求的那些代码和数据,  也包含物理内存或其它物理结构实体.   今天, 我们可以从另一个角度来定义进程,  从内核的角度上来说, 我们可以认为"进程是承担分配系统资源的基本实体", 做个形象的比喻, 我们可以把整个计算机比作一个社会, 那么进程在其中就扮演着家庭的角色,  家庭从社会中获得各种资源, 比如房产, 家电,  教育资源,  医疗资源等, 家庭依据某些目的利用这些社会资源从而达到某种效果, 家庭内部则对资源进行更细的划分和分配, 让资源用到对的地方, 从而增大资源的利用率,  每个家庭成员则相当于线程,  他们从家庭那里获得资源, 有着各自的小目标, 这些小目标共同发挥作用, 让整个家庭朝着某一特定方向行动.    进程的创建就是为它分配相应的资源, 进程的销毁就是剥夺它的所有资源, 家庭亦是如此当然, 你也可以不使用家庭, 而使用其它的社会组织形式,  照着这个方向来说, 计算机那就是乌托邦呀,  资源分配公平,  每个人都知道自己的长处,  有着明确的方向, 人们的精神和物质需求都能得到满足, 社会运行井然有序, 有着完善的制度或规则.  可惜, 现实没有乌托邦.            线程是执行流资源, 因此, 对于进程来说, 线程是进程的一部分. 

不同的操作系统对于线程的具体实现有不同的方式, 对于我们的Linux来说, 由于线程和进程在行为上有很多相通之处,  也有各种各样的队列,  也需要被系统进行管理, 线程也有切换....或者这样说, 线程的系统代码和进程的系统代码很像,  所以, Linux对于线程的描述实际上用的是进程的数据结构, Linux复用了之前进程的代码.  比如说,   尽管线程绝大多数是共用进程的那些内核数据, 但还是要有所区分,  Linux对于线程的抽象描述, 其实上, 用的也是`task_struct`,  进程有自己的`task_struct`,  线程也有自己的`task_struct`, 因此有人说:"Linux没有真正意义上的线程".       Windows就不是这样,  Windows实实在在的为线程实现了专属的相关结构, 我们不评价这两种线程具体实现方法的优劣,  只能说这两种系统的需求不同,  Linux主要作服务器,  要快, 要高可靠性,  要好维护, Windows可能有其它方面的考虑.   当然, 我们也可以认为"Linux也有真正意思上的线程",  前一种说法, 可能更主要的是说线程的形式,  后一种说法, 更主要说的是线程的内在, 只是看待事物的角度不同罢了. 人类的语言还是太贫乏了, 往往无法充分描述这个世界,  世界究竟如何, 还是要靠自己的感受.

由于Linux中的执行流既可以是进程, 也可以是线程, 所以Linux中的执行流有时会被叫做"轻量级进程",  这个"轻量级进程", 可以说是进程, 也可以说是线程, 是一种泛称, 要看具体情况.  你也可以说, Linux既没有进程, 也没有线程, 只有轻量级进程, 还是那句话, 只是角度不同, 不用过于纠结.

----------------

下面我们说说进程是如何把自己内部的资源分配给线程的，其实很简单，但在此之前，我们先去看看虚拟地址被映射到物理地址的实际过程。

这就要谈到页表了。虽然页表带一个“表”字，但实际上，页表并不是一张完整的“表”，而更像是一棵“树”，只是树上的每个节点都是一张表。诚然，如果直接用一张表来做映射，页表会非常简单——把虚拟地址当数组下标，直接索引物理地址——但这种数组方案有一个致命的缺点：空间太大。

我们以 32 位操作系统为例。对于 32 位系统，理论寻址范围是 0 到 2^32−1，共有 2^32 个地址。若把虚拟地址全部展开为一张数组，就需要 2^32 个元素；每个元素为一个地址（32 位，即 4 字节），那么数组大小就是 2^32 × 4 字节，甚至超过了系统能直接使用的内存，这显然不可行。

因此，页表采用了“树结构”，每个节点都是一张项数为 2^10 的表（如下图所示）：

![无标题](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/%E6%97%A0%E6%A0%87%E9%A2%98.png)

具体做法是把虚拟地址从高位到低位拆成三份：长度分别为 10、10、12。它们分别能表示 2^10、2^10、2^12 种状态。10 位的二进制序列足以定位单个节点中的某一项：前 10 位定位根节点中的某个元素，该元素指向第二层的一个节点；中间 10 位再定位第二层节点中的具体项。这样，两层结构合起来可以表示 2^20 个不同的页框（page frame）。

把 2^32 字节的地址空间分成 2^20 个块，每块大小是 2^32 / 2^20 = 2^12 字节，也就是 4 KB——这恰好是常见的页框大小。也就是说，仅靠这两层树结构，就能定位到一个 4 KB 的页框。

在这个比喻里，树第二层中每个节点的每个元素我们用“20 长度”的二进制序列来强调它所寻址到的页框（注意：为了强调寻址页框，这里着重说了 20 位的含义）。除此之外，为了记录权限等信息，每个表项通常按 32 位来存放（含控制位与标志）。每个节点有 2^10 项，所以每个节点大小约为 2^10 × 4 字节 = 4 KB，刚好占一个页框；根节点与第二层的节点在布局上也因此实现了统一。

接着看整个第二层的总规模：不计根节点，仅看第二层的所有节点，每个 4 KB、共有 2^10 张，这些加起来大约是 4 × 2^10 KB，也就是约 4 MB。相比起把整个虚拟空间展开成数组，这已经是一个数量级的下降。

实际情况里，页表很少会真的满。操作系统通常采用惰性分配：有些虚拟地址在进程地址空间上存在，但对应的数据可能还在磁盘（或文件）里，并未加载进内存；相应的页表项也就没有分配或填充。如果进程访问到这些未分配的项，就会触发缺页中断或写时拷贝等机制，由内核把缺失的页表/页框补上。

再说一下与 CPU 的关系：`cr3` 指向第一层的根节点（即页表树的顶层），当发生缺页或相关异常时，导致异常的虚拟地址会被保存到 `cr2`（`cr2` 保存导致最近一次页异常的线性地址），内核据此完成缺页处理并在处理完毕后恢复访问。同时要略微提一下性能方面的一个关键点：页表查找的结果通常会被缓存在 TLB（Translation Lookaside Buffer）里，以避免每次访问都走完整个多级查找。

最后说明一条术语约定：那个根节点我们这里称作“页全局目录（Page Global Directory）”，第二层中的节点称作“页目录（Page Directory）”。（注：不同上下文或不同架构时，术语可能有细微差别，但在本文的讨论范围内按此约定即可。）

当然，我们也知道页表的细节（比如层数、每层位宽、页表项大小等）可以根据体系结构或设置进行调整，但根本道理是相同的：通过层级化的索引把虚拟地址逐级映射到物理页框，从而节省空间并支持按需分配。

下面那段话是我以前写的，说实话，我自己重新看都头疼，所以这里我又重写了一下，里面的一些计算可能也是错的，这里留在这里是给我自己看的，你们就不要看了。

~~这就要谈到页表了, 虽然, 页表带一个"表"字, 但其实上, 页表并不是表, 而是树状结构, 为什么呢?  因为单纯的表太浪费空间了, 我们拿32位操作系统为例,  32位系统理论寻址范围是$0$到$2^{32} - 1$, 这意味着页表要有$2^{32}$种状态, 这样在理论上才可以做到有效寻址, 如果使用表结构就相当是$2^{32}$个$1$连加得到$2^{32}$种状态, 我们都不需要计算就知道这种页表是不可行的, 内存一共只有$2^{32}$字节, 而这种页表有$2^{32}$个项, 每项起码有两个地址, 一个是虚拟地址, 另一个是物理地址, 每个地址$4$字节, 再加上权限标志位, 每项都大于$8$字节, 所以这种页表甚至是整个内存都放不下.   所以我们的页表实际上是树状结构, 树状结构就相当于连乘, 一般情况下, 它是按照$10 * 10 * 12$的方式组织的, 虚拟地址有32比特, 我们可以把这32比特拆成三个部分, 从左往右或者说从最高位到最低位, 我们把前`0`到`9`位分成第一组, 把`10`到`19`分成第二组, 把`20`到`31`分成第三组,  这样的话, 第一组和第二组虚拟地址就可以表示出$2^{10}$种可能, 第三组虚拟地址就可以表示出$2^{12}$种可能, 而对于页表来说, 它可以被看成深度为三的树状结构, 我们一般把第一层树状结构称之为"页全局目录", 页全局目录有$2^{10}$个项, 对应着第一组虚拟地址的$2^{10}$种可能, 页全局目录中的每一项都指向着"页目录", 也就是第二层树状结构, 每个"页目录"同样可以有$2^{10}$个项, "页全局目录"和"页目录"相互配合, 理论上就能表示出$2^{20}$种可能,  这$2^{20}$又意味着什么呢? 内存一共有$2^{32}$字节, 把内存分成$2^{20}$份, 每份的大小就是$2^{12}$字节, 或者说, 4 KB, 而页框的大小一般来说就是4 KB, 也就是说, 仅依靠页全局目录和页目录就可以定位内存中的任何一个页框, 而剩下的第三组虚拟地址可以表示出$2^{12}$种可能, 从而指明目标地址在页框中的具体偏移量, 页目录的每一项都对应着一个"页表", 也就是说, 每个页框都有自己的页表, 来描述该页框的对应附属信息,  具体什么信息就不说了, 种类有很多, 我们在这里不展开, 现在, 我们来计算一下这种页表理论最大值, 首先, 页全局目录和页目录最大有$2^{20}$个地址, 页框也有$2^{20}$个, 每个页框对应一个页表, 一个页表大概是四字节, 所以, 最大的页表是$2^{20} * 12 $字节, 或者说12 MB, 对于内核数据结构来说, 看起来也挺大的, 但实际上, 页表不会达到12 MB, 如果一个进程的页表已经达到了12 MB, 这意味着该进程已经把整个内存用完了, 我们这里说的页表是会有虚拟地址和物理地址的页表, 也就是用户级页表, 内核数据不使用虚拟地址, 它们如果有页表的话, 那肯定是另外的形式, 也就是说, 当页表为12 MB的时候, 即使单论用户空间来说, 就已经占满内存, 那系统在哪里运行呢? 没有系统, 进程又怎么运行呢? 所以说, 这种情况是不可能发生的.      在实际情况下, CPU中的`cr3`寄存器实际指向的是页全局目录, 页全局目录是一定完整拥有$2^{10}$个项的, 每个项都指向一个页目录, 但页目录可能是残缺的, 当虚拟地址指向页目录下一个不存在的子项时, 就可能意味着这个虚拟地址是越界的, 至于写时拷贝, 缺页中断之类的东西, 可能是借助于页表中的某些标志位来实现的.   这就是教科书喜欢说的"x86处理器段地址加偏移量的寻址方式 ".   对于那些引发缺页中断的虚拟地址来说, 会被暂存在`cr2`寄存器中, 等到做好准备之后再重新进行访问. 用户空间使用的都是虚拟地址, 所以我们不需要考虑物理地址怎么转成虚拟地址, 进程地址空间中有各种各样的分区, 相同权限的数据都会放在一块, 所以一个页表对应一个页框是完全够用的, 不用进行更细的划分.~~       

我们知道, 我们使用的地址都是数据起始地址, 在高级语言转为汇编的过程中, 会依据数据的具体类型以该地址为起始读取相应的大小, 可能会有人问超过页框大小的数据是如何进行访问的, 对于这种数据, 一般来说, 是很大的对象, 我们需要认识到的事实是, CPU运行的是汇编代码, 汇编并没有所谓的"面向对象"说法, "类和对象"实际上还是有各种内置类型以各种方式组织形成的, 而在汇编层面上, 原先的类和对象被拆分成基本的内置数据, 它并不是一次性访问整个对象, 而是依据具体需求对对象中的基本数据进行访问, 所以不用担心这种问题. 

画的不太好看, 但我觉得意思是表达够的.

![image-20250218215225706](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250218215225836.png)

其实上面的这些和线程都没什么关系,  之前我们在进程地址空间哪里没有细说具体的转化过程, 在这里给补上.

对于线程来说, 一般来说, 真正需要具体安排的资源就是代码区和栈区, 其它都是直接共用进程的, 其它资源都是全局性的, 不管在进程的哪里, 理论上都应该能被看到, 线程又是进程的一部分, 所以自然是直接公用的, 栈区我们后面再谈, 对于代码区的分配其实很简单, 对于我们C/C++来说, 代码被包含在特定的函数中, 所以想要线程执行某个代码, 只要函数地址交给线程就行了.

![image-20241214160114166](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20241214160114236.png)

当然, 可能会有一些复杂情况, 但这里仅仅是引言, 所以就不讲那么多了.

---------------------

我们一般说: "线程比进程更轻量化", 这到底是什么意思呢? 这就需要对线程和进程进行相互比较.

![image-20250219075157806](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250219075157931.png)

首先, 就线程的创建和销毁来看, 线程明显是要比进程更加轻量化的, 在上面我们已经说过, 对于线程来说, 真正要创建的其实就是`task_struct`, 其它的都可以直接共用进程原有的内核数据;

其次, 线程的切换更加轻量化. 很明显, 线程在切换的过程中, 不需要切换诸如地址空间, 页表之类的进程全局内核数据, 当然, 线程的上下文是切换的, 我们知道, 线程的上下文中保存的是当前时间节点线程的运行信息, 等到线程重新被CPU调度到的时候它需要这些信息帮助自己回到切换前的状态, 也就是说, 线程的上下文信息是局部的内核数据, 所以当然是需要进行切换的. 当然, 也不能一棍子打死, 我们知道CPU中各类寄存器的数据被称为执行流的上下文, 比如`cr3`寄存器指向的是页全局目录 它当然是不需要更换的, 在此基础上, 你可能会问, 执行流的切换实际上就是把CPU寄存器中该更换的那些地址进行更新,  跟数据结构本体比起来, 地址很明显就要小很多, 那这样的话, 线程的切换实际上就是比进程少换了几个地址, 似乎也没提升多少效率, 你说的即对, 也不对, 说对是因为在逻辑上说你说的完全没错, 说不对是因为你少考虑了一些东西, 这些少考虑的东西需要我们结合CPU的硬件结构.

我们知道, 冯诺依曼计算机结构采用的是分级缓存机制, 这个机制我们就不具体说了, 有外存, 内存, 但实际上CPU内部也是有缓存的, 我们把这个缓存叫做"cache", 我们可以在`shell`界面通过`cat /proc/cpuinfo`指令查询`cache`的大小.

![image-20250219084754394](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250219084754613.png)

我这里是多核CPU, 所以有多个`cache`, `cache`下面还可以作更细的划分, 在这里我们就不提了, 我们看到`cache`其实还是很大的, CPU会把频繁用到的数据或者是即将运行的代码加载到这里, 运用局部缓存原理, 提高数据命中率.  因为`cache`中的数据会被频繁访问, 所以我们有时会把这些数据称为"缓存的热数据".  对于进程的切换来说, `cache`可能只做小范围的更新, 而对于进程的切换来说, `cache`就需要大范围的更新.

当然, 我上面所说的线程切换是同一个进程下的线程切换, 不包括跨进程切换线程. 另外这里我们还需要特别提一下, 线程共用进程的资源, 其中一种资源就是时间片, 如果线程用完了时间片, 那就会切换到另一个进程上, 这种用完时间片的线程切换更应该叫做进程切换, 实际上, CPU正是通过时间片的有无来判断这究竟是同一个进程下的线程切换还是跨进程的线程切换, 从而采用不同的切换措施.

为了区分最开始的`task_struct`和后来创建的`task_struct`, 我们一般把最开始的那个叫做主线程, 把后来的叫做副线程, 后面的实操环节我们会具体实验.

---------------

线程有如下优点:

1. 创建一个新线程的代价要比创建一个新进程小得多
2. 与进程之间的切换相比，线程之间的切换需要操作系统做的工作要少很多
3. 线程占用的资源要比进程少很多
4. 能充分利用多处理器的可并行数量
5. 在等待慢速I/O操作结束的同时，程序可执行其他的计算任务
6. 计算密集型应用，为了能在多处理器系统上运行，将计算分解到多个线程中实现
7. I/O密集型应用，为了提高性能，将I/O操作重叠。线程可以同时等待不同的I/O操作。

一二三条我们就跳过了, 至于第四条, 因为线程是并行的, 所以可以充分利用多核处理器, 但实际上, 进程也是并行的, 也可以利用多核处理器, 但代价更高; 第五条我们在一开始就谈过了; 对于第六条来说, 虽然话是这么说, 但实际上, 对于计算密集型的进程, 要结合处理器的具体情况, 如果是单核的, 一个进程就行了, 再往下细分线程反而不合适, 因为线程虽然切换代价小, 但毕竟是有代价的, 单就一个进程的视角来看, 对于单核, 下面不应该细分线程, 这样进程内部就不需要进行执行流切换, 也就是说, 进程向下细分成线程的具体个数, 需要视实际的硬件情况来确定; 第七条还是运用并行特点.

缺点:

1. 性能损失
   一个很少被外部事件阻塞的计算密集型线程往往无法与共它线程共享同一个处理器。如果计算密集型线程的数量比可用的处理器多，那么可能会有较大的性能损失，这里的性能损失指的是增加了额外的同步和调度开销，而可用的资源不变。
2. 健壮性降低
   编写多线程需要更全面更深入的考虑，在一个多线程程序里，因时间分配上的细微偏差或者因共享了不该共享的变量而造成不良影响的可能性是很大的，换句话说线程之间是缺乏保护的。
3. 缺乏访问控制
   进程是访问控制的基本粒度，在一个线程中调用某些OS函数会对整个进程造成影响。
4. 编程难度提高
   编写与调试一个多线程程序比单线程程序困难得多

二三条实际上是一个东西. 因为缺乏访问控制, 所以健壮性降低. 怎么说呢? 因为线程是共用资源的, 它们之间没有很好的隔离, 当一个线程出错, 就会引发其他线程出错, 因此在写代码的时候, 就需要深入考虑各种情况, 对资源进行各种保护, 编程难度自然就高.

------------

线程究竟有哪些共享, 有哪些不共享呢? 具体来说, 包括以下几点是独立的而非共享的

- 线程ID
- 上下文
- 栈
- errno
- 信号屏蔽字
- 调度优先级

其中特别需要强调的是上下文和栈, 上下文独立意味着线程是独立的执行流, 栈独立意味着线程不会出现执行流错乱的问题, 当然, 这建立在代码对的情况下, 其它的并不是特别重要, 特别是面试的时候.

进程的多个线程共享 同一地址空间,因此Text Segment(代码段)、Data Segment(数据段)都是共享的, 如果定义一个函数或者全局变量, 那么在各个线程中都可以访问到他们. 除此之外,各线程还共享以下进程资源和环境:

- 文件描述符表
- 每种信号的处理方式(SIG_ IGN、SIG_DFL或者自定义的信号处理函数)
- 当前工作目录
- 用户id和组id  

重点是文件描述符表, 对网络很重要.

## 线程控制

下面我们说线程控制. 在说线程控制之前, 需要强调是的是, 对于Linux系统内核来说, 没有独立的线程概念, 而只有所谓的"轻量级进程"这种概念, 或者换种说法, Linux没有线程的系统接口, 而只有轻量级进程系统接口, 从上面的学习过程中, 我们知道, 轻量级进程和线程毕竟不是同一个概念, 那我们怎么在Linux使用线程呢?

为此, Linux的一些开发者, 在应用层上开发了一种结构, 该结构使用轻量级进程的系统接口为我们封装了线程的概念, 由于这种结构是以链接库的形式呈现给其它用户的, 所以我们把这种结构称之为"线程库".  我们就是使用"线程库"中的接口来实现对线程的控制的.

上面说过, 线程库是Linux的一些开发者设计的, 而不是官方设计的, 所以它属于第三方库, 所以我们在编译包含线程控制的程序时, 需要给编译器指明线程库.虽说线程库是第三方库, 但理论上来说, 是个Linux系统都会自带线程库, 所以不用担心库的安装问题.

我们先认识第一个接口, `pthread_create`, 它负责生成一个新的线程

![image-20250221091837587](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250221091837833.png)

```cpp
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);

Compile and link with -pthread.
```

先看第一个参数`thread`, 它是一个输出型参数, 输出的内容是线程的唯一标识符, 这样在含有多个线程的情况下, 我们就能对它们进行区分;  第二个参数`attr`, 它是一个输入型参数, 用来约束新线程的一些属性, 但在实际使用上, 我们一般直接置空, 置空的意思就是使用线程库自己的默认参数配置, 除非有特殊需求, 否则我们都直接传空指针.  至于第三个参数`void *(*start_routine) (void *)`, 我们也能看出来, 实际上是一个函数指针,  新线程创建出来肯定是要代码的, 这些代码就以函数指针的形式传给新线程供它们运行, 另外我们也可以看到, 这个函数的参数是`void*`, 返回值也是`void*`, 为什么都是`void*`, 因为线程库自身它并不清楚用户输入的线程行为函数到底是什么样的, `void*`是泛型指针, 泛型指针的好处是具有泛用性, 这样写的话, 就不会把线程行为函数(我自己的叫法) 的具体形式卡的太死, 从C语言的角度来看, `void*`可以是`int*, double*, const char*`之类, 除此之外还有结构体的指针, 对于C++来说, 还多了各种对象的指针, 至于返回值, 那就可以把真正我们需要的数据对象建立在堆上, 因为线程共享堆资源, 只把对象的指针通过返回值传出去, 当然, 这样空泛地说还是有些太抽象了, 我们之后会在代码上实际测试的; 第四个参数`arg`, 这个实际上是`start_routine`指向函数的参数, 直接说也不太好讲明白, 我们还是在代码上见.            之后我们再说返回值, `pthread_create`的返回值, 也就是那个`int`, 这个`int`是用来表示`pthread_create`的运行结果的, 和系统接口不同, 线程库是第三方库, 所以它的错误码用的不是像系统接口那样, 使用`errno`, 而是用返回值的方式来表示, 如果`pthread_create`返回零, 就证明`pthread_create`成功了, 如果不是零, 返回的就是一个错误码, 那就可以依据这个错误码去查错误原因.

另外, 它最后还说, 编译时要链接`pthread`.

```cpp
#include <pthread.h>   // 线程库的头文件
#include<iostream>
#include<unistd.h>

using namespace std;

void* thread_behavior(void* x)
{
    while(true)
    {
        cout << "副线程# pid:"<<getpid()<<endl;
        sleep(1);
    }
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, nullptr, thread_behavior, nullptr);
    while(true)
    {
        cout << "主线程# pid:"<<getpid()<<endl;
        sleep(1);
    }

    return 0;
}
```

`pthread_t`其实就是一个无符号长整型, 只不过重命名了, 对于`pthread_create`来说, 前两个参数几乎都是这样写的, 在新线程被创建之后, 它就会执行`thread_behavior`中的代码, 一开始, 我们还是简单点, 所以我们不打算使用`thread_behavior`的参数, 因此我们对第四个参数进行了置空处理.  在新线程被创建之后, 原先的那个进程就变成了主线程, 而新的线程其实就是副线程, 为了看的更明显一些, 我们让两个线程都死循环, 打印进程标识符, 因为线程本质上是进程的一部分, 所以理论上, 它们应该打印出相同的`pid`.

现在编译它们

```shell
[wind@starry-sky control]$ g++ main.cc -o a
/opt/rh/devtoolset-11/root/usr/libexec/gcc/x86_64-redhat-linux/11/ld: /tmp/ccVoSJLc.o: in function `main':
main.cc:(.text+0x67): undefined reference to `pthread_create'
collect2: error: ld returned 1 exit status
[wind@starry-sky control]$
```

我们发现, 编译报错了,是个链接错误 `g++`说, `pthread_create`是个未定义的引用, 什么叫未定义的引用, 意思就是说, 编译器能在`pthread.h`中找到`pthread_create`的函数声明, 所以编译器是认识它的, 知道它其实是个函数, 因此它没有说`pthread_create`是未定义的标识符,但是, 它说该函数是未被定义的, 也就是说它找不到该函数的具体实现, 为什么找不到呢? 因为函数的具体实现是在线程库里的, 但编译器不知道该用什么库, 线程库虽说是第三方库, 但可以说, 是个Linux系统都自带它, 既然都自带了, 自然把对应的头文件和动态库放在了应该放的位置, 也就是系统的默认头文件路径和库文件路径, 所以编译器可以找到它们, 但编译器不知道具体用什么库, 所以我们需要在选项中为`g++`特别指明应该链接的库名字叫什么(链接库的相关知识我们已经说过, 在此不作赘述)

```shell
[wind@starry-sky control]$ ls
main.cc  makefile
[wind@starry-sky control]$ g++ main.cc -lpthread -o a
[wind@starry-sky control]$ ls
a  main.cc  makefile
[wind@starry-sky control]$
```

 我们用`ldd`看一下程序的动态链接关系

```shell
[wind@starry-sky control]$ ldd a
	linux-vdso.so.1 =>  (0x00007fff54806000)
	libpthread.so.0 => /lib64/libpthread.so.0 (0x00007f2e745c0000)
	libstdc++.so.6 => /home/wind/.VimForCpp/vim/bundle/YCM.so/el7.x86_64/libstdc++.so.6 (0x00007f2e7423f000)
	libm.so.6 => /lib64/libm.so.6 (0x00007f2e73f3d000)
	libgcc_s.so.1 => /lib64/libgcc_s.so.1 (0x00007f2e73d27000)
	libc.so.6 => /lib64/libc.so.6 (0x00007f2e73959000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f2e747dc000)
[wind@starry-sky control]$ ls -l /lib64/libpthread.so.0
lrwxrwxrwx 1 root root 18 Jun 28  2024 /lib64/libpthread.so.0 -> libpthread-2.17.so
[wind@starry-sky control]$
```

下面我们运行一下

```bash
[wind@starry-sky control]$ ./a
主线程# pid:副线程# pid:2598225982

副线程# pid:主线程# pid:2598225982

主线程# pid:25982
副线程# pid:25982
主线程# pid:25982
副线程# pid:25982
主线程# pid:25982
副线程# pid:25982
主线程# pid:25982
副线程# pid:25982
主线程# pid:25982
副线程# pid:25982
主线程# pid:25982
副线程# pid:25982
主线程# pid:25982
副线程# pid:25982
主线程# pid:25982
副线程# pid:25982
主线程# pid:副线程# pid:2598225982

副线程# pid:25982
主线程# pid:25982
^C
[wind@starry-sky control]$
```

我们看到的是, 这两个线程死循环确实交替进行, 另外, 我们看到的是, 在有些时候, 它似乎输出比较混乱, 我们说, 尽管线程有自己独立的栈结构, 或者这样说, 以线程自己的视角来看, 它知道自己这一步应该干什么, 下一步应该干什么, 它的执行流是清晰的, 但是, 从线程调度的角度来看, 系统为了执行流的均衡执行, 它有自己一套的调度机制和时间片分配机制, 我们无法清楚, 哪个线程先运行, 哪个线程后运行, 运行到什么时候切换到另一个线程, 这些我们用户都是不清楚的, 而这两个线程共用一个显示屏, 所以就会引发资源冲突, 所以有时候混在一起, 这种现象也意味着, `cout`它们是不可以重入的, 我们先忽略这种资源冲突现象, 先熟悉线程的基本控制手段, 之后再解决.

我们再运行一下程序, 用`ps ajx`查看一下它们的信息

```shell
[wind@starry-sky control]$ 刚刚试了一下, a这个名字不太好, 干扰项太多了, 换一个^C
[wind@starry-sky control]$ mv a pthread
[wind@starry-sky control]$ ls
main.cc  makefile  pthread
[wind@starry-sky control]$ while :; do ps ajx | head -1 && ps ajx | grep pthread | grep -v grep ; sleep 1; done;
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18123 29849 29849 18123 pts/7    29849 Sl+   1002   0:00 ./pthread
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18123 29849 29849 18123 pts/7    29849 Sl+   1002   0:00 ./pthread
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18123 29849 29849 18123 pts/7    29849 Sl+   1002   0:00 ./pthread
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18123 29849 29849 18123 pts/7    29849 Sl+   1002   0:00 ./pthread
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18123 29849 29849 18123 pts/7    29849 Sl+   1002   0:00 ./pthread
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18123 29849 29849 18123 pts/7    29849 Sl+   1002   0:00 ./pthread
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18123 29849 29849 18123 pts/7    29849 Sl+   1002   0:00 ./pthread
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
18123 29849 29849 18123 pts/7    29849 Sl+   1002   0:00 ./pthread
^C
[wind@starry-sky control]$
```

我们可以看到, 确实只有一个进程, 状态一栏中的`l`就是`lightweight`, 也就是轻量级的意思, 毕竟Linux在形式上没有线程, 只有轻量级进程, 加`l`的意思就是这是一个多线程的进程.

但如果我们就是想要看线程, 当然, 线程这个概念是线程库维护的, 在系统层看不到线程, 这种说法不严谨, 更严谨的说法是, 看看线程在系统中的对应实体, 也就是轻量化进程,   怎么看呢? 再给`ps`加个`L`的选项就行了

```shell
[wind@starry-sky control]$ #a:all  L:lightweight
[wind@starry-sky control]$ ps -aL
  PID   LWP TTY          TIME CMD
30543 30543 pts/7    00:00:00 pthread
30543 30544 pts/7    00:00:00 pthread
30574 30574 pts/0    00:00:00 ps
[wind@starry-sky control]$
```

现在我们就能看到两个`pthread`, 它们的`pid`都是`30543`, `LWP`其实就是`lightweight process`, 轻量级进程的缩写, 这是系统内部为该轻量级进程分配的`id`值, `TTY`就是它们运行的终端编号, 我们可以看到这个终端是`pts/0`, 因为`ps`就是在这里执行的, `pthread`则是运行在`pts/7`上, 这里再次强调, `30543`和`30544`是轻量级进程的`id`值, 不是线程的`id`值, 我们用的线程`id`是`pthread_create`第一个参数的那个, 不是这个, 它们俩没有直接关系.另外我们还可以看到, 有一个`LWP`和自身的`PID`是相同的, 这个`LWP`就是主线程对应的`LWP`.另外, 我们也可以说, CPU运行的实际上都是轻量级进程, 对于没有往下细分线程的进程, 比如上面的`ps`, 它就是一个进程, 它也是一个轻量级进程, 我们可以看到`PID`和`LWP`是相同的, 那就自己一个运行, 而对于向下细分的进程, 那就各运行各的,我们以前说的, CPU运行的是进程, 实际上是不严谨的, CPU实际上运行的是轻量级进程, 当然我说的Linux.

现在我们对主线程和副线程分别发送`9`号信号, 因为线程之间会互相影响, 所以任何一个轻量级进程收到信号都会导致整个进程的退出.

```shell
[wind@starry-sky control]$ kill -9 30543
[wind@starry-sky control]$ ps -al
F S   UID   PID  PPID  C PRI  NI ADDR SZ WCHAN  TTY          TIME CMD
0 R  1002  4318 17593  0  80   0 - 38324 -      pts/0    00:00:00 ps
[wind@starry-sky control]$ # 整个进程都崩溃了
[wind@starry-sky control]$ # 现在再去另一个终端重启一下
[wind@starry-sky control]$ ps -aL | grep -v grep | grep -v ps
  PID   LWP TTY          TIME CMD
 4391  4391 pts/7    00:00:00 pthread
 4391  4392 pts/7    00:00:00 pthread
[wind@starry-sky control]$ kill -9 4392
[wind@starry-sky control]$ ps -aL | grep -v grep | grep -v ps
  PID   LWP TTY          TIME CMD
[wind@starry-sky control]$
```

下面我们再写一份代码, 在这份代码中, 我们会创建一个全局变量或者在栈上开辟一个空间, 主线程将会对这些数据进行修改, 副线程会将它们打印出来, 以此来表现,线程对进程资源的共享.

```cpp
#include <pthread.h>  // 线程库头文件
#include <unistd.h>

#include <iostream>

using namespace std;

// 全局变量 count, 负责计数循环次数
int count = 0;

void* thread_behavior(void* arg) {
    auto& text = *static_cast<string*>(arg);
    while (true) {
        cout << text << " " << count << endl;
        sleep(1);
    }
}

int main() {
    pthread_t tid;

    // 在堆上创建一个字符串存储打印文本
    auto text = new string();

    // 将其传入副线程的执行逻辑中
    pthread_create(&tid, nullptr, thread_behavior, text);

    while (true) {
        // 以按位与的形式抹除掉除最低位和次低位之外的数字, 这样 flag 的范围始终都是 0 ~ 3
        int flag = count & 3;
        switch (flag) {
            case 0:
                text->operator=("因为太阳将要毁伤");
                break;
            case 1:
                text->operator=("因为月亮将要坠落");
                break;
            case 2:
                text->operator=("因为我们从未分离");
                break;
            case 3:
                text->operator=("因为我们终将重逢");
                break;
        }
        count++;
        sleep(1);
    }

    return 0;
}
```

```shell
[wind@starry-sky build]$ ./pthread 
因为太阳将要毁伤 1
因为太阳将要毁伤 1
因为月亮将要坠落 2
因为我们从未分离 3
因为我们终将重逢 4
因为太阳将要毁伤 5
因为月亮将要坠落 6
因为我们终将重逢 8
因为太阳将要毁伤 9
因为月亮将要坠落 10
因为我们从未分离 11
因为我们终将重逢 12
因为太阳将要毁伤 13
因为月亮将要坠落 14
因为我们从未分离 15
^C
[wind@starry-sky build]$ 
```

下面我们不再对副线程进行处理, 只是看看线程`id`长什么样, 采用两种方式, 一种是无符号长整型形式, 另一种是地址形式,  线程`id`实际上就是地址, 至于到底是什么地址,等会再说.

```cpp
void* thread_behavior(void* x)
{
    while(true)
    {
        sleep(2);
    }
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, nullptr, thread_behavior, nullptr);
    printf("%ul\n", tid);
    printf("%p\n", tid);
    return 0;
}
```

 ```shell
 [wind@starry-sky control]$ ./pthread
 827516672l
 0x7f463152e700
 [wind@starry-sky control]$
 ```

--------------

上面我们只要说的是线程的创建, 现在我们来说说线程的等待. 线程也是有等待机制的, 因为线程也占据着一些进程资源, 所以要用等待的方式将已经退出的线程资源回收, 并且获取线程的运行结果, 线程创建肯定是要给它安排任务的, 当它们执行完后, 主线程就需要回收它们的运行结果, 在形式上, 就是`thread_behavior`的`void*`返回值.

![image-20250221154817514](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250221154817643.png)

```cpp
#include <pthread.h>

int pthread_join(pthread_t thread, void **retval);

Compile and link with -pthread.
```

第一个参数`thread`, 相信不用多说, 就是线程`id`, 第二个参数是个二级指针, 是用来回收 `thread_behavior`的`void*`返回值的, 它的意思就是让你在主线程栈帧或者堆上创建一个指针变量, 然后再对指针变量取地址传给`pthread_join`, `pthread_join`在内部就会把`retval`解引用, 把`thread_behavior`的`void*`返回值写回去, 这是指针内容, 我们不会具体谈.. `pthread_join`的返回值和`pthread_create`类似

副线程的执行范围只有`thread_behavior`, 当`thread_behavior`执行完后, 线程就会退出, `pthread_join`的等待方式是阻塞等待, 当副线程不退出时, 主线程就会一直阻塞在`pthread_join`.

下面我们再写一份代码, 创建一个新线程, 这个新线程将会完成一个特定任务: 求一到某个数的连加, 随后把结果传递出来.

![image-20251028100529301](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251028100529301.png)

```shell
[wind@starry-sky build]$ ./pthread 
5050
[wind@starry-sky build]$ 
```

顺口说一下, `pthread_join`没有类似于线程退出码的东西, 因为如果副线程出错崩溃了, 整个线程都会崩溃, 所以主线程接收不到错误码, 所以`pthread_join`压根没有设计线程退出码这种东西.

如果想要终止线程, 不能用`exit()`, `exit()`是用来终止进程的, 如果确实想终止线程的的话, 应该使用`pthread_exit()`, 该函数的参数是`void*`, 其实就是我们应该返回的那个`void*`.

```cpp
void* thread_behavior(void* p)
{
    int& end = *static_cast<int*>(p);
    int* ret = new int();
    for(int start = 1; start <= end; ++start)
    {
        *ret += start;
    }
    pthread_exit(static_cast<void*>(ret));
}

int main()
{
    pthread_t thread;
    int end = 100;
    pthread_create(&thread, nullptr, thread_behavior, &end);
    int* ret = nullptr;
    pthread_join(thread, (void**)&ret);
    cout << *ret <<endl;
    delete ret;
    return 0;
}
```

另外, 需要注意的是, 主线程退出意味整个进程退出.

再来说一个接口, `pthread_cancel`, 它的功能就是, 主线程创建一个新线程, 然后反悔了, 就可以用该函数直接终止指定的线程

```cpp
#include <pthread.h>

int pthread_cancel(pthread_t thread);

Compile and link with -pthread.
```

```cpp
void* thread_behavior(void* p)
{
    int& end = *static_cast<int*>(p);
    int* ret = new int();
    for(int start = 1; start <= end; ++start)
    {
        *ret += start;
    }
    // 确保有足够空闲时间让pthread_cancel发挥作用
    sleep(2);
    pthread_exit(static_cast<void*>(ret));
}

int main()
{
    pthread_t thread;
    int end = 100;
    pthread_create(&thread, nullptr, thread_behavior, &end);
    pthread_cancel(thread);
    int* ret = nullptr;
    pthread_join(thread, (void**)&ret);
    printf("%d\n", ret);
    return 0;
}
```

```shell
[wind@starry-sky control]$ ./pthread
-1
[wind@starry-sky control]$
```

对于被`pthread_cancel`取消的线程来说, `pthread_join`得到是返回值是`PTHREAD_CANCELED`, 该宏的字面量为`-1`, 对于这种情况, 需要警惕因为访问返回值指向空间而导致的段错误以及内存泄露.

------------------------------

C++也是有线程库的, 为了区分它们, 我们把`pthread`叫做原生线程库, C++线程库就继续叫C++线程库, C++线程库实际上是对原生线程库的再封装, 由于在封装的过程中用了些C++自己的语法, 所以C++的线程库更好用, 而且C++是一门跨平台语言, 同一个接口的具体实现, 既有使用`Windows`系统接口写的版本, 也有使用`Linux`原生线程库写的版本,  这意味着, 同一份C++线程库代码可以在`Windows, Linux`这些不同的系统平台使用的, 所以从理论上来说, 我们推荐使用C++线程库, 但是, 在后续的`Linux`学习中, 我们更多的使用原生线程库, 不仅仅是因为我们是学系统的, 更应该侧重于系统, 更重要的是, 因为原生线程库足够原生, 太没有进行过多的封装, 因此有利于我们对于线程这个概念本身的学习. 如果用 C++ 的线程库, 什么都看不到, 那就更不好学. 而在实际工程中, 我们当然尽可能避免直接使用原生线程库.

C++线程库的具体用法是语言层的事, 在这里我们只粗略用用最基本的接口.

```cpp
#include <pthread.h>   // 线程库的头文件
#include<iostream>
#include<unistd.h>
#include<thread>

using namespace std;

void threadrun()
{
    while(true)
    {
        cout << "这是使用C++线程库创建的新线程"<<endl;
        sleep(1);
    }
}

int main()
{
    thread t1(threadrun);
    t1.join();
    return 0;
}
```

因为本质上是对原生线程库的封装, 所以使用时也要链接原生线程库

```shell
[wind@starry-sky control]$ g++ main.cc -o thread
/opt/rh/devtoolset-11/root/usr/libexec/gcc/x86_64-redhat-linux/11/ld: /opt/rh/devtoolset-11/root/usr/lib/gcc/x86_64-redhat-linux/11/libstdc++_nonshared.a(thread48.o): in function `std::thread::_M_start_thread(std::unique_ptr<std::thread::_State, std::default_delete<std::thread::_State> >, void (*)())':
(.text._ZNSt6thread15_M_start_threadESt10unique_ptrINS_6_StateESt14default_deleteIS1_EEPFvvE+0x26): undefined reference to `pthread_create'
/opt/rh/devtoolset-11/root/usr/libexec/gcc/x86_64-redhat-linux/11/ld: /opt/rh/devtoolset-11/root/usr/lib/gcc/x86_64-redhat-linux/11/libstdc++_nonshared.a(thread48.o): in function `std::thread::_M_start_thread(std::shared_ptr<std::thread::_Impl_base>, void (*)())':
(.text._ZNSt6thread15_M_start_threadESt10shared_ptrINS_10_Impl_baseEEPFvvE+0x62): undefined reference to `pthread_create'
/opt/rh/devtoolset-11/root/usr/libexec/gcc/x86_64-redhat-linux/11/ld: /tmp/cc2jNgdU.o: in function `std::thread::thread<void (&)(), , void>(void (&)())':
main.cc:(.text._ZNSt6threadC2IRFvvEJEvEEOT_DpOT0_[_ZNSt6threadC5IRFvvEJEvEEOT_DpOT0_]+0x21): undefined reference to `pthread_create'
collect2: error: ld returned 1 exit status
[wind@starry-sky control]$ g++ main.cc -lpthread -o thread
[wind@starry-sky control]$ ls
main.cc  makefile  pthread  thread
[wind@starry-sky control]$
```

```shell
[wind@starry-sky control]$ ps -aL | grep -v grep | grep -v ps
  PID   LWP TTY          TIME CMD
 2395  2395 pts/8    00:00:00 thread
 2395  2396 pts/8    00:00:00 thread
[wind@starry-sky control]$ kill -9 2395
[wind@starry-sky control]$
```

-----------------

接下来我们朝原理方面说一说, 就是这个线程到底是怎么被创建出来的

之前我们说过, 线程`id`实际上是一个地址, 那究竟是什么地址呢? 马上就会说.

```cpp
void* pthread_behavior(void* p)
{
    int count = 5;
    while(count--)
    {
        // pthread_self能返回自身的线程id
        printf("%p\n", pthread_self());
        sleep(1);
    }
    return nullptr;
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, nullptr, pthread_behavior, nullptr);
    pthread_join(tid, nullptr);
    return 0;
}
```

```shell
[wind@starry-sky control]$ ./pthread
0x7fd644d69700
0x7fd644d69700
0x7fd644d69700
0x7fd644d69700
0x7fd644d69700
[wind@starry-sky control]$
```

  线程库是对轻量级进程进行封装才产生了线程的概念,  它所使用的系统接口是`clone` 

```cpp
#include <sched.h>

int clone(int (*fn)(void *), void *child_stack,
          int flags, void *arg, ...
          /* pid_t *ptid, void *newtls, pid_t *ctid */ );
```

使用`clone`就可以创建一个轻量级进程, 但这轻量级进程的具体属性, 就需要我们通过其中的各种属性进行控制. 从而模拟出线程的效果. 

`fork`和`clone`有一定的相似之处, 因为轻量级进程也包括进程, 所以`fork`在功能上实际是`clone`的子集, 第一个参数`fn`就是轻量级进程要执行的具体函数, 第二个参数是子栈区, 在创建一个新的轻量级线程之前, 我们需要为它创建一个栈区, 这样新的轻量级进程才有自己的栈, 从而不会对别的轻量级进程产生影响 , 第三个参数是个标志位, 用来描述轻量级进程的某些行为, 通过对他的调控, 我们就可以模拟出线程的效果, `arg`是`fn`的参数, 还有其他的一些参数, 在这里我们就不管了.

我们强调, 在系统层是没有线程这一概念的, 只有轻量级进程的概念, 线程库通过对轻量级进程的操作, 为我们在应用层上实现了线程, 同时, 线程库也需要对这些线程进行一定的管理, 所以线程控制块是线程库及其上层才有的概念, 线程控制块中包含许多信息, 比如, 线程在系统层对应的轻量级进程是哪一个, 运行在哪个栈上, 又因为线程库是链接库, 所以它实际上位于地址空间的共享区上, 所以线程库实际上是从共享区开辟一处空间作为线程的独立栈的.由于这种线程实际是在应用层, 所以可以叫做用户级线程

![image-20250222092334899](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250222092334994.png)

在这里, 我通过 clone 模拟实现了 `pthread`, 并且是以 C++的风格

![image-20251103115432022](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251103115432022.png)

在这里, 你可以看到, 我这里返回的是一个智能指针, 也就是被进一步封装的指针.

这个智能指针, 所指向的对象, 你可以视为是一种"线程"控住块, 看, 这里有返回值

![image-20251103115702088](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251103115702088.png)

它的基类则有诸如线程子栈指针, 子线程 `tid`, 任务之类的东西.

![image-20251103115817941](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251103115817941.png)

我没有手动创建局部存储, 因为C/C++对局部存储有特定的初始化, 写起来可就太复杂了, 所以我是让标准库自己创建配置局部存储的

在线程启动这个方法中, 我们可以看到, 我们先从内核中匿名申请了一份空间作为新线程的栈

![image-20251103120105674](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251103120105674.png)

这里使用的申请方法, 是比 malloc 更加底层的 `mmap`, `mmap`它不仅可以申请控件, 还可以把文件映射到申请的空间上, 这里的文件一般指的是设备文件, 我们可以把设备的寄存器给映射到内存上, 这样就可以通过寄存器控制设备. 不过我这里只是单纯设置了空间, 没有做映射, 像我这种做法, 所申请的控件, 叫做"匿名空间", 意思就是这段空间没有文件的所属关系.

在这之后,我们就通过对 clone flags 参数的控制, 模拟出了线程的效果

![image-20251103120533949](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251103120533949.png)

需要注意的是, `pthread`返回的`pthread_t`在早期确实大概类似是线程控制块地址之类的东西, 但随着 Linux 的不断发展, 现在的`pthread_t`已经被一层层封装了, 所以实际上并不是直接指向线程控制块, 但它们之间一定是有所联系的. 另外, `pthread_t`其所在的具体位置由系统决定, 所以它不一定位于进程地址空间中的特定的某个区域.

-----------------------------

下面我们写几分代码, 来检验线程的其它一些特性.

之前我们对线程中行为函数的传参都非常简单, 这次我们来进行多参数, 多类型的传参, 在实际项目中, 对于这种传参应该用C++线程库, 但上面我们也说过, 所以我们这里还是用原生线程库的方式进行传参, 一般情况下, 对于这种传参我们都是直接写一个结构体作为参数包, 在堆上创建这个参数包, 然后传过去, 下面, 我们创建多个线程, 并且对这些线程进行多参数传参, 具体参数有两个,  一个是字符串, 表示我们自定义的线程编号,按照线程创建的时间先后顺序从`0`开始排列, 方便之后我们在打印时区别各个线程, 另一个是纯粹为传参而存在的杂项整型, 没有实际意义.

```cpp
struct Args
{
    string _id;
    int _num;

    Args(int id, int num)
        :_id("thread-" + to_string(id))
        ,_num(num)
        {}
};

void* pthread_behavior(void* p)
{
    auto* args = static_cast<Args*>(p);
    printf("[%s]info: %d\n", args->_id.c_str(), args->_num);
    delete args;
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr));
    vector<pthread_t> threads;
    int num = 5;
    for(int i = 0; i < num; ++i)
    {
        pthread_t id;
        auto* p = new Args(i, rand());
        pthread_create(&id, nullptr, pthread_behavior, p);
        threads.push_back(id);
    }

    for(auto thread: threads)
    {
        pthread_join(thread, nullptr);
    }

    return 0;
}
```

```shell
[wind@starry-sky control]$ ./pthread
[thread-0]info: 1642382978
[thread-1]info: 666567926
[thread-2]info: 261686778
[thread-4]info: 1466408821
[thread-3]info: 455113903
[wind@starry-sky control]$
```

接下来我们把代码稍微改改, 打印一下线程行为函数中局部变量`args`的地址, 此举主要是为了体现线程们具有独立的栈, 其中的局部变量并不是同一个.(其实用刚刚的clone模拟你也可以看出来, 毕竟我是用`mmap`一个栈一个栈地申请的), 因为所有的线程用的都是同一个方法, `pthread_behavior`, 所以它们对于这个函数的栈帧的内存布局都是一致的, 因此, 假设它们用的都是同一个栈, 那么由于内存布局一致, 同样位置的参数其内存地址应该是一样的

```cpp
void* pthread_behavior(void* p)
{
    auto* args = static_cast<Args*>(p);
    printf("[%s]info: %p\n", args->_id.c_str(), &args);
    delete args;
    return nullptr;
}
```

```shell
[wind@starry-sky control]$ ./pthread
[thread-0]info: 0x7f268adceef8
[thread-1]info: 0x7f268a5cdef8
[thread-2]info: 0x7f2689dccef8
[thread-3]info: 0x7f26895cbef8
[thread-4]info: 0x7f2688dcaef8
[wind@starry-sky control]$
```

下面我们用些手段, 来证明线程们的栈是具有独立性, 但并不是完全隔绝,毕竟还是在一个地址空间的.  我们让主线程定位副线程中的局部变量, 为了特别区分, 现在我们把主线程叫做`thread-0`, 副线程从`1`开始.

下面代码的意思, 就是我们通过全局变量这一手段, 将副线程函数中的局部变量地址拿到, 这样就可以定位和使用副线程中的变量资源了.

```cpp
void*a = nullptr;

struct Args
{
    string _id;
    int _num;

    Args(int id, int num)
        :_id("thread-" + to_string(id))
        ,_num(num)
        {}
};

void* pthread_behavior(void* p)
{
    auto* args = static_cast<Args*>(p);
    if(args->_id == "thread-2")
    {
        a = &args;
    }
    printf("[%s]info: %p\n", args->_id.c_str(), &args);
    if(a == &args)
    {
        sleep(2);
    }
    delete args;
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr));
    vector<pthread_t> threads;
    int num = 5;
    for(int i = 1; i <= num; ++i)
    {
        pthread_t id;
        auto* p = new Args(i, rand());
        pthread_create(&id, nullptr, pthread_behavior, p);
        threads.push_back(id);
    }

    sleep(2);
    printf("[%s]info: %s->%p\n", "thread-0", "thread-2::args", a);
    for(auto thread: threads)
    {
        pthread_join(thread, nullptr);
    }

    return 0;
}
```

里面的`sleep`是为了增强时序性, 虽然线程的具体调度是完全看调度器的, 但我们可以通过`sleep`尽量让它们的执行顺序朝着我们设想的那样走:  主线程的`sleep(2)`是为了让主线程查询`a`之前确保`a`已经被赋值了, 副线程的`sleep(2)`是为了让目标副线程在主线程查询`a`之前不退出.

```cpp
[wind@starry-sky control]$ ./pthread
[thread-1]info: 0x7fa3a52f8ef8
[thread-4]info: 0x7fa3a3af5ef8
[thread-5]info: 0x7fa3a32f4ef8
[thread-3]info: 0x7fa3a42f6ef8
[thread-2]info: 0x7fa3a4af7ef8
[thread-0]info: thread-2::args->0x7fa3a4af7ef8
[wind@starry-sky control]$
```

需要注意的是, 日常生活中我们不会真的这样写, 这份代码纯粹是为了证明, 我们在项目里不会通过拿到副线程局部变量地址的形式来修改它们, 也不会以其他方式修改局部变量, 这样会造成变量资源的管理范围不明, 从而引发各种各样的问题.

## 局部储存

现在我们要逐步进入正题了, 在上面的代码中, 我们通过使用全局变量将独立栈中局部变量的地址在线程之间共享, 这说明在线程们的眼中, 这全局变量`a`都是同一个, 如何验证这个观点呢? 很简单, 我们让线程们都打印一下`a`本身的地址即可, 因为它们处于同一个地址空间, 如果地址相同, 就意味这它们眼中的`a`确实是同一个`a`.

```cpp
void*a = nullptr;

struct Args
{
    string _id;
    int _num;

    Args(int id, int num)
        :_id("thread-" + to_string(id))
        ,_num(num)
        {}
};

void* pthread_behavior(void* p)
{
    auto* args = static_cast<Args*>(p);
    printf("[%s]info: %s = %p\n", args->_id.c_str(), "global::&a", &a);
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr));
    vector<pthread_t> threads;
    vector<Args*> va;
    int num = 5;
    for(int i = 1; i <= num; ++i)
    {
        pthread_t id;
        auto* p = new Args(i, rand());
        va.push_back(p);
        pthread_create(&id, nullptr, pthread_behavior, p);
        threads.push_back(id);
    }

    printf("[%s]info: %s = %p\n", "thread-0", "global::&a", &a);
    for(auto thread: threads)
    {
        pthread_join(thread, nullptr);
    }

    // 考虑到线程可能创建失败,  释放在主线程阶段或许更安全
    for(auto p : va)
    {
        delete p;
    }

    return 0;
}
```

```shelll
[wind@starry-sky control]$ ./pthread
[thread-1]info: global::&a = 0x410230
[thread-0]info: global::&a = 0x410230
[thread-4]info: global::&a = 0x410230
[thread-5]info: global::&a = 0x410230
[thread-3]info: global::&a = 0x410230
[thread-2]info: global::&a = 0x410230
[wind@starry-sky control]$
```

运行的实际结果证明了我们的想法, 并且从地址的具体值也可以看到它比较小, 应该是全局变量, 因为全局变量段在地址空间较低的位置,

这种, 对于多执行流共享的资源便称之为"临界资源", 由于大家都能对临界资源进行操作, 所以有时候就会互相干扰, 为此我们就需要对临界资源保护, 让我们先来看第一种保护手段     局部存储.

局部存储可以让线程们具有私有的全局变量,   什么叫私有的全局变量呢?就是线程各用各的全局变量, 它们之间互不干扰.  当我们在全局变量前修饰`__thread`或`thread_local`时, 它就成为了私有全局变量.  私有全局变量的生命周期与线程的生命周期相同. 这些变量会在各自线程的局部存储中单独定义实例.

局部存储（TLS）是由编译器和原生线程库共同实现的。在编译时，编译器识别出被 __thread 标记为线程私有的全局变量，对其进行特殊标记，表明“新线程创建时需为每个线程生成一份实例”。程序运行时，当创建新线程，线程库向操作系统申请若干内存空间——实质是将虚拟地址映射到物理内存。随后，线程库在这些空间中实例化线程控制块（TCB）、局部存储空间（TLS）和独立栈等结构。TCB 通过指针等索引，将这些可能分散的区域逻辑上紧密联系起来。接着，线程库根据编译器的标记，在 TLS 空间中为每个线程实例化对应的私有全局变量。至于这个局部存储大致在哪里, 那也是因为其本质为内核决定, 所以不一定有明确的位置特征--比如说它一定在地址空间中的某个区域, 但大致在堆上, 因为底层的 `mmap`一般就是往堆上找, 因为堆的设计目的就是留块地方动态分配空间.

```cpp
__thread void*a = nullptr;

struct Args
{
    string _id;
    int _num;
    Args(int id, int num)
        :_id("thread-" + to_string(id))
        ,_num(num)
        {}
};

void* pthread_behavior(void* p)
{
    auto* args = static_cast<Args*>(p);
    printf("[%s]info: %s = %p\n", args->_id.c_str(), "global::&a", &a);
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr));
    vector<pthread_t> threads;
    vector<Args*> va;
    int num = 5;
    for(int i = 1; i <= num; ++i)
    {
        pthread_t id;
        auto* p = new Args(i, rand());
        va.push_back(p);
        pthread_create(&id, nullptr, pthread_behavior, p);
        threads.push_back(id);
    }

    printf("[%s]info: %s = %p\n", "thread-0", "global::&a", &a);
    for(auto thread: threads)
    {
        pthread_join(thread, nullptr);
    }

    // 考虑到线程可能创建失败,  释放在主线程阶段或许更安全
    for(auto p : va)
    {
        delete p;
    }

    return 0;
}
```

```shell
[wind@starry-sky control]$ ./pthread
[thread-1]info: global::&a = 0x7f222727a6f8
[thread-3]info: global::&a = 0x7f22262786f8
[thread-0]info: global::&a = 0x7f2228313778
[thread-4]info: global::&a = 0x7f2225a776f8
[thread-5]info: global::&a = 0x7f22252766f8
[thread-2]info: global::&a = 0x7f2226a796f8
[wind@starry-sky control]$
```

我们看到, 这些地址是不同的, 其次, 我们还可以看到它们的地址差不多,  那可能是内核觉得这里比较适合, 所以都放在差不多的地方了.

我们用`thread_local`修饰也可以达到相同的效果.

```cpp
thread_local void*a = nullptr;
```

```shell
[wind@starry-sky control]$ ./pthread
[thread-1]info: global::&a = 0x7fa40b3146e8
[thread-2]info: global::&a = 0x7fa40ab136e8
[thread-3]info: global::&a = 0x7fa40a3126e8
[thread-4]info: global::&a = 0x7fa409b116e8
[thread-0]info: global::&a = 0x7fa40c3ad768
[thread-5]info: global::&a = 0x7fa4093106e8
[wind@starry-sky control]$
```

局部存储（TLS）有什么用呢？比如，假设我们有多个针对同一事物的预测模型，想比较它们的性能。我们可以为每个模型设置一些初始的私有全局变量（比如统计指标或配置参数），然后用多线程分别运行这些模型，每个线程独立记录自己的结果，最后汇总对比它们的效果。

那`__thread`和`thread_local`有什么区别呢? 单纯从效果上看`__thread`只能用在内置类型上, 用在自定义类型会报错

```cpp
__thread string b("cdsccf");
```

```shell
[wind@starry-sky control]$ make
main.cc:13:17: error: non-local variable ‘b’ declared ‘__thread’ needs dynamic initialization
   13 | __thread string b("cdsccf");
      |                 ^
main.cc:13:17: note: C++11 ‘thread_local’ allows dynamic initialization and destruction
make: *** [pthread] Error 1
[wind@starry-sky control]$
```

`thread_local`不会

```shell
[wind@starry-sky control]$ cat main.cc | head -13 | tail -1
thread_local string b("cdsccf");
[wind@starry-sky control]$ make clean
[wind@starry-sky control]$ make
[wind@starry-sky control]$ ls
main.cc  makefile  pthread  thread.h
[wind@starry-sky control]$
```

为什么呢?

Linux 最初是用 C 语言写的，虽然开发中常能看到面向对象的影子，但 C 毕竟不是成熟的面向对象语言，没有复杂的类型支持。早期 GCC 引入的 `__thread` 是为线程局部存储设计的，它追求简单高效，只支持静态初始化的简单类型。当时无论从设计思想（保持低开销）还是技术实现（无法调用构造/析构函数）来看，它都不支持复杂类型。后来，随着 C++ 的发展，实际项目中出现了更复杂的需求，于是 C++11 在 2011 年引入了 `thread_local` 关键字。为了支持复杂类型，编译器、线程库和系统内核都做了相应的适配，让线程能动态构造和析构对象。值得一提的是，`__thread` 只是 GCC 的扩展，不是 C 的标准关键字，而 `thread_local` 是 C++ 的正式语法，属于语言标准的一部分，适配性和功能都更强。所以，我们推荐在现代编程中使用 `thread_local`。

## 线程分离

我们知道`pthread_join`是阻塞式地对线程进行等待, 我觉得主线程在这里光等着有些浪费, 我也不操心线程的运行结果, 有没有其它回收进程资源的方式.

当然是有的. 那就是线程分离`thread detach`.  对于退出线程的资源回收, 有两种方式

- `join`     将子线程的执行结果和状态汇合回主线程，让主线程等待并在完成后利用这些信息继续后续操作，像是“分出去的支流回到主流”。
- `detach` 让子线程成为执行流的末梢，独立运行并在完成后自动结束，不再参与主线程的后续流程，像是“放手让支流自流”。

这两种操作是对立的, 在默认情况下, 新创建的线程是`joinable`, 线程退出之后需要`join`, 如果主线程的后续操作不需要副线程的结果, `join`就会让人难受, 所以可以使用`detach`让线程退出之后自动返还资源, 我们把这种操作叫做"线程分离"

```cpp
int pthread_detach(pthread_t thread);
```

参数是哪个线程, 那个线程就会被分离.

结合`pthread_self()`可以让一个线程自分离.

对已经分离的线程进行`join`是未定义的行为.

```cpp
void* pthread_behavior(void* p)
{
    auto* args = static_cast<Args*>(p);
    pthread_detach(pthread_self());
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr));
    vector<pthread_t> threads;
    vector<Args*> va;
    int num = 5;
    for(int i = 1; i <= num; ++i)
    {
        pthread_t id;
        auto* p = new Args(i, rand());
        va.push_back(p);
        pthread_create(&id, nullptr, pthread_behavior, p);
        threads.push_back(id);
    }

    // 确保都分离了
    sleep(1);
    for(auto thread: threads)
    {
        int result = pthread_join(thread, nullptr);
        if(result == 0)
        {
            printf("%s: %ld\n", "汇合成功", thread);
        }
        else
        {
            printf("%s: %ld", "汇合失败", thread);
            printf("   %s:%s\n", "失败原因", strerror(result));
        }
    }

    // 考虑到线程可能创建失败,  释放在主线程阶段或许更安全
    for(auto p : va)
    {
        delete p;
    }

    return 0;
}
```

```shell
[wind@starry-sky control]$ make clean ; make
[wind@starry-sky control]$ ./pthread
汇合失败: 139994473813760   失败原因:Invalid argument
汇合失败: 139994465421056   失败原因:Invalid argument
汇合失败: 139994457028352   失败原因:Invalid argument
汇合失败: 139994473813760   失败原因:Invalid argument
汇合失败: 139994448635648   失败原因:Invalid argument
[wind@starry-sky control]$
```

在使用线程分离的时候需要注意一点, 由于没有原来的`join`来对主线程进行阻塞, 所以可能出现主线程先退出而副线程还在运行的情况, 主线程退出, 副线程也会退出, 所以要注意这一点, 让主线程最后退出.

```cpp
void* pthread_behavior(void* p)
{
    auto* args = static_cast<Args*>(p);
    cout << "start"<< endl;
    pthread_detach(pthread_self());
    sleep(5);
    cout << "end"<<endl;
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr));
    vector<pthread_t> threads;
    vector<Args*> va;
    int num = 5;
    for(int i = 1; i <= num; ++i)
    {
        pthread_t id;
        auto* p = new Args(i, rand());
        va.push_back(p);
        pthread_create(&id, nullptr, pthread_behavior, p);
        threads.push_back(id);
    }

    // 考虑到线程可能创建失败,  释放在主线程阶段或许更安全
    for(auto p : va)
    {
        delete p;
    }

    return 0;
}
```

```shell
[wind@starry-sky control]$ make clean ; make
[wind@starry-sky control]$ ./pthread
start
start[wind@starry-sky control]$
```

我们看到, `start`甚至没有打全.

那如何让主线程最后退出呢?有一种方法, 就是不让主线程退出. 比如我们后端的服务器来说, 程序就是一直循环, 以提供网络服务, 后面我们还会见到更多方式.

线程是否被分离可以视为一种线程属性, 调用`pthread_detach`的实质就是往线程控制块里改一下对应的标记位, 就会改变线程的退出行为, 使之自动释放资源.

## 互斥锁

有时, 我们要让多个线程同时操作一个资源, 此时这个资源就变成了临界资源, 由于线程们对临界资源并发操作, 所以就会引发很多奇怪的问题, 比如我们下面的代码

```cpp
int tickets = 1000;

struct Args
{
    string _id;
    int _num;
    Args(int id)
        :_id("thread-" + to_string(id))
        ,_num(0)
        {}
};

void* pthread_behavior(void * p)
{
    auto* args = static_cast<Args*>(p);
    while(true)
    {
        if(tickets > 0)
        {
            // 还有剩余的票可以购买
            usleep(1000);  // 制造窗口期
            printf("[%s]info %s, %s:%d\n", args->_id.c_str(), "Buy a ticket", "remaining tickets", tickets);
            --tickets;
        }
        else
            pthread_exit(nullptr);
    }
}

int main()
{
    vector<pthread_t> threads;
    vector<Args*> va;
    const int num = 5;
    for(int i = 1; i <= num; ++i)
    {
        pthread_t id;
        Args* data = new Args(i);
        va.push_back(data);
        pthread_create(&id, nullptr, pthread_behavior, data);
        threads.push_back(id);
    }

    for(pthread_t thread : threads)
    {
        pthread_join(thread, nullptr);
    }

    for(auto p: va)
    {
        delete p;
    }
    return 0;
}

```

在这份代码中, 多线程们有一个共同的全局变量`ticket`, 表示剩余的票数, 每个线程会去"购票", 在代码上就是对该全局变量进行减减, 当抢不到票时, 线程自动退出, 程序结束.

```shell
[wind@starry-sky control]$ make clean; make
[wind@starry-sky control]$ ./pthread
# ..... 省略
[thread-5]info Buy a ticket, remaining tickets:17
[thread-4]info Buy a ticket, remaining tickets:16
[thread-3]info Buy a ticket, remaining tickets:15
[thread-2]info Buy a ticket, remaining tickets:14
[thread-1]info Buy a ticket, remaining tickets:13
[thread-5]info Buy a ticket, remaining tickets:12
[thread-4]info Buy a ticket, remaining tickets:11
[thread-3]info Buy a ticket, remaining tickets:10
[thread-2]info Buy a ticket, remaining tickets:9
[thread-1]info Buy a ticket, remaining tickets:8
[thread-5]info Buy a ticket, remaining tickets:7
[thread-4]info Buy a ticket, remaining tickets:6
[thread-3]info Buy a ticket, remaining tickets:5
[thread-2]info Buy a ticket, remaining tickets:4
[thread-1]info Buy a ticket, remaining tickets:3
[thread-5]info Buy a ticket, remaining tickets:2
[thread-4]info Buy a ticket, remaining tickets:1
[thread-3]info Buy a ticket, remaining tickets:0
[thread-2]info Buy a ticket, remaining tickets:-1
[thread-1]info Buy a ticket, remaining tickets:-2
[thread-5]info Buy a ticket, remaining tickets:-3
[wind@starry-sky control]$ # 啊, 我忘了, 应该换子文件夹, 换成 mutex
```

奇了怪了, 抢到了零票甚至是负票?

我们先不谈上面代码的问题, 我们先来说说, 多个线程对同一个变量进行加加减减这件事本身是否是安全的,  答案很明显, 不安全.

计算机中只有CPU拥有计算能力, 所以无论是数值运算还是逻辑运算都必须把数据从内存加载到CPU上, 然后写回数据或者返回结果

首先加加减减这种操作是非原子化的, 它最起码分为三步, 第一步, 把内存中的变量加载到CPU中的某个寄存器中, 第二步, 寄存器对存储在其内部的数据进行加加减减, 第三步, 把CPU中的变量写回到内存中去.

用汇编来写就是

```assembly
movl tickets(%rip), %eax   # 将 tickets 的值加载到寄存器 eax
decl %eax                  # 对 eax 减 1（前置减减）
movl %eax, tickets(%rip)   # 将结果写回 tickets
```

在其中的任何一个环节, 线程都有可能被切换.

现在`thread-1`来执行该语句, 它首先把`tickets`(假设现在是1000)加载到寄存器上, 此时`tickets`实际上就变成了该线程上下文的一部分, 换种说法, 这就相当于`thread-1`现在有一个自己的`tickets` 不巧的是, 该线程此时被切换了, 为了让`thread-1`回来之后能恢复切换前的状态, 就需要把它的上下文打包带走,  于是它就带走了一个`tickets`(1000),  新来的是`thread-2`, 它也在执行减减操作, 它的情况比较好, 把三个环节都执行完了, 于是现在内存中的`tickets`变成了`999`, 现在我们说一下夸张的情况,  我们假想这个线程的循环之中只有减减, 没有其它东西, 所以它立刻又对`tickets`进行减减了,   一直减到`100`, 它继续减减, 刚把`tickets`(100)加载到寄存器上, 终于用完了时间片, 被切换了, 于是它就把`tickets`(100)作为自己的上下文打包带走了,注意此时内存`ticksts`是`100`,  又过了一会, 又轮到`thread-1`了, 它回来的第一件事是恢复切换前的CPU状态, 于是`eax`就变成了`1000`, 之后它对`1000`减减得到`999`, 又把`999`写到内存中, 于是`tickets`就由`100`变成了`999`, 然后我们就不看然后了.当然我们是夸张来说的, 我们把这种问题称为"数据不一致"问题.

不过这并不是上面代码出错的主要原因, 有没有这种可能, 上面的代码主要原因是因为`if`, 逻辑判断也是需要将数据写到寄存器上的, 所以可能出现这种情况, 一个线程还没来得及减减就切换了, 第二个线程条件判断为真, 进入了分支, 它甚至还没打印就被切换了, 之后第一个线程又回来进行了减减, 这样第二个线程再打印的时候就可能是非法的

实际上, 在上面的线程入口函数代码中, `usleep(1000)`有两个作用, 一方面, 它为了确认有没有等完, 需要频繁地在用户态和内核态切换, 而在执行流从内核态切换回用户态前, 就会检查时间片有没有耗完, 耗完就立刻切换,  另一方面耗时间, 让线程不能一口气走完整个分支, 增大数据不一致的可能, `printf`也有类似功能, 

为了解决类似的问题, 我们就需要保证, 对于临界资源的任何访问, 在任何时候都只能有一个执行流进行, 我们把这种操作叫做"互斥".原生线程库使用了"锁"实现"互斥".

先让我们认识几个锁的接口

```cpp
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_init(pthread_mutex_t *restrict mutex,
                       const pthread_mutexattr_t *restrict attr);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```

所谓"锁", 其实就是一种对象, `pthread_mutex_t`便是它的类型, 我们可以使用`pthread_mutex_init`创建一个锁, 第一个参数是输出型参数, 用来返回锁的指针, 第二个参数, 则是生成锁的相应配置, 我们在这里就直接传空指针, 表示默认配置.    如果想生成一个全局的锁, 便可以使用宏`PTHREAD_MUTEX_INITIALIZER`, `pthread_mutex_destroy`可以销毁一把锁.

为了便于我们理解锁, 我们可以先讲一个故事. 在你的学校, 有这样一个自习室:  这个自习室很小, 每次只能容纳一个人, 但设施完善, 很适合专心学习不被打扰.  最初为了方便, 自习室的门并没有装上锁.  但因为没有锁, 所以总是出现里面的人被外界打扰的情况, 为此, 学校给自习室装上了锁, 并规定: 如果自习室没人的话, 钥匙就会挂在门上, 当有同学要进入自习室自习时, 就把钥匙拿到自习室内, 并将门从内部反锁, 这样, 其它同学看到门上没有挂钥匙, 就知道自习室里已经有人了, 即使有人想进去, 也会因为门被锁住而无法进入, 当你不自习时, 就把锁打开, 把钥匙放到原处.

在这个故事中, 自习室就是被共享的临界资源, 我们要保证它只能被单执行流所访问. 那如何在临界资源的大门上装锁呢? 使用`pthread_mutex_lock`即可. 

```cpp
int pthread_mutex_lock(pthread_mutex_t *mutex);
```

装上锁后, 线程来到这里就会尝试解锁, 如果没有钥匙, 就会解锁失败, 进入阻塞状态, 如果有钥匙就会解锁成功, 访问到临界资源, 我们把某个线程"解锁成功"叫做该线程"获得了锁", 这样就可以保证那些会受临界资源影响的代码(临界区)在任何时候, 最多都只有一个线程执行, 也就是线程对临界区代码串行执行. 需要注意的是, 因为加锁后的临界区代码是串行执行的, 所以其代码量要尽可能小, 毕竟当初我们之所以使用多线程, 是为了用并行的方式增大资源利用率, 临界区代码太大会让每次串行都消耗更多的时间和空间, 从而降低资源利用率.  另外, 需要注意的是, 临界资源访问完毕后要"释放锁", 相当于把钥匙还到原处.

```cpp
#include"main.h"

using namespace std;

int tickets = 1000;

struct Args
{
    string _id;
    pthread_mutex_t* lock;
    Args(int id, pthread_mutex_t* mutex)
        :_id("thread-" + to_string(id))
        ,lock(mutex)
        {}
};

void* pthread_behavior(void * p)
{
    auto* args = static_cast<Args*>(p);
    while(true)
    {
        // 因为使用临界资源作为条件变量
        // 所以整个条件语句都成为了"临界区""
        pthread_mutex_lock(args->lock);
        // 没有获取锁的线程会阻塞在这里
        // 进而无法访问临界资源
        // 我们称之为该线程在等待锁资源
        if(tickets > 0)
        {
            // 还有剩余的票可以购买
            usleep(1000);  // 制造窗口期
            printf("[%s]info %s, %s:%d\n", args->_id.c_str(), "Buy a ticket", "remaining tickets", tickets);
            --tickets;
            // 释放锁    类似于把钥匙还回去
            pthread_mutex_unlock(args->lock);
        }
        else
        {
            // 线程退出不会将锁释放
            // 这种情况下别的线程就拿不到锁, 从而一直阻塞, 无法退出
            pthread_mutex_unlock(args->lock);
            pthread_exit(nullptr);
        }
    }
}

int main()
{
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, nullptr);

    vector<pthread_t> threads;
    vector<Args*> va;
    const int num = 5;
    for(int i = 1; i <= num; ++i)
    {
        pthread_t id;
        Args* data = new Args(i, &lock);
        va.push_back(data);
        pthread_create(&id, nullptr, pthread_behavior, data);
        threads.push_back(id);
    }

    for(pthread_t thread : threads)
    {
        pthread_join(thread, nullptr);
    }

    for(auto p: va)
    {
        delete p;
    }

    pthread_mutex_destroy(&lock);

    return 0;
}

```

我们看看效果

```shell
[wind@starry-sky mutex]$ make clean
[wind@starry-sky mutex]$ make
[wind@starry-sky mutex]$ ./thread | tail -10
[thread-2]info Buy a ticket, remaining tickets:10
[thread-2]info Buy a ticket, remaining tickets:9
[thread-2]info Buy a ticket, remaining tickets:8
[thread-2]info Buy a ticket, remaining tickets:7
[thread-2]info Buy a ticket, remaining tickets:6
[thread-2]info Buy a ticket, remaining tickets:5
[thread-2]info Buy a ticket, remaining tickets:4
[thread-2]info Buy a ticket, remaining tickets:3
[thread-2]info Buy a ticket, remaining tickets:2
[thread-2]info Buy a ticket, remaining tickets:1
[wind@starry-sky mutex]$
```

我们看到确实没有出现剩余零张甚至负张票还能抢到的情况.

但是, 有一个新的问题, 票似乎都是一个线程抢的

```shell
[wind@starry-sky mutex]$ ./thread > txt
[wind@starry-sky mutex]$ cat txt | tail -1
[thread-1]info Buy a ticket, remaining tickets:1
[wind@starry-sky mutex]$ cat txt | grep thread-2 | head -1
[wind@starry-sky mutex]$ # thread-2 没有抢到一张票
[wind@starry-sky mutex]$ cat txt | grep thread-3 | head -1
[wind@starry-sky mutex]$ # thread-3 也是如此
[wind@starry-sky mutex]$ cat txt | grep thread-4 | head -1
[wind@starry-sky mutex]$ cat txt | grep thread-5 | head -1
[wind@starry-sky mutex]$ # 也就是说, thread-1从头抢到尾
[wind@starry-sky mutex]$ rm txt
[wind@starry-sky mutex]$ # 也有可能是少数几个线程轮流抢
[wind@starry-sky mutex]$ # 总之, 线程们的抢票能力有些太不均衡了
[wind@starry-sky mutex]$ # 此时, 我们可以说, 那些迟迟访问不到临界资源的线程被饿死了
[wind@starry-sky mutex]$ # 为了避免线程被饿死, 我们需要进行线程同步操作
```

为什么呢? 假设线程`A`获得锁, 于是其它线程就会因为等待锁资源而一直阻塞在`pthread_mutex_lock()`, 从阻塞到被唤醒这个过程是需要时间的, 由于释放锁后没有其他动作(按照正常逻辑来说, 抢到票之后应该对这张票进行"包装", 比如标注谁抢到了这张票, 这张票是干什么用的, 什么时候用的, 包装完成后把相关信息放到数据库里, 但这里我们为了省事, 并没有写) 所以线程`A`会立刻再次来到`pthread_mutex_lock()`, 而此时其它线程还没有被来得及被唤醒, 所以线程`A`又获得了锁, 等到其它线程被唤醒之后, 发现锁还是被别人占有着, 所以又回去睡觉了.   在这里, 我们让线程`A`释放锁之后小睡一会, 就当它是去干那些对抢到的票进行后续操作的事了.

```cpp
void* pthread_behavior(void * p)
{
    auto* args = static_cast<Args*>(p);
    while(true)
    {
        // 因为使用临界资源作为条件变量
        // 所以整个条件语句都成为了"临界区""
        pthread_mutex_lock(args->lock);
        // 没有获取锁的线程会阻塞在这里
        // 进而无法访问临界资源
        // 我们称之为该线程在等待锁资源
        if(tickets > 0)
        {
            // 还有剩余的票可以购买
            usleep(1000);  // 制造窗口期
            printf("[%s]info %s, %s:%d\n", args->_id.c_str(), "Buy a ticket", "remaining tickets", tickets);
            --tickets;
            // 释放锁    类似于把钥匙还回去
            pthread_mutex_unlock(args->lock);
        }
        else
        {
            // 线程退出不会将锁释放
            // 别的线程就会无法退出
            pthread_mutex_unlock(args->lock);
            pthread_exit(nullptr);
        }
        usleep(10);
    }
}

```

```shell
[wind@starry-sky mutex]$ make clean ; make
[wind@starry-sky mutex]$ ./thread > txt
[wind@starry-sky mutex]$ cat txt | grep thread-1 | head -1
[thread-1]info Buy a ticket, remaining tickets:1000
[wind@starry-sky mutex]$ cat txt | grep thread-2 | head -1
[thread-2]info Buy a ticket, remaining tickets:999
[wind@starry-sky mutex]$ cat txt | grep thread-3 | head -1
[thread-3]info Buy a ticket, remaining tickets:998
[wind@starry-sky mutex]$ cat txt | grep thread-4 | head -1
[thread-4]info Buy a ticket, remaining tickets:997
[wind@starry-sky mutex]$ cat txt | grep thread-5 | head -1
[thread-5]info Buy a ticket, remaining tickets:996
[wind@starry-sky mutex]$ cat txt | grep thread-1 | head -2
[thread-1]info Buy a ticket, remaining tickets:1000
[thread-1]info Buy a ticket, remaining tickets:995
[wind@starry-sky mutex]$ # 我们看到, 确实均衡了
```

如果用自习室这个例子的话. 张三天没亮就去自习室了, 果然没人, 进去自习了, 其他同学过来, 就知道自习室已经有人了, 于是就说;"在这等着也不是事, 我们去打把游戏", 过了一会, 由于没吃早饭, 张三饿了, 一开门发现门口乌压压的人, 他就想, 算了, 饿一顿不算什么, 又学了一会, 他学累了, 打算出来, 出来之后把钥匙放在原处后, 发现门口没什么人, 他就想, 反正也没人等着, 再回去自习吧, 这时候也不能吃早饭了, 再过一会, 等到中午, 早午饭一块吃, 但其实上, 刚刚就有同学过来, 看到自习室有人, 又听说里面的张三是个卷王, 心想, 估计他不到中午不会出来, 算了, 回去睡觉; 又过一会, 张三又出来, 把钥匙挂到原位了, 一看时间, 还是有点早, 没到吃午饭的时间, 算了回去吧, 总之, 张三总是因为各种原因犹犹豫豫, 刚把钥匙挂回去, 又取下来重新回去自习, 最后的结果是, 张三把大部分时间花在了瞻前顾后上, 没学到什么, 其他人, 更没学到什么, 终于, 张三下定决心出来了, 其他人看到, 一拥而上, 挤得那叫一个头破血流

后来, 领导知道这件事了, 领导说, 不能这样子, 我们需要 再加两条规则. 首先, 外面的同学必须有序排队, 如果自习室有人, 那就先排着, 等有人出来了, 队前面的先进, 并且, 如果有人把钥匙挂回去了, 你就不能在 直接进了, 要排到末尾, 轮到你再进.

把这个故事映射到线程上, 那就是要让线程获取锁具有顺序性, 我们把这种操作叫做同步., 现在不细说了.

-----------------

当一个线程获得锁后，它可能会被正常切换，就像在自习室占了座位想上厕所，可以把锁带在身上，上完厕所再回来。锁在释放前始终是线程上下文的一部分，线程切换时也会带着锁走。只要线程不释放锁，锁就绑定在它身上。所以线程退出时，锁不会自动释放——线程上下文失效，锁就‘丢失’了。

对于等待锁的线程来说，被锁保护的临界区代码要么由持锁线程执行，要么无人执行，保证了串行效果。即便持锁线程被切换走，其他线程依然会被阻塞，就像时间被暂停（因为阻塞不消耗时间片，所以看上去就像时间停止一般）。只有锁释放后，等待线程才能继续运行。对它们而言，临界区代码要么没开始，要么已完成，呈现出原子化的特性。

当然这只是线程们的时间被暂停了, 我们的时间可没有暂停键,当然也有可能是我们看不到, 所以, 锁实际上是一种时间换安全的策略.

-------------

下面, 我们换种方式创建一把锁, 那就是使用宏`PTHREAD_MUTEX_INITIALIZER`, 该宏可用于创建全局和静态锁, 但不能在函数栈帧上创建锁.

```cpp
#include"main.h"

using namespace std;

// 全局锁的定义
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int tickets = 1000;

struct Args
{
    string _id;
    pthread_mutex_t* _lock;
    Args(int id)
        :_id("thread-" + to_string(id))
        ,_lock(&lock)
        {}
};

void* pthread_behavior(void * p)
{
    auto* args = static_cast<Args*>(p);
    while(true)
    {
        // 因为使用临界资源作为条件变量
        // 所以整个条件语句都成为了"临界区""
        pthread_mutex_lock(args->_lock);
        // 没有获取锁的线程会阻塞在这里
        // 进而无法访问临界资源
        // 我们称之为该线程在等待锁资源
        if(tickets > 0)
        {
            // 还有剩余的票可以购买
            usleep(1000);  // 制造窗口期
            printf("[%s]info %s, %s:%d\n", args->_id.c_str(), "Buy a ticket", "remaining tickets", tickets);
            --tickets;
            // 释放锁    类似于把钥匙还回去
            pthread_mutex_unlock(args->_lock);
        }
        else
        {
            // 线程退出不会将锁释放
            // 别的线程就会无法退出
            pthread_mutex_unlock(args->_lock);
            pthread_exit(nullptr);
        }
        usleep(10);
    }
}

int main()
{
    // pthread_mutex_t lock;
    // pthread_mutex_init(&lock, nullptr);

    vector<pthread_t> threads;
    vector<Args*> va;
    const int num = 5;
    for(int i = 1; i <= num; ++i)
    {
        pthread_t id;
        Args* data = new Args(i);
        va.push_back(data);
        pthread_create(&id, nullptr, pthread_behavior, data);
        threads.push_back(id);
    }

    for(pthread_t thread : threads)
    {
        pthread_join(thread, nullptr);
    }

    for(auto p: va)
    {
        delete p;
    }

    // pthread_mutex_destroy(&lock);

    return 0;
}

```

```shell
[wind@starry-sky mutex]$ make
[wind@starry-sky mutex]$ ./thread | tail -1
[thread-2]info Buy a ticket, remaining tickets:1
[wind@starry-sky mutex]$
```

---------------------------

下面我们来说锁的实现方式.  在上面的过程中, 我们可以认识到锁本身是一种临界资源, 它以自身的原子性为依据, 将临界区代码也变得具有原子性. 一个线程要么拥有锁, 要么没有锁, 它的获取与释放都是串行的, 释放我们就不说了, 只有有锁的线程才能释放锁, 所以我们将把重点放在锁的获取上.   

下列代码是`pthread_mutex_lock(mutex)`逻辑上的伪代码,   为了减少干扰项, 其中的部分语句以C的方式来写.

```assembly
lock:
	movb $0, %al  
	xchgb %al, mutex
	if(al寄存器内容>0)
		return 0;
	else
		阻塞挂起;
	goto lock;其它

unlock:
	movb $1, mutex
	唤醒等待锁的线程
	return 0;
```

是的, `pthread_mutex_lock`翻译成汇编不是一条语句,  那它怎么做到原子性呢? 它不怕做到一半被切换走了, 留下一个中间状态吗?  不要着急, 等会我们细细说道.我们需要的注意的是,  一条汇编指令天然是原子性指令,  但原子性指令不一定是一条汇编. 尽管多核处理器可能有多个CPU, 每个CPU有自己的寄存器, 但也不要担心, 它有保证一条指令原子性的能力.

为了说明白互斥锁的实现原理, 我们首先要了解一个前提: 大多数计算机架构, 都提供了`swap or exchange`指令, 该指令的作用是把寄存器和内存单元上的数据进行交换, 上文中的`xchgb`就是这种指令, 

好了, 我们要说正题了.

我们以最开始的代码为例, 就是锁的本体是在独立栈上,  使用的是`pthread_mutex_init()`的方式. 不过, 我又想了一下, 其实也不用纠结那种锁的创建形式.

现在我们创建了一个锁, 它在内存上,  我们可以认为它的初始值为`1`, `al`是八位寄存器,  其实质是通用寄存器`eax`的第八位, `moveb`就是`move byte`,将`0`写入到`al`寄存器中,  也就是对`al`进行清零操作.   第二条指令, `xchgb %al, mutex`, 就是把`al`和`mutex`(`mutex`是指针)中的内容进行交换, 进行交换之后, 内存上的`mutex`就是零了, 而`al`就是`1`了,   这条指令的实质, 其实就是线程把锁变成了自己的上下文, 从而获得这把锁.   之后如果`al`中的内容是大于零的, 则说明线程拥有锁, 于是退出`pthread_mutex_lock()`, 反之, 则意味着没有锁, 那就先挂起, 挂起完了之后再通过`goto`回去

在上面的伪代码中, 真正使用临界资源的只有一行`xchgb %al, mutex`,  其它语句因为没有使用临界资源(寄存器中的内容是线程的上下文)所以即使并行处理, 也不会发生安全问题;  而对于`xchgb %al, mutex`来说, 它的实质就是尝试获得锁, 如果内存上的`mutex`是1, 那交换过来, 就相当于获得了锁, 反之, 如果`mutex`是`0`, 那交换过来, 就意味着获取失败, 所以不会出现所谓"获取中"的状态.

我们来假想一下, 最开始, `mutex`的量为`1`, 线程一来了, 做完`xchgb %al, mutex`就被切换走了, 但此时, 锁相当于成为线程一的上下文也被一块打包带走了, 此时内存`mutex`已经是0, 然后线程二来了, 线程二做完`xchgb %al, mutex`后获取锁失败, 被挂起了,  然后线程一回来, 继续运行成功返回, 接着运行. 

在锁的生命周期中, 只能被转移, 不能被创造和销毁. 所以锁自始至终, 只有一把.   ~~这让我想到费米子, 当然这可能是一种无效联想, 因为费米子是量子力学里的东西, 比锁更难理解. 费米子遵守泡利不相容原理，同一量子态只能有一个粒子，类似于锁的互斥性——同一时刻只能被一个线程持有, 它们可以通过物理过程转移位置，类似锁在线程间传递（从一个线程释放后被另一个线程获取）,与之相关的守恒量（如轻子数）不会凭空变化，类比锁在程序生命周期中数量不变。~~

对于锁的释放, 其实, 也并不是只有一种方式, 也可以不由拥有锁的线程释放锁, 这相当于一种强行释放, 用这种方式通常是拥有锁的线程因为某些原因还不了锁, 所以要强制释放.但这是我们后面在探讨的内容

-------------

下面我们把锁封装一下, 我们可以通过利用C++自动析构的特性, 实现锁的自动释放, 这样我们就不需要再手动写释放锁了, 相当于从`#ifndef`到`#pragma once`的那种感觉

```cpp
#include <pthread.h>

// 锁的基类, 便于对其进行后续扩展
class _base_lock_
{
protected:
    pthread_mutex_t *__mutex;
    _base_lock_(pthread_mutex_t *__lock_)
        : __mutex(__lock_)
    {
    }

    int lock()
    {
        return pthread_mutex_lock(__mutex);
    }

    int unlock()
    {
        return pthread_mutex_unlock(__mutex);
    }
};

class _lock final : public _base_lock_
{
public:
    _lock(pthread_mutex_t *_lock)
        : _base_lock_(_lock)
    {
        lock();
    }

    ~_lock()
    {
        unlock();
    }
};
```

```cpp
void *pthread_behavior(void *p)
{
    auto *args = static_cast<Args *>(p);
    while (true)
    {
        // 因为使用临界资源作为条件变量
        // 所以整个条件语句都成为了"临界区""
        // pthread_mutex_lock(args->_lock);
        // 没有获取锁的线程会阻塞在这里
        // 进而无法访问临界资源
        // 我们称之为该线程在等待锁资源

        {
            _lock guard(&lock);
            if (tickets > 0)
            {
                // 还有剩余的票可以购买
                usleep(1000); // 制造窗口期
                printf("[%s]info %s, %s:%d\n", args->_id.c_str(), "Buy a ticket", "remaining tickets", tickets);
                --tickets;
                // 释放锁    类似于把钥匙还回去
                pthread_mutex_unlock(args->_lock);
            }
            else
            {
                // 线程退出不会将锁释放
                // 别的线程就会无法退出
                // pthread_mutex_unlock(args->_lock);
                pthread_exit(nullptr);
            }
        }
        usleep(10);
    }
}
```

`guard`的生命周期是在`14`到`30`这个大括号中的, 所以当它生命周期结束后就会自动释放锁,   这种锁的风格被称为`RAII`, 也是C++线程锁的风格

```shell
[wind@starry-sky mutex]$ make clean ; make
[wind@starry-sky mutex]$ ./thread | tail -1
[thread-5]info Buy a ticket, remaining tickets:1
[wind@starry-sky mutex]$
```

## 锁

在线程互斥环节, 我们引入了锁的概念, 下面我们介绍一下其它的锁概念

-----------

死锁:

死锁就是始终获得不了的锁, 因为没有锁会阻塞, 从而造成某些执行流一直阻塞, 不往下运行的情况.

比如之前因为我们将抢到票的后续操作省略而导致部分线程始终获得不了所, 从而处于一种卡死的状态.   

一把锁仍可能造成死锁, 比如一个线程连续申请两把锁, 这可能是无意中把函数名敲错了, 少了一个`un`, 还有, 线程带着锁退出了, 如果仍有线程需要锁资源, 那也会造成死锁, 所以之前我们强调两个条件分支都要释放锁.

死锁的形成有四个必要条件，这些条件共同揭示了死锁发生的逻辑基础，并体现了一个逐步演化的过程。锁机制在保护临界资源的同时，本身也作为一种临界资源，将其他资源的竞争性统一转移到锁上，以便于用户更好地管理。从某种意义上说，锁已成为临界资源的抽象模型，管理好锁即管理好了其他临界资源。然而，这一机制也为死锁的发生创造了条件：

- 互斥条件：临界资源的访问通常是非原子性的，多个执行流同时访问会导致逻辑混乱。为解决这一问题，我们引入锁，使执行流对临界资源进行互斥访问。锁保护了其他临界资源的安全访问，但作为一种特殊的临界资源，它也将竞争集中于自身，为死锁引入了潜在风险。
- 请求与保持条件：当一个执行流因无法获得所需锁而阻塞时，它不会释放已持有的锁。这种行为使锁的分配陷入僵局，资源竞争的紧张状态开始显现，推动死锁进入条件积累阶段。
- 不剥夺条件：锁只能由持有它的执行流自行释放，无法被其他执行流或外部强制剥夺。由于这一特性，阻塞的执行流无法通过干预打破僵局，锁的竞争进一步加剧，死锁的条件积累持续深化。
- 循环等待条件：当多个执行流同时满足“请求与保持”和“不剥夺”条件时，会形成锁需求的闭环。例如，执行流 `A` 需要锁 `b` 才能继续运行，而锁 `b` 被执行流 `B` 持有；`B` 需要锁 `a` 才能继续运行并释放 `b`，而锁 `a` 被 `A` 占有。由于没有外部机制打破循环，锁分配陷入彻底阻塞。这是死锁的确立阶段，也是其核心标志，因为闭环直接导致执行流的永久等待。

在实践中，我们通常从“请求与保持”和“不剥夺”条件入手避免死锁。因为完全放弃锁（即互斥条件）不仅不现实，还会破坏对临界资源的管理，而“循环等待”本质上是前两个条件在锁这一抽象模型上的累积结果。因此，合理设计锁的请求与释放策略，是管理临界资源、防止死锁的关键。

在代码设计中，防范死锁主要从以下两方面入手，通过合理的逻辑和技巧降低死锁发生的风险：

- 避免锁未释放的场景：理论上，代码中应包含锁的释放逻辑，但实际情况中，锁可能因各种原因未能释放。例如，执行流阻塞导致无法执行释放操作，或异常抛出跳过了释放代码。为应对这些情况：
  - 对于阻塞问题，可通过优化代码逻辑避免死锁。例如，使用 `pthread_mutex_trylock` 替代阻塞式锁请求，它在无法获取锁时立即返回，允许执行流释放已有锁后再尝试，避免陷入僵局。
  - 对于异常跳跃问题，可借助代码技巧确保锁释放。例如，使用 `RAII`（资源获取即初始化）机制，通过对象生命周期自动管理锁，无论是否发生异常，都能保证锁被正确释放。
- 资源一次性分配：将多个临界资源集中管理，供执行流一次性访问和分配。这种方式在逻辑上减少了资源请求的分散性与复杂性，避免了因逐步获取资源而形成循环等待。例如，将所需资源整合为一组，在执行流开始时统一分配，使用完毕后一次性释放，从而降低死锁可能性。

通过以上方法，可以从代码层面有效减少“请求与保持”和“循环等待”条件的发生，进而防范死锁。

死锁作为一个经典且持久的话题，在并发编程中备受关注。目前，已有完全成熟的解决算法，例如死锁检测算法和银行家算法。这些算法本质上是通过代码技巧，对死锁防范思路的进一步发散和深化。然而，对于初学者来说，深入研究这些算法可能并非当务之急。我们需要优先掌握死锁的基本概念和防范思路，理解其本质后再逐步扩展：

- 死锁检测算法（了解即可）：用于识别系统中是否已发生死锁，通常通过资源分配图分析循环等待。此算法适用于事后诊断，但不直接预防死锁。
- 银行家算法（了解即可）：通过预判资源分配的安全性，提前避免死锁发生。它更偏向预防，但实现复杂，需熟悉系统状态的动态管理。

作为初学者，我们不必急于钻研这些算法的细节。当前阶段，理解死锁的成因和基本应对策略更为重要，后续学习中再逐步发散到具体算法。

## 线程同步

在多线程环境中，临界资源只有一份，而线程却有多个，这不可避免地引发线程对临界资源的激烈竞争。上文讨论的死锁，实际上是这种竞争极端化的具象体现：每个线程都在“卷”，拼命抢夺资源，却不清楚自己在争夺什么——大家都在抢蛋糕，但没人愿意去做蛋糕。这种无序的争抢不仅导致资源分配僵局，还衍生出一种“劣币驱逐良币”的现象：那些试图跳出竞争、创造价值的线程（想要做蛋糕的人），往往被嘲笑为“不切实际”，被挤出局。

死锁的本质，正是这种极端竞争的缩影。线程们在锁的束缚下互相等待，形成闭环，最终陷入停滞。这种局面映射到更大的语境中，仿佛文明在激烈的内部竞争中走向慢性死亡——我们尚未拥抱群星的广阔未来，就已在摇篮中溺亡。死锁不仅仅是技术层面的话题.

在多线程环境中，同步的作用在于建立更为均衡的竞争关系。同步不仅协调线程之间的资源访问，还涵盖线程与临界资源之间的交互。通过锁或其他机制，同步确保多个线程有序访问临界资源，避免无序争抢引发的混乱与冲突；同时，当临界资源尚未准备好时，同步机制会阻塞线程，直到资源可用。这种均衡并非单纯限制竞争，而是引导其向良性方向发展：线程不再盲目抢占，而是遵循规则有序等待和访问。在此过程中，竞争衍生出合作性——线程之间通过共享规则相互配合，线程与资源通过状态同步实现协调，确保系统整体高效运行，避免陷入各自为战的僵局。同步的本质，是在竞争中注入有序性，最终实现资源利用的最大化与系统稳定性的提升。

在 Linux 中，条件变量（Condition Variable）是一种体现同步有序性的解决方案。作为 Linux 原生线程库提供的机制，条件变量形象地维护了一个线程阻塞队列。当临界资源被某线程占用时，它将其他线程置于阻塞状态，并遵循“先进先出”的顺序性；一旦资源可用，队列中最前的线程会被唤醒，继续访问资源。在代码中，条件变量对应类型为 `pthread_cond_t` 的对象，用户通过操作该对象控制其行为。然而，由于条件变量的机制较为复杂，非原子化操作，必须搭配锁（如互斥锁）保护其访问，以确保线程安全和逻辑正确性。

下面让我们认识具体的接口,  由于条件变量与互斥锁都为原生线程库的一部分, 所以它们的接口形式十分类似

```cpp
// 条件变量的销毁与创建
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_init(pthread_cond_t *restrict cond,
                      const pthread_condattr_t *restrict attr);
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
```

```cpp
// 将线程加入阻塞队列
int pthread_cond_wait(pthread_cond_t *restrict cond,
                      pthread_mutex_t *restrict mutex);
```

```cpp
// 进行线程唤醒
int pthread_cond_broadcast(pthread_cond_t *cond);    // 唤醒队列中的所有线程
int pthread_cond_signal(pthread_cond_t *cond);		// 只唤醒一个线程, 默认最前面的
```

下面我们具体简单使用一下:

```cpp
#include <cstdio>
#include <vector>
#include <unistd.h>
#include <cstdint>
#include <pthread.h>

using namespace std;

// 在抢票机制中加入线程同步

pthread_mutex_t lock = PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 模拟最开始资源未准备好访问的状态
int tickets = 0;

void *pthread_behavior(void *args__)
{
    pthread_detach(pthread_self());
    // 这里使用的是传值拷贝, 为了适应void*的大小, 特地用了uint64_t
    // 也就是说, 你可以把void*当做无符号64位整型作为参数传参
    uint64_t id = (uint64_t)args__;
    while (true)
    {
        pthread_mutex_lock(&lock);
        pthread_cond_wait(&cond, &lock);
        if (tickets > 0)
        {
            printf("[%s-%llu]info: %s, %s:%d\n", "thread", id, "Buy a ticket", "remaining tickets", tickets);
            --tickets;
            pthread_mutex_unlock(&lock);
        }
        else
        {
            pthread_mutex_unlock(&lock);
            pthread_exit(nullptr);
        }
    }
}

int main()
{
    for (uint64_t i = 1; i <= 5; ++i)
    {
        pthread_t thread;
        pthread_create(&thread, nullptr, pthread_behavior, (void *)i);
        // 让阻塞队列更有序
        usleep(10);
    }

    // 初始化临界资源
    pthread_mutex_lock(&lock);
    printf("[%s-0]info: %s\n", "thread", "Initial resource allocation in progress...");    // 标准输出也是一种临界资源 上锁更安全
    tickets = 100;
    pthread_mutex_unlock(&lock);

    while (true)
    {
        usleep(1000);
        // 唤醒一个阻塞线程
        pthread_cond_signal(&cond);
    }
    return 0;
}

```

新线程被创建出来后会尝试获得锁, 获取成功后便会进入条件变量的阻塞队列中, `pthread_cond_wait()`有两个参数, 我们要说的是第二个`pthread_mutex_t*`, 为什么会有这个参数呢?  这个参数就是为了防止死锁的出现, 进入阻塞队列的线程, 会释放自身获得的锁, 这样第二个线程才能进来继续加入阻塞队列中. 线程创建的`usleep`只是为了让阻塞队列中的线程序列更有序而已, 但在实际中, 我们并没有这种需求, 所以可以完全去除.

线程行为参数使用的不是以往的指针传参, 而是传值拷贝, 这里就是单纯把`void*`的八个比特位当做存储数据本体的空间, 将编号直接传入, 直接识别, 为此, 序号类型也应有八比特位的大小, 所以使用了`uint64_t`, 即`unsigned long int`.

`pthread_cond_wait(&cond, &lock);`不是一个原子性操作, 所以它必须要被锁保护着, 在`if`前进入阻塞队列是为了保证临界资源允许访问后再对其进行判断, 在这份代码中, 我们将票数最开始设置为`0`, 因此最开始副线程不能直接抢票(这是从逻辑上来讲的, 票数为零, 那就不应该抢票, 所以条件变量可以拦住线程, 让它抢不了), 而在后来, 在主线程已经将临界资源初始完后, 唤醒阻塞队列中的线程, 让它们开始抢票, 如果把`tickets`看做容器, 将`ticket`看做其中的元素, 那所谓的临界资源未准备好即表示容器为空, 没有可供处理的元素, 所以先让副线程等待, 而当有一个元素进入后, 便意味着容器有元素了, 此时便可以发出线程激活信号, 唤醒一个线程处理数据.

此时运行程序, 便会发现其抢票信息具有周期性, 都是按照`1, 2, 3, 4, 5`的顺序轮流抢票.  这种周期性是用队列形成的, 具有确定性, 不像之前用的`usleep`, 或者所谓的"对抢到票做处理的后续环节".

## 生产消费者模型Ⅰ

下面我们来谈所谓的"生产消费者模型", `Producer-Consumer Model`, 即`cp`问题.

有一个超市, 为了简化模型, 我们认为这个超市只卖一种商品, 卖什么呢? 卖鸡蛋吧,  据说美国那边鸡蛋价格飞涨, 而且数量也比较稀少,    在超市之外, 还有两种角色, 一种是消费者, 另一种是生产者, 为什么要有超市这种存在呢? 超市实际上就是一种资源池, 充当着缓冲的角色,  平时超市可以多屯一点鸡蛋, 毕竟美国人对鸡蛋有很旺盛的需求, 短时间鸡蛋产量下降, 超市可以调整策略, 对鸡蛋的价格进行一定的调高, 保证绝大多数消费者还是可以买到鸡蛋的,   超市也一定程度上实现了生产者与消费者行为的解耦, 作为生产者, 我不需要关心哪里消费者多, 自己亲自去售卖, 我只要给超市供货就行, 作为消费者, 我不在乎这是从个农场来的鸡蛋, 我只要能吃到就行了,  消费者和生产者的行为是可以独立并发处理的, 他们并不在乎对方在干什么

对于计算机来说, 生产者和消费者都由线程来扮演, 超市则是双方共同使用的, 具有特定结构的内存空间,  生产者生产数据, 将数据扔进超市, 消费者从超市里拿到数据, 进行进一步处理, 生产者知道, 只要把数据扔进超市, 就会有消费者来处理, 它不在乎具体是哪个消费者, 消费者也不在乎这数据是从哪来的, 它只要依据要求对数据进行处理就行.  整个过程, 在本质上就是执行流在做通信

其中, 生产者与生产者之间是竞争关系, 延伸为互斥关系, 作为生产者, 我当然希望直接垄断, 这样利益最大化, 当生产者往存储临界资源的容器里插入新元素的过程中, 不能又来一个生产者插入元素, 

生产者与消费者之间首先要有同步关系, 生产者要先生产出数据, 消费者才可以得到数据, 它们交替进行, 具有周期性, 其次, 消费者与生产者之间也要有互斥关系, 显而易见, 一个链表不能同时插入删除节点.

消费者之间则是互斥关系, 这盒鸡蛋, 要么被你买走, 要么被他买走, 不存在中间情况.

生产者和消费者不解耦是什么情况, 其实就是函数调用, 生产者调用一个函数, 必须等到消费者处理完毕, 生产者才能继续运行, 其实就是单执行流串行处理,对于生产消费模型来说, 生产者可以直接发送函数对象和参数, 交给消费者执行.

下面我们写一个阻塞队列, 该队列的特点是, 当队列为空时, 从队列中获取元素的操作将会被阻塞, 直到队列中又被放入了元素, 当队列满时, 往队列中存放元素的操作也会被阻塞, 直到有元素被从队列中取出. 

我们的阻塞队列是在`std::queue`的基础上包装得来的, 其中有两个核心接口, `emplace`(其实就是`push`)和`front`, 其中的成员变量`_max_size`表示我们设定的队列最大可容纳元素个数, 无论是`emplace`还是`front`, 其中都涉及到对临界资源的访问, 所以上来的第一件事就是上锁,  随后对`queue`的元素个数进行检查, 对于`emplace`来说, 如果已经达到我们设定的最大元素个数, 那就进入信号量`_not_full`的阻塞队列, 如果没有达到, 则正常插入新数据, 并打印信息,  插入一个新数据意味着现在容器一定不是空的, 所以唤醒在`_not_empty`中的线程, 让其去读取新数据, `front`读出一个数据意味着现在容器中又少了一个元素, 所以就可以唤醒`_not_full`中的线程, 让它插入新的数据.

```cpp
#include<pthread.h>
#include<queue>
#include<utility>
#include<cstdio>

template<class T>
class blocking_queue
{
    private:
    typedef std::queue<T> queue;

    pthread_mutex_t _lock;
    pthread_cond_t _not_full;
    pthread_cond_t _not_empty;
    size_t _max_size;
    queue _q;

    public:
    blocking_queue(size_t max_size = 5)
    {
        pthread_mutex_init(&_lock, nullptr);
        pthread_cond_init(&_not_full, nullptr);
        pthread_cond_init(&_not_empty, nullptr);
        _max_size = max_size;
    }

    ~blocking_queue()
    {
        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_not_full);
        pthread_cond_destroy(&_not_empty);
    }

    template<class... Args>
    void emplace(Args&&... args)
    {
        pthread_mutex_lock(&_lock);
        if(_q.size() == _max_size)
        {
            pthread_cond_wait(&_not_full, &_lock);
        }
        _q.emplace(std::forward<Args>(args)...);
        printf("[producer]info: Push a piece of data: %d\n", args...);
        pthread_cond_signal(&_not_empty);
        pthread_mutex_unlock(&_lock);
    }

    T front()
    {
        pthread_mutex_lock(&_lock);
        if(_q.size() == 0)
        {
            pthread_cond_wait(&_not_empty, &_lock);
        }
        T result = _q.front();
        printf("[consumer]info: Received a piece of data: %d\n", result);
        _q.pop();
        pthread_cond_signal(&_not_full);
        pthread_mutex_unlock(&_lock);
        return result;
    }

};


#include"blocking_queue.hpp"
#include<unistd.h>

using namespace std;

// pthread_mutex_t lock = PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

void* producer(void* _p)
{
    blocking_queue<int>* q = static_cast<blocking_queue<int>*>(_p);
    int data = 0;
    while(true)
    {
        q->emplace(data++);
        // pthread_mutex_lock(&lock);
        // printf("[producer]info: Push a piece of data: %d\n", data++);
        // pthread_mutex_unlock(&lock);
    }
}

void* consumer(void* _p)
{
    blocking_queue<int>* q = static_cast<blocking_queue<int>*>(_p);
    while(true)
    {
        auto data = q->front();
        // pthread_mutex_lock(&lock);
        // printf("[consumer]info: Received a piece of data: %d\n", data);
        // pthread_mutex_unlock(&lock);
        // 让它们打印慢一点, 方便我复制执行结果
        sleep(2);
    }
}

int main()
{
    pthread_t p,c;
    auto _p = new blocking_queue<int>();
    pthread_create(&p, nullptr, producer, (void*)_p);
    pthread_create(&c, nullptr, consumer, (void*)_p);

    pthread_join(p, nullptr);
    pthread_join(c, nullptr);

    return 0;
}
```

```shell
[wind@starry-sky synchronization]$ g++ test.cc -lpthread -o out
[wind@starry-sky synchronization]$ ./out
[producer]info: Push a piece of data: 0
[producer]info: Push a piece of data: 1
[producer]info: Push a piece of data: 2
[producer]info: Push a piece of data: 3
[producer]info: Push a piece of data: 4
[consumer]info: Received a piece of data: 0
[producer]info: Push a piece of data: 5
[consumer]info: Received a piece of data: 1
[producer]info: Push a piece of data: 6
[consumer]info: Received a piece of data: 2
[producer]info: Push a piece of data: 7
^C
[wind@starry-sky synchronization]$
```

我们让消费者每次读出一个数据就`sleep(2)`, 这导致数据的读出很慢, 所以生产者最开始会一下子就插入五个数据, 并在插入第六个数据的过程中被阻塞住, 等到消费者读出数据后, 便会唤醒生产者, 于是生产者立刻又插入了一个新数据, 并再次陷入阻塞. 等待消费者消费数据.

另外你应该还能看出来, 打印内容最开始是放在外面的, 但后来放到里面了, 这是因为放到外面容易产生误导:  看起来就像是一次性生产六个一样, 但实际上, 应该是生产者有先发优势, 可以认为它早早就差一步就能把第六个数据插入了, 在消费者获取到数据并释放锁后, 生产者立刻结束, 抢先把内容打印出来了, 大概效果就是这样(下面这张图是之后写的代码, 所以看起来感觉完全不一样)

![image-20251103233846590](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251103233846590.png)    

```shell
[wind@starry-sky synchronization]$ ./out
[producer]info: Push a piece of data: 0
[producer]info: Push a piece of data: 1
[producer]info: Push a piece of data: 2
[producer]info: Push a piece of data: 3
[producer]info: Push a piece of data: 4
[producer]info: Push a piece of data: 5
[consumer]info: Received a piece of data: 0
[consumer]info: Received a piece of data: 1
[producer]info: Push a piece of data: 6
[consumer]info: Received a piece of data: 2
[producer]info: Push a piece of data: 7
```

如果是生产者慢的话, 消费者就会先进入阻塞, 等待生产者

```cpp
void* producer(void* _p)
{
    blocking_queue<int>* q = static_cast<blocking_queue<int>*>(_p);
    int data = 0;
    while(true)
    {
        sleep(2);
        q->emplace(data++);
    }
}

void* consumer(void* _p)
{
    blocking_queue<int>* q = static_cast<blocking_queue<int>*>(_p);
    while(true)
    {
        auto data = q->front();
    }
}

```

```shell
[wind@starry-sky synchronization]$ g++ test.cc -lpthread -o out
[wind@starry-sky synchronization]$ ./out
[producer]info: Push a piece of data: 0
[consumer]info: Received a piece of data: 0
[producer]info: Push a piece of data: 1
[consumer]info: Received a piece of data: 1
[producer]info: Push a piece of data: 2
[consumer]info: Received a piece of data: 2
[producer]info: Push a piece of data: 3
[consumer]info: Received a piece of data: 3
[producer]info: Push a piece of data: 4
[consumer]info: Received a piece of data: 4
[producer]info: Push a piece of data: 5
[consumer]info: Received a piece of data: 5
[producer]info: Push a piece of data: 6
[consumer]info: Received a piece of data: 6
[producer]info: Push a piece of data: 7
[consumer]info: Received a piece of data: 7
[producer]info: Push a piece of data: 8
[consumer]info: Received a piece of data: 8
[producer]info: Push a piece of data: 9
[consumer]info: Received a piece of data: 9
[producer]info: Push a piece of data: 10
[consumer]info: Received a piece of data: 10
[producer]info: Push a piece of data: 11
[consumer]info: Received a piece of data: 11
[producer]info: Push a piece of data: 12
[consumer]info: Received a piece of data: 12
[producer]info: Push a piece of data: 13
[consumer]info: Received a piece of data: 13
[producer]info: Push a piece of data: 14
[consumer]info: Received a piece of data: 14
^C
[wind@starry-sky synchronization]$
```

实际上, 我们还可以定义一个区间, 当元素个数已经变为左区间时, 就给生产者发信号, 让它赶紧去生产数据, 当元素个数已经变为右区间时, 就给消费者发信号, 让它赶紧消费数据, 如果在区间中, 那便不去管理.

```cpp
// 我后来看了一下这份代码有可能会引发段错误
// 你可以再翻一点, 程序运行展示之后有一个封装更好看的
// 写的更严谨的实现, 但需要一些C++的基础知识
template <class T>
class blocking_queue
{
private:
    typedef std::queue<T> queue;
    typedef size_t interval;

    pthread_mutex_t _lock;
    pthread_cond_t _not_full;
    pthread_cond_t _not_empty;
    size_t _max_size;
    interval _left;
    interval _right;
    queue _q;

public:
    blocking_queue(size_t max_size = 12)
    {
        pthread_mutex_init(&_lock, nullptr);
        pthread_cond_init(&_not_full, nullptr);
        pthread_cond_init(&_not_empty, nullptr);
        _max_size = max_size;
        _left = _max_size / 3;
        _right = _max_size * 2 / 3;
    }

    ~blocking_queue()
    {
        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_not_full);
        pthread_cond_destroy(&_not_empty);
    }

    template <class... Args>
    void emplace(Args &&...args)
    {
        pthread_mutex_lock(&_lock);
        if (_q.size() == _max_size)
        {
            pthread_cond_wait(&_not_full, &_lock);
        }
        _q.emplace(std::forward<Args>(args)...);
        printf("[producer]info: Push a piece of data: %d\n", args...);
        if (_q.size() == _right)
        {
            pthread_cond_signal(&_not_empty);
        }
        pthread_mutex_unlock(&_lock);
    }

    T front()
    {
        pthread_mutex_lock(&_lock);
        if (_q.size() == 0)
        {
            pthread_cond_wait(&_not_empty, &_lock);
        }
        T result = _q.front();
        printf("[consumer]info: Received a piece of data: %d\n", result);
        _q.pop();
        if (_q.size() == _left)
        {
            pthread_cond_signal(&_not_full);
        }
        pthread_mutex_unlock(&_lock);
        return result;
    }
};
```

```shell
[wind@starry-sky synchronization]$ ./out
[producer]info: Push a piece of data: 0
[producer]info: Push a piece of data: 1
[producer]info: Push a piece of data: 2
[producer]info: Push a piece of data: 3
[producer]info: Push a piece of data: 4
[producer]info: Push a piece of data: 5
[producer]info: Push a piece of data: 6
[producer]info: Push a piece of data: 7
[consumer]info: Received a piece of data: 0
[consumer]info: Received a piece of data: 1
[consumer]info: Received a piece of data: 2
[consumer]info: Received a piece of data: 3
[consumer]info: Received a piece of data: 4
[consumer]info: Received a piece of data: 5
[consumer]info: Received a piece of data: 6
[consumer]info: Received a piece of data: 7
[producer]info: Push a piece of data: 8
[producer]info: Push a piece of data: 9
[producer]info: Push a piece of data: 10
[producer]info: Push a piece of data: 11
[producer]info: Push a piece of data: 12
[producer]info: Push a piece of data: 13
[producer]info: Push a piece of data: 14
[producer]info: Push a piece of data: 15
[consumer]info: Received a piece of data: 8
[consumer]info: Received a piece of data: 9
[consumer]info: Received a piece of data: 10
[consumer]info: Received a piece of data: 11
[consumer]info: Received a piece of data: 12
[consumer]info: Received a piece of data: 13
[consumer]info: Received a piece of data: 14
[consumer]info: Received a piece of data: 15
[producer]info: Push a piece of data: 16
[producer]info: Push a piece of data: 17
^C
[wind@starry-sky synchronization]$
```

理论上来说, 消费者不会一口气把数据消费完, 但我们这里由于生产者是`sleep(1)`的, 所以最后消费者还是把数据都消费完后才阻塞停止, 我们也可以让消费者`sleep(1)`, 在这里, 左右区间的感觉就很明显了

```sell
[wind@starry-sky synchronization]$ ./out
[producer]info: Push a piece of data: 0
[producer]info: Push a piece of data: 1
[producer]info: Push a piece of data: 2
[producer]info: Push a piece of data: 3
[producer]info: Push a piece of data: 4
[producer]info: Push a piece of data: 5
[producer]info: Push a piece of data: 6
[producer]info: Push a piece of data: 7
[producer]info: Push a piece of data: 8
[producer]info: Push a piece of data: 9
[producer]info: Push a piece of data: 10
[producer]info: Push a piece of data: 11
[consumer]info: Received a piece of data: 0
[consumer]info: Received a piece of data: 1
[consumer]info: Received a piece of data: 2
[consumer]info: Received a piece of data: 3
[consumer]info: Received a piece of data: 4
[consumer]info: Received a piece of data: 5
[consumer]info: Received a piece of data: 6
[consumer]info: Received a piece of data: 7
[producer]info: Push a piece of data: 12
[producer]info: Push a piece of data: 13
[producer]info: Push a piece of data: 14
[producer]info: Push a piece of data: 15
[producer]info: Push a piece of data: 16
[producer]info: Push a piece of data: 17
[producer]info: Push a piece of data: 18
[producer]info: Push a piece of data: 19
[consumer]info: Received a piece of data: 8
[consumer]info: Received a piece of data: 9
[consumer]info: Received a piece of data: 10
[consumer]info: Received a piece of data: 11
[consumer]info: Received a piece of data: 12
[consumer]info: Received a piece of data: 13
[consumer]info: Received a piece of data: 14
[consumer]info: Received a piece of data: 15
[producer]info: Push a piece of data: 20
[producer]info: Push a piece of data: 21
[producer]info: Push a piece of data: 22
[producer]info: Push a piece of data: 23
[producer]info: Push a piece of data: 24
[producer]info: Push a piece of data: 25
[producer]info: Push a piece of data: 26
[producer]info: Push a piece of data: 27
[consumer]info: Received a piece of data: 16
^C
[wind@starry-sky synchronization]$
```

```cpp
// #define USE_CLASSIC_COND_SCHEME

#include <pthread.h>
#include <unistd.h>

#include <format>
#include <iostream>
#include <memory>
#include <queue>

struct lock_guard {
    pthread_mutex_t* _mutex;
    lock_guard(pthread_mutex_t* mutex) : _mutex(mutex) {
        pthread_mutex_lock(_mutex);
    };
    ~lock_guard() {
        pthread_mutex_unlock(_mutex);
    };
};

template <typename T>
class blocking_queue {
    // unique_ptr 明确生命周期, 防止因拷贝造成的生命周期异常
    using queue_t = std::queue<T>;
    using queue_ptr_t = std::unique_ptr<queue_t>;

    using mutex_t = pthread_mutex_t;
    using cond_t = pthread_cond_t;

    using mutex_ptr_t = std::unique_ptr<mutex_t, void (*)(mutex_t*)>;
    using cond_ptr_t = std::unique_ptr<cond_t, void (*)(cond_t*)>;

    using self_t = blocking_queue<T>;
    using self_ptr_t = std::unique_ptr<self_t>;

   public:
    blocking_queue(size_t max_size = 12)
#ifdef USE_CLASSIC_COND_SCHEME
        : _max_size(max_size),
#else
        : _left_size(max_size / 3),
          _right_size(max_size * 2 / 3),
          _wake_batch_size(_right_size - _left_size),
#endif
          _queue(std::make_unique<queue_t>()),
          _mutex(new mutex_t, delete_mutex_func),
          _not_full(new cond_t, delete_cond_func),
          _not_empty(new cond_t, delete_cond_func) {
        pthread_mutex_init(_mutex.get(), nullptr);
        pthread_cond_init(_not_full.get(), nullptr);
        pthread_cond_init(_not_empty.get(), nullptr);
    }

    template <typename... Args>
    void emplace(Args... args) {
        lock_guard lock(_mutex.get());

        // 理论上用 if 就可以, 但这里的while是为了防止某些异常唤醒场景
        // 在异常唤醒情况下, 尽管线程获得锁, 但可能条件仍不满足, 此时应该让它再次进入等待队列,
        // 而不是直接出去
#ifdef USE_CLASSIC_COND_SCHEME
        while (_queue->size() >= _max_size)
#else
        while (_queue->size() >= _right_size)
#endif
        {
            pthread_cond_wait(_not_full.get(), _mutex.get());
        }
        _queue->emplace(std::forward<Args>(args)...);
        std::cout << std::format("生产了一份新的数据: {}", T(std::forward<Args>(args)...))
                  << std::endl;

        // 唤醒一个或一批线程

#ifdef USE_CLASSIC_COND_SCHEME
        pthread_cond_signal(_not_empty.get());
#else
        if (_queue->size() == _right_size) {
            for (size_t i = 0; i < _wake_batch_size; ++i) {
                pthread_cond_signal(_not_empty.get());
            }
        }
#endif
    };

    // 为了保护其中的资源, 我们不能返回引用
    // 另外, 为了确保取出和移除队首元素的原子性
    // 我们需要把front和pop合并
    T front() {
        T result;
        {
            lock_guard lock(_mutex.get());
#ifdef USE_CLASSIC_COND_SCHEME
            while (_queue->size() == 0)
#else
            // _left_size 可能过小, 导致其为零
            // 所以没有等于号, 但对于这种情况
            // 我应该诘问使用者, 为什么给如此小的_max_size? 
            while (_queue->size() < _left_size)
#endif
            {
                pthread_cond_wait(_not_empty.get(), _mutex.get());
            }
            result = _queue->front();
            _queue->pop();
            std::cout << std::format("获取了一份新的数据: {}", result) << std::endl;

#ifdef USE_CLASSIC_COND_SCHEME
            pthread_cond_signal(_not_full.get());
#else
            if (_queue->size() == _left_size) {
                for (size_t i = 0; i < _wake_batch_size; ++i) {
                    pthread_cond_signal(_not_full.get());
                }
            }
#endif
        }

        return result;
    }

   private:
    static void delete_mutex_func(mutex_t* ptr) {
        pthread_mutex_destroy(ptr);
        delete ptr;
    }

    static void delete_cond_func(cond_t* ptr) {
        pthread_cond_destroy(ptr);
        delete ptr;
    }

   private:
#ifdef USE_CLASSIC_COND_SCHEME
    size_t _max_size;
#else
    size_t _left_size;
    size_t _right_size;
    const size_t _wake_batch_size;
#endif
    queue_ptr_t _queue;
    mutex_ptr_t _mutex;
    cond_ptr_t _not_full;
    cond_ptr_t _not_empty;
};
```

----

在实践中, 往往不会把`int`传来传去, 而是更喜欢传函数对象, 让消费者去执行任务.    

![image-20250304144139195](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250304144139544.png)

任务从哪里来的, 很明显是从其它外设那里得来的, 比如, 网卡中的网络请求,  用户在远端发出一个请求, 通过网络传到服务器, 被生产者接收,  生产者将这些请求包装成具体的可调用对象, 放入阻塞队列中,  消费者则从其中取出任务, 对其进行回调操作.  所以说上面的这张图画得并不完整, 甚至可以说是缺少了很关键的部分, 那就是生产者和消费者在非临界区中的行为.

![image-20250304145600932](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250304145601157.png)

为什么我说这是关键内容, 因为这正是生产消费者模型高效性的体现.   诚然,  消费者与消费者之间具有互斥性, 生产者与生产者之间也具有互斥性,  生产者与消费者之间具有互斥性与同步性,  总之, 临界区在任何时候都最多只有一个执行流在运行, 因为临界区是串行的, 所以看上去并不高效, 可为什么, 又说生产消费者模型具有高效性?

因为上面的说法太局限了,  并没有考虑到非临界区的情况. 生产者和消费者在非临界区仍然是以并行的方式运行的, 这意味着, 这个进程在同时处理着数据的接收和可执行对象的回调, 而且由于临界区代码简单, 运行所消耗的时间很短, 因此绝大多数的生产者和消费者其实都是在执行非临界区代码, 只有小部分要么被锁或者条件变量阻塞, 要么在运行临界区代码, 所以整个模型的并发度很高, 进而效率很高.   这才是生产消费者模型的关键.

而在上面的代码中, 由于我们还没有学过网络, 所以没有办法, 非临界区写不出什么代码, 临界区的粒度反而比非临界区粒度重,  因此察觉不出生产消费者模型的高效性,   或者更形象的说, 现在我们的生产消费者模型打的是逆风局,  等到了网络, 好日子就到了.(并非好日子)

下面我们把阻塞队列中的元素换成可调用对象,  来实现一个计算器:   计算器支持加减乘除四种运算,   在生产者的非临界区, 我们使用伪随机数生成运算中的两个操作数和具体的运算,   在消费者的非临界区, 我们对发过来的可调用对象进行回调,  执行相应操作, 在这里, 就是打印一下结果和可能存在的错误, 为了简化模型, 我们用的还是最开始的版本, 而不是用区间的那个.    我们将分为C++98和C++11两种风格实现, 首先实现C++98,

既然, 我们用的是C++98风格, 那就把`emplace`改成`push`吧, 不过, 在尝试之后发现还是不能在外部打日志, 所以就将就写一个对应的成员函数

```cpp
// blocking_queue.hpp  中的 push

void push(const T& val)
{
    pthread_mutex_lock(&_lock);
    if (_q.size() == _max_size)
    {
        pthread_cond_wait(&_not_full, &_lock);
    }
    _q.push(val);
    val.producer_info();
    pthread_cond_signal(&_not_empty);
    pthread_mutex_unlock(&_lock);
}

ttemplate <class... Args>
    void emplace(Args &&...args)
    {
        pthread_mutex_lock(&_lock);
        if (_q.size() == _max_size)
        {
            pthread_cond_wait(&_not_full, &_lock);
        }
        _q.emplace(std::forward<Args>(args)...);
        printf("[producer]info: Generate a task: %d %c %d = ?\n", args...);
        pthread_cond_signal(&_not_empty);
        pthread_mutex_unlock(&_lock);
    }

//  main.cc
using namespace std;

void* producer(void* _p)
{
    blocking_queue<task>* q = static_cast<blocking_queue<task>*>(_p);
    while(true)
    {
        int left = rand() % 10;
        int right = rand() % 10;
        int a = rand() % 4;
        char op;
        switch(a)
        {
            case 0: op = '+'; break;
            case 1: op = '-'; break;
            case 2: op = '*'; break;
            case 3: op = '/'; break;
        }
        // q->emplace(left, op, right);
        q->push(task(left, op, right));
        sleep(1);
    }
}

void* consumer(void* _p)
{
    blocking_queue<task>* q = static_cast<blocking_queue<task>*>(_p);
    while(true)
    {
        auto task = q->front();
        auto result = task();
        cout << "[consumer]info: ";
        if(result.second == 0)
            cout << "The result is "<< result.first<<endl;
        else
            cout << "error! "<<strerror(result.second)<<endl;
    }
}


// task.hpp
class task
{
    public:
    task(int left, char op, int right)
        : _left(left), _op(op), _right(right)
        {
        }

    std::pair<int, int> operator()()
    {
        int _result = 0;
        int _errno = 0;
        switch (_op)
        {
            case '+':
                _result = _left + _right;
                break;
            case '-':
                _result = _left - _right;
                break;
            case '*':
                _result = _left * _right;
                break;
            case '/':
                if (_right == 0)
                    _errno = 1;
                else
                    _result = _left / _right;
                break;
        }
        return std::pair<int, int>(_result, _errno);
    }

    void producer_info()const
    {
        std::cout << "[producer]info: Generate a task: "<< _left << " " << _op << " " << _right << " = ?" <<std::endl;
    }

    private:
    int _left;   // 左操作数
    char _op;     // 操作符
    int _right;  // 右操作数
};
```

来看看效果:

```shell
[wind@starry-sky synchronization]$ ./w.out
[producer]info: Generate a task: 1 - 7 = ?
[consumer]info: The result is -6
[producer]info: Generate a task: 8 + 3 = ?
[consumer]info: The result is 11
[producer]info: Generate a task: 5 * 7 = ?
[consumer]info: The result is 35
[producer]info: Generate a task: 5 * 9 = ?
[consumer]info: The result is 45
[producer]info: Generate a task: 1 - 0 = ?
[consumer]info: The result is 1
[producer]info: Generate a task: 6 - 4 = ?
[consumer]info: The result is 2
[producer]info: Generate a task: 8 * 7 = ?
[consumer]info: The result is 56
[producer]info: Generate a task: 6 / 3 = ?
[consumer]info: The result is 2
[producer]info: Generate a task: 8 - 9 = ?
[consumer]info: The result is -1
[producer]info: Generate a task: 6 + 2 = ?
[consumer]info: The result is 8
[producer]info: Generate a task: 0 / 0 = ?
[consumer]info: error! Operation not permitted
[producer]info: Generate a task: 0 * 6 = ?
[consumer]info: The result is 0
^C
[wind@starry-sky synchronization]$
```

 C++11不写了, `lambda`, `function`, `bind`,  稍微改改形式

尽管C++11没有写, 但后来我又用上述那个"封装更好的"阻塞队列重新实现了一下, 我个人觉得写的有些太过复杂了, 不太合适理解, 代码也挺多, 这里就粘贴了.不过效果可以展示一下, 他用的就是区间唤醒的方法

![image-20251104213802803](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251104213802803.png)

现在让我们稍微回过头来, 看看我们代码中一些悬而未决的问题, 在阻塞队列的生产者和消费者里, 它们对临界资源进行正式插入或取出数据之前, 都会判断临界资源是否处于准备好访问的状态. (更优封装的版本没有这个问题)

```cpp
if (_q.size() == _max_size)
{
    pthread_cond_wait(&_not_full, &_lock);
}

if (_q.size() == 0)
{
    pthread_cond_wait(&_not_empty, &_lock);
}
```

但实际上, 这两份代码是存在一定问题的, 这就是所谓的"伪唤醒",  即线程在本不该(临界资源未准备好)的时候被意外唤醒,  从而从  `pthread_cond_wait`中返回继续运行.

这种意外出现的原因有很多, 比如, 这可能是对面使用`pthread_cond_broadcast`唤醒所有线程造成的, 有可能唤醒的太多了, 导致意外发生, 在条件不满足的情况下继续执行临界区的后续代码. 在新版本的阻塞队列中, 你可以看到, 我批量唤醒线程用的也不是这个, 而是通过循环若干次`pthread_cond_signal`的方式实现的, 所防范的就是这种情况, 如果信号量中已经没有现成, 那么继续唤醒将不会有任何反应.

另外, 无论对于`pthread_cond_broadcast`还是`pthread_cond_signal`, 其逻辑都是先将线程(们)唤醒, 然后并不是让它们直接返回继续执行后续代码, 而是先争夺锁, 拿到锁之后再返回. 对于`pthread_cond_broadcast`来说, 他首先换唤醒了所有的线程, 为方便起见, 我们假设这些线程都是生产者. 其中一个线程成功争夺到了锁, 它朝队列中又插入了一个数据, 然后释放锁, 进入了非临界区. 在这种情况下, 就可能会引发这种意外: 刚刚这个线程恰好所插入的数据恰好让队列达到了我们的最大设计容量, 也就是`_max_size`, 可是, 在它释放完锁后, 锁又被一个生产者抢到了, 此时它就会退出`pthread_cond_broadcast`, 继续插入数据, 从而造成队列中的实际数据量大于我们所预想的最大个数.

换成`while`的好处在于, 即使真的发生这种意外, 也会因为while的循环性条件判断逻辑而再一次进入信号量, 避免队列的行为超出我们之前的预想.

你可能会说, 反正就是因为对面用了`pthread_cond_broadcast`而造成伪唤醒的, 那我不用不就行了, 但需要注意的是, 上面所说的只是伪唤醒的一种原因, 它还可能由其它原因造成, 比如`pthread_cond_wait`失败了(非常小的可能性, 偶尔一次两次), 总之为了保险起见, 我们要把`if`改成`while`, 这样即使真的出现伪唤醒, 它也会因为`while`循环而重新运行`pthread_cond_wait`进而再次被阻塞住, 

在解决了伪唤醒问题后, 我们就可以正式开始多生产者, 多消费者的代码了.

怎么把上面的代码改成多生产多消费呢? 其实很简单, 就是多创建几个线程, 除此之外就几乎没有要改的了, 因为多生产和多消费的唯一区别就是之前我们说的这张图中的非临界区域变成并行处理了, 非临界区变成并行就变成并行呗,  不会有什么问题, 而且甚至是好事, 增加了并行度, 效率更高了, 至于临界区, 在我们处理完伪循环问题后和之前的单生产单消费相同, 还是任何时候都最多只能有一个线程运行, 没有发生变化.

![image-20250304145600932](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250304145601157.png)

```cpp
int main()
{
    srand((unsigned int)time(nullptr));
    blocking_queue<task> critical;
    auto _p = &critical;

    pthread_t p[5], c[5];
    for(int i = 0; i < 5; ++i)
    {
        pthread_create(p + i, nullptr, producer, (void*)_p);
    }

    for(int i = 0; i < 5; ++i)
    {
        pthread_create(c + i, nullptr, consumer, (void*)_p);
    }

    // 线程分离, 不等待了
    while(true)
    {
        usleep(100);
    }

    return 0;
}
```

除此之外, 还在日志中添加了线程`id` , 将`if`改成`while`, 两个线程行为函数前面加了个`pthread_detach`之外就没有了.

```shell
[wind@starry-sky synchronization]$ ./w.out
[producer]<139699557517056>info: Generate a task: 5 + 6 = ?
[producer]<139699540731648>info: Generate a task: 2 + 9 = ?
[producer]<139699549124352>info: Generate a task: 3 / 5 = ?
[producer]<139699523946240>info: Generate a task: 2 * 3 = ?
[consumer]<[consumer]<139699481982720139699507160832>info: >info: The result is The result is 1111
[producer]<139699532338944>info: Generate a task: 7 / 3 = ?
[consumer]<139699515553536>info: The result is 0
[consumer]<139699515553536>info: The result is 6
[consumer]<139699515553536>info: The result is 2

[producer]<139699557517056>info: Generate a task: 8 + 7 = ?
[consumer]<139699515553536>info: The result is 15
[producer]<139699540731648>info: Generate a task: 2 * 2 = ?
[consumer]<139699498768128>info: The result is 4
[producer]<139699549124352>info: Generate a task: 6 / 7 = ?
[consumer]<139699490375424>info: The result is 0
[producer]<139699523946240>info: Generate a task: 4 + 4 = ?
[consumer]<139699481982720>info: The result is 8
[producer]<139699532338944>info: Generate a task: 2 * 1 = ?
[consumer]<139699507160832>info: The result is 2
[producer]<139699557517056>info: Generate a task: 2 * 9 = ?
[consumer]<139699515553536>info: The result is 18
[producer]<139699540731648>info: Generate a task: 4 - 3 = ?
[consumer]<139699498768128>info: The result is 1
[producer]<139699523946240>info: Generate a task: 5 + 6 = ?
[consumer]<139699490375424>info: The result is 11
[producer]<139699549124352>info: Generate a task: 4 / 6 = ?
[consumer]<139699481982720>info: The result is 0
[producer]<139699532338944>info: Generate a task: 5 + 8 = ?
[consumer]<139699507160832>info: The result is 13
[producer]<139699557517056>info: Generate a task: 1 + 8 = ?
[consumer]<139699515553536>info: The result is 9
[producer]<139699540731648>info: Generate a task: 0 * 1 = ?
[consumer]<139699498768128>info: The result is 0
[producer]<139699523946240>info: Generate a task: 4 / 7 = ?
[consumer]<139699490375424>info: The result is 0
[producer]<139699532338944>info: Generate a task: 1 / 2 = ?
[consumer]<139699481982720>info: The result is 0
[producer]<139699549124352>info: Generate a task: 3 / 6 = ?
[consumer]<139699490375424>info: The result is 0
^C
[wind@starry-sky synchronization]$ 
```

有些打印是乱的, 这是因为我们没有把`consumer`的打印用锁保护, 现在还不急着管.

## 生产消费者模型Ⅱ

上文的生产消费模型是基于条件变量的, 现在让我们再认识一个基于信号量的

我们曾在进程中通信稍微抽象地提了信号量,   我们说, 信号量可以看成一个计数器, 这个计数器指示的是临界资源的份数, 临界资源本身并不能真正的进行并发访问, 但可以将临界资源分成独立的多份,    每一份可以进行串行访问, 所以在外部看来, 这个临界资源就有并发处理的感觉, 这种做法可以提高程序的并发度, 这样就能变得更加高效了.

在上面的生产消费模型中, 我们把`queue`当做一个整体来用, 它不能再继续往下分了, 再往下继续分虽然也能分出来, 但它们是不独立的,   其中一个状态发生变化就会影响其它份, 所以我们在`queue`上加了锁.

而现在, 我们可以拿一个可以容纳300个元素的全局数组, 把它均分为三份, 通过某种机制, 让任意时刻最多只有三个线程被放进临界区访问临界资源, 这样就能做到对一个临界资源的并发访问. 如何实现这种机制呢? 那就需要使用信号量来实现. 互斥锁就是一种特殊的信号量, 它每次都只会放一个线程进临界区, 所以它也被叫做二元信号量(要么为零, 要么为一).

对于临界资源进行并发访问有两点需要满足, 一是对于临界资源本身来说, 它的组织形式能够支持其被分成独立的, 不会相互干扰的块,  并且存在合理的分配机制, 让线程进入临界区后获得对应的块,   二是,  信号量要守好临界区的大门, 只允许一定数目的线程进入,   我们的代码所控制的就是第一点, 第二点那就是信号量的事了.

拿电影院举例,   买票就相当于申请信号量, 买到票就相当于获得信号量, 买到票意味着电影院中的某个座位被你预定了,  获得信号量意味着临界资源中的某一小份被你预定, 每买一张电影票,  剩下的电影票数就会减一,   每有一个线程获得了一个信号量, 信号量就会减一,   票数为零就意味着里面座位满了, 容不下人了, 你也不能进入,  信号量为零意味着可供分配的块已经没了, 你进来也只能捣乱, 所以不能让你进入临界区.   有人退电影票了,   那就又剩下一个座位, 有人来买就能进去,   一个线程干完活了, 返还信号量, 信号量就会加一, 如果一个线程竞争到了信号量, 那它就可以进入临界区.

信号量本身作为一种临界资源, 支持原子性地加加减减操作, 它能保护自己, 所以可以看守临界区的大门, 此时对临界资源是否准备完毕的检查就不需要真的直接访问临界资源, 而只要看信号量就可以了,  信号量的初始化值就是临界资源最大的可供分配块数, 信号量既然放这个线程进来, 那就必然意味着现在临界资源是准备好的, 我们不需要再在临界区里判断临界资源状态, 这样就省了一些代码, 临界区跑的越快越好, 结构越简单越好.

信号量接口很简单

```cpp
#include <semaphore.h>

// sem: 信号量对象     pshared:  为零,线程间共享, 不为零进程共享    value:信号量初始值 
int sem_init(sem_t *sem, int pshared, unsigned int value);

// 对信号量减一, 简称为`P()`
int sem_wait(sem_t *sem);

// 对信号量加一, 简称为`V()`
int sem_post(sem_t *sem);

// 销毁信号量
int sem_destroy(sem_t *sem);
```

下面我们就以基于环形队列的生产消费者模型来使用信号量.

环形队列我们之前说过, 不过已经很长时间了, 其在逻辑上是个环, 遵循着先进先出的原则

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202408131339375.mp4"></video>

其在物理层, 可以以数组的方式实现, 每入一个数据, `rear`加加一次, 每次加加后要进行模运算, 防止其越界. 

![绘图1](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202408141051212.png)

环形队列判空只需要看看`rear`和`front`是否相同即可.    `rear`指向的是下一个数据写入的位置, 当`rear->next`为`front`时, 就意味着队列满了.当然, 对于数组来说, 是`(rear + 1) % size`为`front`,   或者也可以引入一个计数器, 标记已经用了或者还剩多少空间,   既然它是计数器, 那我们就可以交给信号量来做, 而在内部不再判断状态., 有计数器就可以不多开辟一个空间了, 当两指针相遇, 信号量为零就是满了, 没满就是空的.

我们最开始仍然以单生产者单消费者为基本模型.

首先, 该容器的组织形式是可以支持一定程度的边生产, 边消费. 我们可以把环形队列具象意象, 我们假设, 环形队列就是一张圆桌, 圆桌上有一圈盘子,   `a`是生产者, 可以往盘子里放苹果, `b`是消费者, 可以从盘里拿苹果, 只要我们满足如下要求, 模型就可以并行处理

- 对于同一个盘子, 二者不能同时访问
- 二者都没有超过对方   

这两个要求其实是可以归类为一个, 当队列满或者空的时候, 不能再继续并行访问, 或者换句话说, 不空不满的时候, 生产者和消费者可以并行访问.

当队列为空时, 需要生产者赶快生产,   当队列为满时, 需要消费者赶快消费.

--------------------

生产者关注的是剩余空间个数, 我们可以用`SpaceSum`信号量表示;  消费者关注的是剩余数据的个数, 可以用`DataSum`信号量表示, 在最开始, `SpaceSum`为队列容量`N`, `DataSum`为`0`, 消费者申请不到信号量, 阻塞, 生产者申请到信号量, 信号量`SpaceSum`减一,   生产者生产一份数据, 意味着队列中出现新的数据, 需要对`DataSum`进行加加操作, 假设, 消费者一直阻塞, 模型仍能继续运行, 因为当生产者追上消费者时, 其`SpaceSum`变为`0`, 申请不到, 会被阻塞.  反之, 这对于消费者也是适用的. 

所以只要让生产者和消费者减减自己的信号量, 加加对方的信号量, 整个模型就可以正常运行.

好的, 现在就让我们来正式写写代码

我们首先先不写循环队列,   我们先解决之前的一个问题,   之前我们的`info`没有放在临界区里, 而标准输出又是同一个屏幕, 所以打印的时候会比较混乱, 另外, 对于对临界资源的使用, 我们按理应该放在一块,  所以我打算建立一个数据守卫层, 这个数据守卫层将作为循环队列的基本元素, 而我们之后的数据, 无论是进还是出都必须经过这个数据守卫层,  在数据守卫层打印`info`.

```cpp
#pragma once

#include<functional>
#include <utility>


template<class T>
class DataGuard
{
    public:
    typedef std::function<void(const T& val)> function;

    template<class push_info, class pop_info>
    DataGuard(push_info in_log, pop_info out_log)
    :_data(T())
    ,_in_log(in_log)
    ,_out_log(out_log)
    {}

    template<class... Args>
    void push(Args&&... args)
    {
        T temp(std::forward<Args>(args)...);
        _in_log(temp);
        _data = std::move(temp);
    }

    T pop()
    {
        _out_log(_data);
        return _data;
    }

    private:
    T _data;
    function _in_log;
    function _out_log;
};
```

这就是专门再建立一个层方便我们打日志, 没有其它的意思.

接下来我们正式实现循环队列.   我们先来想想该有哪些成员,   首先我们需要一个描述最大容积的`_capa`,  形象的说, 就是描述循环队列这张桌子上有多少个盘子,  接着是`_queue`, 我们用`vector`模拟`_queue`, 因为我们使用`vector`模拟`queue`, 所以要有下标, 它们分别是`_push_idx    and      _pop_idx`,    最后是两个信号量, 分别描述, 有多少可使用的空间`_space_sum`, 和已经有数据的位置`_data_sum`,   

```cpp
#pragma once

#include"DataGuard.hpp"
#include<pthread.h>
#include <semaphore.h>
#include <utility>
#include<vector>

template<class T>
class RingQueue
{
    private:
    typedef DataGuard<T> Data_guard;
    typedef typename Data_guard::function function;
    typedef std::vector<Data_guard> vector;
    typedef sem_t sem;

    public:
    RingQueue(size_t capa, function in_log = [](const T& val){}, function out_log = [](const T& val){})
        

    ~RingQueue()
    

    template<class... Args>
    void push(Args&&... args)
    

    T pop()
    

    private:
    inline void P(sem* sum)
    {
        sem_wait(sum);
    }

    inline void V(sem* sum)
    {
        sem_post(sum);
    }

    private:
    size_t _capa;
    vector _queue;
    int _push_idx;
    int _pop_idx;
    sem _space_sum;
    sem _data_sum;
};
```

接下来写构造函数,  构造函数将由三个三个参数, 第一个是用户必须要输入的,  是循环队列的最大容积,  剩下两个参数是可调用对象, 用来打印`info`,  将会被吃传到数据守卫层,  很明显, 最开始剩余空间数量是`_capa`,  剩余数据个数是`0`, 我们将用`sem_init`对它们进行初始化, 

```cpp
RingQueue(size_t capa, function in_log = [](const T& val){}, function out_log = [](const T& val){})
        :_capa(capa)
        ,_queue(_capa, Data_guard(in_log, out_log))
        ,_push_idx(0)
        ,_pop_idx(0)
        {
            sem_init(&_space_sum, 0, _capa);
            sem_init(&_data_sum, 0, 0);
        }
```

析构函数就不必多说了, 就是把信号量释放

```cpp
~RingQueue()
{
    sem_destroy(&_space_sum);
    sem_destroy(&_data_sum);
}
```

接下来是`push`, 能`push`的前提是有剩余空间, 所以首先要从`_space_sum`那里申请, 把数据转发到守卫层后, 那就意味着多一个数据, 所以应该再让`_data_sum`加一, 然后下标更新, 下标更新目前不需要被信号量保护, 因为现在是单生产者, 单消费者, 生产者更新它的`push_idx`, 消费者更新它的`pop_idx`, 二这互不打扰, 等到多生产者多消费者我们在加锁(二元信号量).

```cpp
template<class... Args>
    void push(Args&&... args)
{
    P(&_space_sum);
    _queue[_push_idx].push(std::forward<Args>(args)...);
    V(&_data_sum);
    ++_push_idx;
    _push_idx %= _capa;
}
```

`pop`的形式和`push`相同

```cpp
T pop()
{
    P(&_data_sum);
    T temp = _queue[_pop_idx].pop();
    V(&_space_sum);
    ++_pop_idx;
    _pop_idx %= _capa;
    return std::move(temp);
}
```

我们把背后工作做好了, `main.cc`就会非常简洁

```cpp
#include<iostream>
#include<ctime>
#include<unistd.h>
#include"RingQueue.hpp"

using namespace std;

typedef RingQueue<int> queue;

void* producer(void* p)
{
    queue* pq = static_cast<queue*>(p);
    while(true)
    {
        int temp = rand() % 10;
        pq->push(temp);
        sleep(1);
    }

    return nullptr;
}

void* consumer(void* p)
{
    queue* pq = static_cast<queue*>(p);
    while(true)
    {
        int temp = pq->pop();
    }
    return nullptr;
}


int main()
{
    queue pq(5, [](const int& val){cout << "[producer]info# push :"<<val<<endl;}, [](const int& val){cout<<"[consumer]info# pop  :"<<val<<endl;});
    pthread_t p, c;
    pthread_create(&p, nullptr, producer, &pq);
    pthread_create(&c, nullptr, consumer, &pq);
    pthread_join(p, nullptr);
    pthread_join(c, nullptr);
    return 0;
}
```

好的, 让我们看看运行效果

```shell
[wind@starry-sky CP_circular_queue]$ ./w.out
[producer]info# push :3
[consumer]info# pop  :3
[producer]info# push :6
[consumer]info# pop  :6
[producer]info# push :7
[consumer]info# pop  :7
[producer]info# push :5
[consumer]info# pop  :5
[producer]info# push :3
[consumer]info# pop  :3
[producer]info# push :5
[consumer]info# pop  :5
[producer]info# push :6
[consumer]info# pop  :6
[producer]info# push :2
[consumer]info# pop  :2
[producer]info# push :9
[consumer]info# pop  :9
[producer]info# push :1
[consumer]info# pop  :1
[producer]info# push :2
[consumer]info# pop  :2
[producer]info# push :7
[consumer]info# pop  :7
[producer]info# push :0
[consumer]info# pop  :0
[producer]info# push :9
[consumer]info# pop  :9
[producer]info# push :3
[consumer]info# pop  :3
^C
[wind@starry-sky CP_circular_queue]$
```

因为我们的`producer  push`一次就`sleep`, 所以很慢, 因此都是`consumer`在等`producer`.

把`sleep`去掉, 就是五五一组

```shell
[producer]info# push :9
[producer]info# push :0
[producer]info# push :4
[producer]info# push :0
[producer]info# push :2
[consumer]info# pop  :9
[consumer]info# pop  :0
[consumer]info# pop  :4
[consumer]info# pop  :0
[consumer]info# pop  :2
[producer]info# push :9
[producer]info# push :4
[producer]info# push :4
[producer]info# push :2
[producer]info# push :9
[consumer]info# pop  :9
[consumer]info# pop  :4
[consumer]info# pop  :4
[consumer]info# pop  :2
[consumer]info# pop  :9
[producer]info# push :0
[producer]info# push :9
^C[producer]info# push :0
[producer]info# push :3
[producer]info# push :5

[wind@starry-sky CP_circular_queue]$ 
```

下面我们来写多生产多消费, 此时就需要加锁了, 就我们目前的模型来说, 因为它只有`push_idx`和`pop_idx`只有两个, 所以一定不能同时让多个生产者或者消费者进来,  进来就会竞争下标(下标此时也成为临界资源了), 可能就会出现诸如数据覆盖, 下标更新异常等问题, 所以一定是要加锁了, 现在的问题, 该往哪里加锁.

以`push`为例, 有两种方案, 第一种是锁在外, 信号量在里, 第二种是信号量在外, 锁在里.

```cpp
 RingQueue(size_t capa, function in_log = [](const T& val){}, function out_log = [](const T& val){})
        :_capa(capa)
        ,_queue(_capa, Data_guard(in_log, out_log))
        ,_push_idx(0)
        ,_pop_idx(0)
        {
            sem_init(&_space_sum, 0, _capa);
            sem_init(&_data_sum, 0, 0);
            pthread_mutex_init(&_push_mutex, nullptr);
            pthread_mutex_init(&_pop_mutex, nullptr);
        }

~RingQueue()
{
    sem_destroy(&_space_sum);
    sem_destroy(&_data_sum);
    pthread_mutex_destroy(&_push_mutex);
    pthread_mutex_destroy(&_pop_mutex);
}

inline void lock(mutex* m)
{
    pthread_mutex_lock(m);
}

inline void unlock(mutex* m)
{
    pthread_mutex_unlock(m);
}

```

```cpp
// 第一种
template<class... Args>
    void push(Args&&... args)
    {
        lock(&_push_mutex);
        P(&_space_sum);
        _queue[_push_idx].push(std::forward<Args>(args)...);   
        ++_push_idx;
        _push_idx %= _capa;
    	V(&_data_sum);
        unlock(&_push_mutex);
    }

// 第二种
template<class... Args>
    void push(Args&&... args)
    {
        P(&_space_sum);
        lock(&_push_mutex);
        _queue[_push_idx].push(std::forward<Args>(args)...);
        ++_push_idx;
        _push_idx %= _capa;
        unlock(&_push_mutex);
        V(&_data_sum);
    }
```

我们选择第二种, 第二种实际上是一种分级渐进竞争机制,   首先需要肯定的是, 锁和信号量都是可以保护自己的, 所以不必在意谁保护谁, 它们都可以在外面.    外面一堆线程,   而最外面是信号量守着门的, 信号量有多份,  所以这样就可以降低竞争性,  然后等到一小部分的线程进去之后, 需要获得锁, 锁只有一个, 但参与竞争的线程较少, 也会降低竞争性, 并且这两级是渐进式的,最里面的线程在生产的同时,             而对于第一种方法, 首先最外层就会有较激烈的竞争, 每次只能放一个线程进来, 放进来之后, 它也不需要竞争, 因为信号量有多份, 直接就进去了,  所以第二种竞争状况不均衡,  就会拖累效率.               或者换种角度来看, 拿到信号量就相当于预约了生成这个动作, 只不过需要等一等.    就像是买了电影票之后,   放映厅里就会给你留个位子, 时候快到的时候在门口排个队, 时候到了就直接进,    第一种方法则是先排队, 排好队后再买票.    

把循环队列改完之后, 再把`main.cc`改一下

```cpp
#include<iostream>
#include<ctime>
#include<cstdio>
#include<unordered_map>
#include<unistd.h>
#include"RingQueue.hpp"

using namespace std;

typedef RingQueue<int> queue;
typedef std::unordered_map<pthread_t, int> unordered_map_;

void* producer(void* p)
{
    pthread_detach(pthread_self());
    queue* pq = static_cast<queue*>(p);
    while(true)
    {
        int temp = rand() % 10;
        pq->push(temp);
        sleep(1);
    }

    return nullptr;
}

void* consumer(void* p)
{
    pthread_detach(pthread_self());
    queue* pq = static_cast<queue*>(p);
    while(true)
    {
        int temp = pq->pop();
        sleep(1);
    }
    return nullptr;
}

unordered_map_ producers;
unordered_map_ consumers;

int main()
{
    queue pq(5, [](const int& val){printf("\033[31m[producer-%d]info$ push: %d\033[0m\n", producers[pthread_self()], val);}, [](const int& val){printf("\033[32m[consumer-%d]info$ pop : %d\033[0m\n", consumers[pthread_self()], val);});
    for(int i = 1; i <= 7; ++i)
    {
        pthread_t c;
        pthread_create(&c, nullptr, consumer, &pq);
        consumers.emplace(c, i);
    }
    for(int i = 1; i <= 9; ++i)
    {
        pthread_t p;
        pthread_create(&p ,nullptr, producer, &pq);
        producers.emplace(p, i);
    }

    while(true);
    return 0;
}
```

我们看看效果

![image-20250307121542510](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250307121542685.png)

不是特别有序, 这是因为`producer`是一个一个创建的, 创建之后调度顺序也不同, 但这种程度的乱没有实质影响, 它还是可以正常的进行写读的.

去掉`sleep`是这种样子.

![image-20250307123031728](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250307123031840.png)

我们看到同一个序号总是一块一块地跑, 这可能是因为其它线程从阻塞到唤醒还是需要一定时间的, 所以在这个过程中, 就是同一个序号中线程在运行, 但也问题不大, 那些进入二级竞争(已经获得信号量但未获得锁)的线程被唤醒后会接替工作, 不会让一个线程一直运行.

接下来我们把`int`换成`task`

我们把`task`改一下

```cpp
#pragma once

#include <utility>
#include <cstdio>
#include<functional>
#include<unordered_map>

// 前置声明
extern std::unordered_map<pthread_t, int> producers;

// C++11风格
class task
{
    private:
    typedef std::function<int(int, int)> func;
    typedef std::unordered_map<char, func> hash;
    hash counter
    {
        {{'+'},{[](int left, int right){return left + right;}}},
        {{'-'},{[](int left, int right){return left - right;}}},
        {{'*'},{[](int left, int right){return left * right;}}},
        {{'/'},{[](int left, int right){
            if(right == 0)
                throw "Zero cannot be a divisor";
            return left / right;
        }}},
    };

    typedef std::unordered_map<int, char> info;
    info op_
    {
        {{0}, {'+'}},
        {{1}, {'-'}},
        {{2}, {'*'}},
        {{3}, {'/'}},
    };

    public:
    task()
        :_left(0), _oper(op_[0]), _right(0)
        {}

    task(int left, int op, int right)
        :_left(left), _oper(op_[op%4]), _right(right)
        {}

    void push_info()const
    {
        printf("\033[31m[producer-%d]info$ %d %c %d = ?\033[0m\n", producers[pthread_self()], _left, _oper, _right);
    }

    int operator()()
    {
        return counter[_oper](_left, _right);
    }

    private:
    int  _left;
    char _oper;
    int  _right;
};
```

这是语言方面的事, 在这里我就不解释了.

`main.cc`也做了一定程度的修改

```cpp
#include<iostream>
#include<ctime>
#include<cstdio>
#include<unordered_map>
#include<unistd.h>
#include"RingQueue.hpp"
#include"task.hpp"

using namespace std;

typedef task data_type;
typedef RingQueue<data_type> queue;
typedef std::unordered_map<pthread_t, int> unordered_map_;


unordered_map_ producers;
unordered_map_ consumers;

void* producer(void* p)
{
    pthread_detach(pthread_self());
    queue* pq = static_cast<queue*>(p);
    while(true)
    {
        int left = rand() % 10;
        int right = rand() % 10;
        int op = rand() % 4;
        pq->push(left, op, right);
    }

    return nullptr;
}

void* consumer(void* p)
{
    pthread_detach(pthread_self());
    queue* pq = static_cast<queue*>(p);
    while(true)
    {
        task t = pq->pop();
        try
        {
            int result = t();
            printf("\033[32m[consumer-%d]info$ %d\033[0m\n", consumers[pthread_self()], result);
        }
        catch(const char* temp)
        {
            printf("\033[32m[consumer-%d]info$ %s\033[0m\n", consumers[pthread_self()], temp);
        }
    }
    return nullptr;
}


int main()
{
    queue pq(5, [](const task& t){t.push_info();});
    for(int i = 1; i <= 9; ++i)
    {
        pthread_t p;
        pthread_create(&p ,nullptr, producer, &pq);
        producers.emplace(p, i);
    }
    for(int i = 1; i <= 7; ++i)
    {
        pthread_t c;
        pthread_create(&c, nullptr, consumer, &pq);
        consumers.emplace(c, i);
    }


    while(true);
    return 0;
}
```

运行结果如下, 本来我担心打印效果可能不太好, 但其实还行

![image-20250307133014733](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250307133014876.png)

## 线程池

我们的线程学习已经接近尾声了, 接下来我们来随手写个线程池.    线程池其实就是对上面的代码进行重新组合而已.   主要是将几个坑.

之前我们似乎提过池化技术, 也不知道是在语言层还是系统层了, 我们知道, 包括线程在内的计算机资源, 在被申请时, 总是要有一些开销的, 池化思想的核心就是我们不应该急着用某些东西的时候才去创建它, 而是应该先一次性创建多个, 等到用的时候马上就用, 销毁的时候也一块销毁. 把资源进行统一的管理, 不要零散地分步.   是一种以空间换时间的操作.

我们将要写的线程池使用的是原生线程接口. 线程池在外部将处于一个黑箱状态, 外部执行流只要往里面放入任务, 便不需要再管, 线程池会自动完成任务.  线程池在内部有一个任务队列和若干线程, 线程在队列为空时便阻塞, 不为空则从中取出任务执行.   这其实就是一种单生产者多消费者的生产消费模型.

我们先说坑, 再看代码,  首先作为线程池肯定是要创建线程的, 但需要注意, `pthread_create`里面的线程行为函数不能使用普通的成员函数,  为什么呢?   因为普通的成员函数是有一个隐式的参数的, 那就是`this`指针, 所以我们必须使用静态成员函数作为线程的行为函数,  但是使用静态成员函数必然会带来一个问题, 那就是没有`this`指针就无法访问普通成员变量, 无法访问就不能从不能调用成员条件变量或者成员锁, 依据从队列中取出任务.   所以`pthread_creat`还要把`this`作为线程行为函数的参数传进去, 这就是我们写线程池最想强调的目的, 让你知道有这样一个坑.

下面就是正常代码了, 我们这里把打印全部交给消费者来做.

```cpp
// thread_pool.hpp
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
            t.push_info();
            try
            {
                int result = t();
                printf("\033[32m[consumer]info$ %d\033[0m\n", result);
            }
            catch (const char *temp)
            {
                printf("\033[32m[consumer]info$ %s\033[0m\n", temp);
            }
        }
    }

private:
    std::vector<thread> _threads;
    std::queue<task_> _tasks;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;
};


// main.cc
#include"main.h"

using namespace std;

int main()
{
    srand((unsigned int)time(nullptr));
    thread_pool<task> threads;
    threads.start();
    while(true)
    {
        sleep(1);
        int left = rand() % 10;
        int right = rand() % 10;
        int op = rand() % 4;
        threads.push(left, op, right);
    }
    return 0;
}
```

![image-20250307162742158](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/20250307162742405.png)

我们的线程池还有一些东西没写, 比如我们没有`join, detach`之类的东西, 就不写了.

## 封装线程

这一节其实没什么用, 我们再怎么封装肯定没有人家标准库写的好, 所以这里主要就是体会一下感觉.

 接下来我们简单地封装一下线程的接口.   

```cpp
#pragma once

#include<pthread.h>

class thread
{
    typedef void(*call_back_t)();

    static void* behavior(void* p_)
    {
        thread* td = static_cast<thread*>(p_);
        td->_cb();
        return nullptr;
    }

    public:
    thread(call_back_t cb)
        :_tid(pthread_t()), _is_running(false), _cb(cb){}

    void run()
    {
        pthread_create(&_tid, nullptr, behavior, this);
        _is_running = true;
    }

    void join()
    {
        pthread_join(_tid, nullptr);
        _is_running = false;
    }

    bool isrun()
    {
        return _is_running;
    }


    private:
    pthread_t _tid;
    bool _is_running;
    call_back_t _cb;
};
```

```cpp
#include "thread.hpp"
#include <iostream>
#include <unistd.h>

using namespace std;

void f()
{
    while (true)
    {
        cout << "shabc" << endl;
        sleep(1);
    }
}

int main()
{
    thread t(f);
    t.run();
    while (true);
    return 0;
}
```

```shell
[wind@starry-sky encapsulating]$ ./w.out
shabc
shabc
shabc
shabc
shabc
^C
[wind@starry-sky encapsulating]$
```

这里主要是语言的事, 而不是系统的,    如果想传参, 那就写成模版的样子, 模版类型是回调参数类型, 然后直接用即可.

## 线程中的 STL 和 smart_ptr

STL的设计初衷是挖掘性能, 而一旦涉及到加锁保证线程安全, 会对性能造成巨大的影响 , 所以STL并不线程安全, 因此如果在多线程环境下使用它们, 可能就需要线程安全操作. 

对于智能指针来说, 它们可以认为是线程安全的, 对于`unique_ptr`, 它不支持复制拷贝之类的操作, 只在代码块范围生效, 所以无法再多线程环境下使用.  对于`shared_ptr`, 理论上引用机制机制会引发问题, 但标准库在实现`shared_ptr`的时候考虑到了这点, 它的引用计数操作是原子性的, 所以不必担心线程安全.

## 单例模式下的线程安全

之前我们已经在语言层说过单例模式了, 只不过由于那是还没有学习线程, 所以并没有说单例模式的线程安全问题, 今天我们就在系统层, 把线程安全考虑一下.  我们就不考虑饿汉模式了, 直接用懒汉模式.

我们就在之前的线程池上改.   首先把构造函数私有化, 拷贝构造, 赋值重载什么的, 都禁用掉, 然后在私有域中定义一个静态的指针成员, 写个垃圾回收器什么, 线程池的`push pop`本身已经有线程安全操作, 所以我们只需要在`getInstance`上加锁就行.   可能我们有这种需求, 主线程创建多个副线程, 副线程又使用线程库, 这样就可能会导致静态指针的数据不一致的现象.

```cpp
static thread_pool<task_>* getInstance(int num = 5)
{
    pthread_mutex_lock(&mutex_);
    if(_tp == nullptr)
    {
        _tp = new thread_pool<task_>(num);
    }
    pthread_mutex_unlock(&mutex_);
    return _tp;
}

```

但这里是有问题的, 这个条件语句只在第一次调用时才是有效的, 其它时候都不需再次执行, 但却被加上了锁. 这就会造成一定的时间消耗, 为此, 我们可以在外部再加一个条件判断

```cpp
static thread_pool<task_> *getInstance(int num = 5)
{
    // 为了效率
    if (_tp == nullptr)
    {
        // 为了线程安全
        pthread_mutex_lock(&mutex_);
        // 为了延迟加载
        if (_tp == nullptr)
        {
            _tp = new thread_pool<task_>(num);
        }
        pthread_mutex_unlock(&mutex_);
    }
    return _tp;
}
```

就这样了, 由于代码的改动部分较少, 所以这里就不回显了.

## 其它的锁

互斥锁对于不同的场景有各种各样的叫法, 但本质上它们都是一样的.不需要太过担心.

-  悲观锁：在每次取数据时，总是担心数据会被其他线程修改，所以会在取数据前先加锁（读锁，写锁，行锁等），当其他线程想要访问数据时，被阻塞挂起。  (上面我们用的都是悲观锁)
- 乐观锁：每次取数据时候，总是乐观的认为数据不会被其他线程修改，因此不上锁。但是在更新数据前，会判断其他数据在更新前有没有对数据进行修改。主要采用两种方式：版本号机制和CAS操作。  (数据库层会详细说的)
- 自旋锁，公平锁，非公平锁 ....

这里只是略微说说自旋锁, 自旋锁其实就是一种轮询锁, 临界区代码有长有短, 一般的锁逻辑是,  先竞争锁资源, 竞争不到就挂起,  所以如果线程要重新竞争锁, 就必须先被唤醒, 当临界区代码很短时, 唤醒的时间消耗就成为了一种负担, 此时我们就可以使用自旋锁, 自旋锁竞争不到不挂起, 而是在这里一直循环, 一直试图竞争锁, 这样就省下了线程唤醒的时间.

为了更方便理解, 我们可以举个现实的例子.   你现在想要使用某个公共资源,  可是它已经有人用了, 如果前面的那位还要再用几个小时, 那你不需要再等了, 完全可以去干些别的事, 这就类似于, 临界区代码很长, 很耗时, 如果一个线程没有竞争到锁, 那它大可以直接去阻塞, 相当于你去干别的事, 但如果前面那位很快就好, 那你就可以在这里等, 对应于临界区代码很短, 线程竞争不到锁, 不挂起, 就在这里循环式地尝试请求锁, 

 什么叫短的临界区呢? 比如只加加减减, 做几次运算的那种就算短的, 这里没有明确的标准, 依靠自己的判断.

自旋锁有两种实现方式:

一是把`pthread_mutex_trylock`套在循环里,  `pthread_mutex_trylock`竞争不到锁会直接返回, 所以套个`while`就有自旋锁的功能了.不过也有专门的自旋锁接口

```cpp
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_trylock(pthread_spinlock_t *lock);
```

`pthread_spin_lock`里面就是一个循环函数, 如果申请不到锁, 它就会在里面一直循环, 直到申请到锁才会跳出来.   `pthread_spin_trylock`是申请不到就会返回, 功能上和`pthread_mutex_trylock`一样的.

## 读写锁

接下来我们说说读写锁, 读写锁是为了解决某些场景下的效率问题而发展出来的:   在编写多线程的时候，有一种情况是十分常见的。那就是，有些公共数据修改的机会比较少。相比较改写，它们读的机会反而高的多。通常而言，在读的过程中，往往伴随着查找的操作，中间耗时很长。给这种代码段加锁，会极大地降低我们程序的效率。  称之为读写者问题.

我们举个例子, 就是我现在写的这篇文章. 我是这篇文章的写者, 在我写的时候, 不允许别人再打开这个文件, 对其进行修改, 当我在写时, 说明这篇文章我没写好, 因为少写一些内容, 就不能允许别人来读, 防止引发歧义.   而在我不写的时候, 我把这篇文章发到网上, 那大家就都可以读, 而且可以同时读.

在读写者问题中, 会有三种对象, 写者   临界资源    读者,     写者与写者之间具有互斥关系, 一个线程在改写内存中的某个数据时, 不允许别的线程再来写, 写者与读者之间也具有互斥关系, 一个线程在改写数据, 说明现在的数据是残缺的, 混乱的,  如果又来一个线程来读, 必然会引发问题,  一个线程在读数据, 说明它还没读完, 写的线程不能去改,   读者与读者之间则是并发的, 只要线程`a`能读, 线程`b`自然也能读 .   读之所以可以一起读, 是因为单纯的读不会引发临界资源的状态变化, 从而引发数据不一致问题.

看起来在形式上和消费生产者模型有些类似,  那它们之间有什么区别吗? 当然是有的, 对于生产者和消费者来说, 它们其实都扮演者写者的角色, 生产者把数据放到临界容器中, 必然会对临界容器进行修改, 而消费者是取出数据, 而不是单纯的读数据, 所以依旧会造成对临界容器的修改, 所以它其实上也是写者. 

读写锁的使用方式和一般的互斥锁相同:

```cpp
// 读写锁的创建与销毁
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock,
                        const pthread_rwlockattr_t *restrict attr);
```

```cpp
// 读者加锁
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
```

```cpp
// 写者加锁
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
```

```cpp
// 释放锁
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
```

一般情况下, 对于读写者模型, 一般是读者多, 写者少, 这就很容易导致写者的饥饿问题: 读者由于数量上的优势, 会更容易竞争到锁, 写者就不容易竞争到锁.   这也是读写者模型的默认情况, 被称为读者优先, 还有一种策略, 叫做写者优先.

读者优先的意思是, 当临界资源正在被读时, 同时来了一个读者, 一个写者, 那读者就要比写者先访问临界资源

写者优先与之相反, 要先让写者进入.

我们这里只说读者优先, 建议看一下伪代码, 看一下是怎么做到读者先进去的, 从而加深对读者优先的理解.

```cpp
// 设置全局变量, 用来统计读者的个数
int readers_num = 0;

// 读者
lock(&rlock);
// 如果是第一个读者
// 对资源加锁, 从而
// 阻止写者访问资源
if(readers_num == 0)
    lock(&wlock);
++readers_num;
unlock(&rlock);

// 读者并发区
// 数据读取操作

lock(&rlock);
--readers_num;
// 最后一个读者
// 解锁临界资源
if(readers_num == 0)
    unlock(&wlock);
unlock(&rlock);

// 写者
lock(&wlock);

// 写者互斥区
// 写操作

unlock(&wlock);
```

只要有读者在读, 写者锁就不会被释放, 写者就进不来.

# end

