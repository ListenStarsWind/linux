# raw_file_system

## preface

什么是原始文件系统？我们之前说的文件系统针对已经打开的文件来说的，而今天，我们将从硬件入手，说一说未打开的文件系统。

## physical hard disk

首先要说的是，我们的主要方向是后端，对于那些服务器来说，所使用的主流存储介质还是硬盘，尽管现在笔记本电脑上主要使用固态硬盘，但服务器和个人电脑很明显有着不同的需求，个人电脑要求便携性和高性能，在这方面，固态硬盘确实优于硬盘，但是，对于服务器来说，我们可能要存储大量的数据，而硬盘比固态在价格上确实是有优势的，所以服务器主要还是使用硬盘的，最多面向用户，把用户经常使用的数据存储在固态中，很少使用的存储在硬盘中。总的来说，服务器就是要用硬盘的，所以我们有必要详细了解一下它最基本的物理结构和实现原理。

对于一个没有被打开的文件来说，它当然是存储在硬盘当中。

考虑到大家手上的笔记本电脑主要用的都是固态，所以没怎么见过硬盘，所以我们先来看看硬盘的外貌。

 ![797c78c66a774b668c034174f83b5de3](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201415089.jpeg)

![hdd1](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201416269.jpg)

![641](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201442601.png)

硬盘，有时被称为“永久存储介质”，这里的永久是相对的，它的意思就是说，这种存储介质断电不会丢失数据。硬盘在主体结构上是机械的，原本外设的存储效率就低，但再怎么低，其内部的主体结构还是各种电路，信息都是通过电信号传递的，但硬盘不一样，即使硬盘的机械运动再快，也比不过光电信号，所以硬盘经常出现效率问题，但不要担心，我们稍后在抽象硬盘章节中会说如何从软件层面提高硬盘效率的。现在我们先不管这些。

在第二张照片里，我们可以看到硬盘里面有若干类似光盘的结构。这就是硬盘中存储数据的核心载体，我们称之为“盘片”。“盘片”和光盘有些不同，光盘只有一张光面，盘片的两面都是光面。数据就是存储在这些光面上的。

![4722a0b864df44f7a8d84d008852507d](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201446684.jpg)

硬盘是磁性存储介质，所以“盘片”也称为“磁盘”。“磁头”负责对磁盘上的数据进行读写，当硬盘断电后，摇头臂的马达就会失效，弹性组件就会把磁头移动到磁头存放区，也就是图中的橙色部分。当硬盘上电后，主轴上的马达就会带动磁盘进行高速转动，这会使得磁盘周围的空气快速流动，从而使得磁头悬浮于磁盘之上，不与磁盘物理接触。磁头会不断左右摆动，以便于对数据进行定位和访问。除此之外我们还可以看到，磁盘是有多个的，每个磁盘的每一面都有一个对应的磁头。磁头和磁盘的距离非常近，这是为了更敏感地识别磁盘上的磁性信息。不过如此近的距离会使得硬盘对空气的要求很高，所以硬盘需要在无尘环境下进行组装，而硬盘在普通环境下拆开，这个硬盘就不能再使用了。使用的话，空气中的小灰尘就会磨花磁盘，于是数据就会在物理层面上被破坏了。所以真拆开了，要去硬盘厂商那里把数据转移出来。

那数据究竟存在磁盘的哪里呢？磁盘的两面看似光滑，实际上每个光面上都有许多同心圆，我们称之为“磁道”，磁道又可以划分成许多“扇区”，扇区就是数据存储的基本元单位。也就是说，硬盘只能一个一个地读取扇区。就像内存一个一个读字节那样。

![2-1Q01215492WL](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201527534.jpg)

磁道是一圈一圈的，扇区是一个一个的。扇区是磁道的基本组成结构。扇区对数据的存储容量是一定的，一般来说，是512字节。从圆心那里画几条辐射线，两条辐射线之间的磁道部分就可以视为一个扇区。不过这实际不够严谨，磁道的半径不同，在图中看，扇区物理大小自然不同，实际上不是这样的，扇区大小都是一样的，半径短的磁道扇区少一些，半径长的磁道扇区多一些。不过为了方便对下面的描述进行简化，我们姑且这样认为。

除此之外还有“柱面”的概念，这个现在了解一下即可，在本篇文章的后面用不到。它是硬盘结构中的一个概念，用于描述数据在硬盘上的物理位置。它是由硬盘中相同半径位置上的所有磁盘盘片上的磁道组成的一个集合。可以简单理解为，当硬盘的磁头组在相同位置（垂直对齐）时，跨越所有盘片上的磁道集合就是一个柱面。磁头的左右移动就是在寻找目标柱面，找到之后，磁头就不会再摆动了，它就选中了这个柱面。这里之所以说现在只需要了解，是因为本篇文章讲的机理，不是实操，实操过程中，肯定要有意识地把数据存在同一个柱面上，这样就可以在不移动磁头的情况下访问该柱面上的所有磁道数据，提高数据访问效率。

![6e8e981f7c78fa57679576abcf2ebecc](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201614099.png)

那这512字节到底是怎么存储的呢？其实很简单，相信我们小时候肯定是见过吸铁石的，吸铁石都有南北两级，扇区里也有很多小吸铁石，我们规定，如果小吸铁石上面是北极，下面是南极，那么它就表示`1`，如果小吸铁石上面是南极，下面是北极，那么它就表示`0`。通过某些机制，很明显是某种物理机制，或者材料机制，具体我不知道，可以改变扇区中小吸铁石的磁性，从而改变它们的`0 1`状态。既然能表示`0 1`，那就好说了，这就是一个比特位，一字节是八比特位，那4096个小吸铁石不就能存储512字节吗。

怎么存数据知道了，接下来要说说怎么粉碎其中的数据。硬盘是有寿命的，对于大型互联网公司来说，为了保证用户数据不丢失，就要在时机到时更换磁盘，他们把旧数据导入到新硬盘后，就要粉碎其中的数据，防止用户数据泄露。

这里的粉碎不是我们平常说的右键删除或者`rm`，不是软件层面的数据删除，等会我们会说的，软件层面的数据没有真正删除。一般也不是IT企业自己覆写硬盘，而是硬盘厂商自己根据自家硬盘的实际情况，设计专门的一键粉碎算法，并将其提供给企业，粉碎几轮数据，检查通过后，就差不多可以重新在市场上流通。

## abstract hard disk

我们毕竟是搞软件的，所以比起上面的硬盘，我们更应该关注这个抽象硬盘，抽象硬盘的意思就是从系统的角度来看硬盘，系统会通过一系列中间层，对物理硬盘进行抽象建模，就像它对其他硬件那样，这种抽象可以让系统完全忽视硬盘的物理结构，从而便于系统对数据的操作。

硬盘在硬件层面使用的是“CHS寻址方式”，此处的`CHS`即是`Cylinder-Head-Sector`，其中`Cylinder`代表柱面，`Head`代表磁头，`Sector`代表扇区，这种寻址方式就是先确定柱面，也就是磁道半径，然后柱面有若干个盘面，每个盘面都有一个对应的磁头，要选中那个磁头进行读写，面和半径都确定了，接下来就要找具体扇区了。这是硬件层面的寻址，操作系统一般不用这种寻址方式。

那操作系统主要用什么寻址方式呢？别急，在说这个之前我们先看看磁带。

![67EwfIGr2bjO9nZGPdx3Tg](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201700202.jpg)

![image-20241120170816115](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201708190.png)

磁带也是一种存储介质，它里面也是一圈一圈的，如果把它里面的带子扯出来放好，就会发现它是一个很长的长条。同样的，各个盘面上的磁道我们也可以抽象的把它展开，这样也能形成一个很长的长条。

![image-20241120172248155](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201722193.png)

这样就能把硬盘抽象成线性的存储结构。

每个面都可以划分出若干磁道

![image-20241120172722466](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201727492.png)

每个磁道又可以划分出若干扇区

![image-20241120172900025](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201729064.png)

这样的话，我们其实就可以把硬盘抽象成以扇区大小为单位元素的数组。既然是数组，那就有下标。

这就是系统寻址硬盘的方式，系统把扇区编号通过硬盘驱动传给硬盘，硬盘自己有对应的接口，会把编号转换成"CHS"格式，再自己锁定这些扇区。

怎么转换呢？在以上面那种姑且眼光来看硬盘的话，很简单，由于磁道的扇区数都是相同的，把编号除一下，余一下，就能出来了。

比如，我随口编一个硬盘参数：六个面，每个面20000个扇区，200个磁道，每个磁道100个扇区，假如现在编号是24567，24567/20000得1，如果盘面编号从0开始，那就是编号为1的盘面，24567%20000得4567,4567/100得45，那就是使用编号为45的磁道，具体位置是该磁道的66位置扇区。

当然，实际上肯定不是这样转的，但这不需要我们担心，硬盘厂商会在硬盘内部配置相应的转换器的，作为系统，只需要给编号就行了，其它怎么转化的，不用我们亲自去管。

这个扇区编号寻址方式被称为LBA（Logical Block Addressing）寻址方式。

-----------

现在回到硬盘物理结构上，硬盘上有一些寄存器，实际上是端口或者串口，不过看做寄存器就行了。比如控制寄存器，负责控制信息流向，是读，还是写？还有位置寄存器，如果是读，读哪些编号扇区，如果是写，又写哪些扇区；有数据寄存器，如果要写数据，那把数据放在这里面，如果要读数据，从这里把数据读走；还有状态寄存器，之前的操作结果如何？成功还是失败？成功了，现在是否进入就绪状态，可以执行其它操作？等等。

实际上，对于现在的计算机来说，CPU已经不直接亲自去执行IO任务了，而是直接给dma芯片发IO任务，由dma芯片对IO任务进行实际执行，从而提高CPU效率。

## ext2_file_systeam

ext2是一个具体的原始文件系统，专门用于Linux系统文件管理，下面我们就来学习它。

假设现在我们有一个800G的硬盘。800GB太大了，所以要进行范围划分，我们称之为分区，分区在软件层面上很简单，就是写个范围结构体，记录一下开头和末尾的扇区编号，然后就行了。比如，不过要注意的是Linux的分区和Windows的分区有些不同，Windows的分区是分成C盘，D盘，E盘这种，而Linux没有盘符这种概念，它用的是挂载点。Linux的文件是树结构而不是森林结构，硬盘每个大的分区就是一个挂载点，挂载点非常灵活，它可以直接挂载到特定目录下，比如`/home`、`/var`下，使用起来的感觉就像普通文件，即使是不同硬盘也能做到无缝衔接，用起来像是一个整体。

比如，依据下面这张图，我们把一个硬盘划分出了`n+2`个区域。或者说`n+2`个挂载点。

![image-20241120194848049](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411201948217.png)

为了方便起见，可以把这些分区统称为“块”。第一个块比较特殊，它的大小固定，其内部存储的是一个程序，该程序的职责是在电脑开机时，从硬盘的其它部位找到操作系统内核程序，然后引导内核加载到内存中，随后它会把指挥权移交给系统内核，自己则从内存中退出。也就是管开机的，它里面的数据若遭到破坏，就会开不开机。不过我们的服务器倒也很少开关机，所以了解即可。

其它的块大小在格式化之后确定，不能更改。格式化之后说。

接下来我们可以对普通的块再进行划分，图中也画出了。接下来我们管好每个块，就能管好整个硬盘，体现了分治的思想。

我们先说`Date blocks`。`Date blocks`是存储文件内容的区域，它的内部也进行了再划分，为了方便叙述，我们同样将`Date block`中的每个小单元称为“块”。这些块的大小同样是在格式化阶段确定，在格式化之后便无法修改。一般来说，每个块的大小是4KB，即4096字节。硬盘确实是一个一个扇区读写的，但对于操作系统来说，它一般都是以`Date blocks`中的块作为读写单位，也就是说，系统是一个块一个块地读写的，每个块会又会带动八个扇区进行读写。当通过文件接口读写文件时，都是以块的大小为单位加载到内存中的，这就是局部原理的一种体现，当用户需要某处数据时，系统会默认用户也需要相邻的其它数据，从而把它们都加载到内存中，所以说链表读写效率，或者说缓冲命中率低。

现在，扇区的概念已经被吸收到`Date blocks`的块概念中，这意味着，当用户需要某处数据时，CPU发出的IO命令都是找到某某分区`Date blocks`的某某块，而不会再说哪个编号的扇区了。同样的，既然`Date blocks`中的块大小一致，那也可以把它们一一编号用来指代。

好的，再继续深入了解`Date blocks`之前，先让我们看看`inode Table`，`inode Table`是一个结构体数组，其中的每个结构体，都对应着一个文件，我们把这些结构体称为`inode`，`inode`的大小同样是在格式化过程中确定，一般是128字节，`inode`中存储着对应文件的种种属性，比如权限，所属组，最近修改时间什么的，每个`inode`同样有着编号，与它在数组中的下标有一定关系。`inode`编号的作用范围是其所在的分区。`inode Table`中的`inode`个数同样是在格式化过程中被固定，此后不能修改，这意味着，如果`inode Table`中找不到空位置了，即使`Date blocks`中仍有可被使用的块，这个分区也无法再创建新文件了，同样的，如果`Date blocks`中所有块都被使用，即使`inode Table`中仍有可以使用的位置，也无法再创建新文件了。

另外，需要说明的是，`inode`中并不包含文件名，或者说，Linux系统不认文件名，它只认文件编号。文件编号是文件的唯一标识符，它与`inode`编号并不是同一概念，`ls`带上`-i`选项可以给出文件对应的编号。

```shell
[wind@starry-sky ~]$ ls -li
total 24
1048999 drwxrwxr-x  5 wind wind 4096 Oct 26 16:55 comparisons
1048585 -rw-r--r--  1 root root  827 Oct 13 18:49 install.sh
1048931 -rw-rw-r--  1 wind wind   60 Nov 14 12:51 logbook.txt
1058790 -rw-rw-r--  1 wind wind   65 Nov 20 10:01 log.txt
1058780 drwxrwxr-x 23 wind wind 4096 Nov 16 18:31 projects
1186273 drwxrwxr-x  3 wind wind 4096 Nov 11 11:55 resources
[wind@starry-sky ~]$ 
```

那问题来了，怎么把文件名与文件编号建立联系呢？别担心，等会我们会说的。

`inode`中还存储着对应文件的块索引，它也是一个数组

```cpp
#define NUM 15

struct inode
{
    文件编号;
    文件类型;
    权限;
    引用计数;
    拥有者;
    所属组;
    ACM时间;
    int blocks[NUM];
};
```

接下来我们看看`blocks`是如何索引`Date`块的，数组的前12个元素是直接索引，也就是说，它们指向的`Date`块中都存储着文件的内容，比如10下标是4578，则4578块中就存储着文件内容，第13,14号元素是间接索引，或者称二级索引，它们指向的`Date`块中并不存储文件内容，而是存着一个类似于`blocks`的数组，不过，这个数组中的所有元素都是直接索引，当然，因为每个块是4096字节的，所以可以存储1024个整型元素，这些整型元素都是直接索引，它们都直接指向存储着文件内容的其它块。现在让我们来算一算，`blocks`的前12个元素直接指向了12个块，第13,14个元素指向两个块，其中的每个块都可以指向1024个块，这样，在第15号元素参与之前，我们已经可以索引12 + 1024 + 1024 等于2060个块，每个块4KB，这样就有了8240KB，或者说差不多8MB。

要是文件比8MB大怎么办？没事，第15号元素还没使用呢，15号元素称为三级索引，它指向的块中存储的是二级索引，这样就会多出1024个二级索引，然后这1024个二级索引中的每一个都能分出1024个直接索引，于是就有了1024 * 1024个直接索引，这样就又可以支配1024\*1024\*4096字节，或者说4GB。

接下来我们谈谈`Block Bitmap`。它是一个位图，`Date blocks`中有多少块，它就有多少位，当位图中的某一位置1后，就意味着系统认为该比特位对应的块中信息是有效的，当位图中的某位为0是，就意味着对应的块中的数据不被系统所承认，尽管这些块中可能还残留着数据，这就是存数据可能要化很长时间，而删除数据却非常快的原因，因为它只是把`Block Bitmap`中的对应位置为0而已，没有破坏块中的数据，当某个位被置为0后，就意味着对应的块现在已经处于就绪状态，可以被覆写，否则就是处于保护状态，块中数据不允许被修改。

`inode Bitmap`于此同理，它也是一个位图，`inode Table`可以容纳多少个`inode`，它就有多少位，当某位置为1时，就意味着对应编号的`inode`是有效的，否则系统就认为这些`inode`中的信息是无效的。我们删除文件的过程就是，找到该文件对应的`inode`，从而找到其对应的块，然后在`Block Bitmap`把涉及到的块对应的比特位置为0，然后再找到`inode Bitmap`把该文件对应`inode`的对应比特位置为0，根本不碰`Date blocks`。这为数据恢复提供了思路，反正文件内容并没有真的删除。

接下来说`Group Descriptor Table`。它负责描述这单个分区的大致情况，例如，这个`block group`还有多少空间可以使用，以及创建新文件时应该为其分配的`inode`编号，当然理论上对两个位图遍历一下就能获得这些使用信息，但效率有些低，所以就有了`group descriptor table`直接记录这些信息。

`Super block`比较特殊，虽说在图中它位于`Block group 0`，但实际上它是用来描述所有`Block group`的，比如，这个硬盘有多少分区，每个分区多大，它们所在的扇区编号范围是多少，每个分区中的各个部分，`Group Descriptor Table, Block Bitmap, inode Bitmap, inode Table, Date blocks`又各自多大，位于其`block group`的具体位置范围，整套文件系统的状态如何······总而言之，它对该硬盘上的整套文件系统进行了描述。

因为`super block`太重要了，没了它，整套文件系统就无法使用了，所以它是有备份的，有的`block group`有`super block`，有的没有，当`super block`要被更新时，所有的`super block`都会被更新，因此不是每个`block group`都有`super block`，那样更新起来太麻烦了。

像`super block`这种具有重要意义的数据，其内部都会有一种名为“魔数”的结构，其位置通常位于重要数据的开头或者结尾，这些魔数相当于一个特定的标识符，可以对数据进行快速检查。比如`boot block`的魔数通常位于其尾部，是一个二字节的数字，其值通常为`0xAA55`，通过该魔数可以快速判断出该分区为引导分区；又如`super block`的魔数可能为`0xEF53`，可以帮助判断`super block`是否有效。

很明显，在正式使用该硬盘存储数据时，其上述的文件组织结构必须已经存在才行，文件的存储是建立在上述文件系统已经存在的情况下才能进行。将一个完全空的硬盘建立文件系统的过程，对其中的各个组分进行定义和初始化的过程被称为格式化，格式化的目的是让硬盘变成已经可以存储文件的最开始状态，当然`Date blocks`中的块不一定真覆写，但两个位图肯定要全部置为0的，所以格式化也能视为对存储介质中文件的清空，比如一个U盘不好用了，里面的数据又不重要，就可以直接格式化，如果格式化都解决不了问题，那估计就是物理问题了。

我们可以使用`stat`查看文件的各种状态

```shell
[wind@starry-sky Debug]$ ls
code.cpp  makefile
[wind@starry-sky Debug]$ stat makefile
  File: ‘makefile’
  Size: 76        	Blocks: 8          IO Block: 4096   regular file
Device: fd01h/64769d	Inode: 1445099     Links: 1
Access: (0664/-rw-rw-r--)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-22 19:07:45.051640147 +0800
Modify: 2024-11-22 19:07:46.646703345 +0800
Change: 2024-11-22 19:07:46.647703385 +0800
 Birth: -
[wind@starry-sky Debug]$
```

`size`表示大小，文本文件`makefile`一共76字节，`IO Block`表示系统块的大小，可以看到使用的就是八个扇区块，每个扇区512字节，每个系统块4096字节的经典方案，`Blocks`表示使用的扇区块大小，76字节小于一个系统块的大小，所以使用一个系统块，即八个扇区块，`Device`是存储介质，也就是硬盘的标识符，`Inode`是`Inode`编号，`Links`表示引用数，其它的就不看了。

现在我们大致说说一个文件的整个生命周期。我们通过某个进程创建一个文件，比如`bash`，这个进程的控制块中包含着其所在路径的详细信息，通过某些机制，具体什么机制等会会说，系统可以找到对应的硬盘分区，然后根据`Group Descriptor Table`获取一个`inode table`中的空位置，对该位置上的`inode`进行定义和初始化，然后在`Date blocks`中写入文件内容，并修改两个`map`即可。删除文件则是先找到其所在的分区，然后修改两个位图的对应位即可。修改文件，就是把它的属性和内容加载到内存中，供进程进行访问和修改。

接下来说说上文中省略的机制——如何把目录信息与上文所说的文件系统建立联系。以及回答一个问题：文件名是如何与`inode`编号建立联系的。

首先让我们回答一下一个问题：目录或者说文件夹，是文件吗？当然是文件了！既然文件分为两部分——内容和属性，这对目录仍旧使用，目录的属性依旧可以使用`stat`进行查看。

```shell
[wind@starry-sky Debug]$ ll -a
total 12
drwxrwxr-x 2 wind wind 4096 Nov 22 19:07 .
drwxrwxr-x 3 wind wind 4096 Nov 22 19:02 ..
lrwxrwxrwx 1 wind wind   19 Nov 22 19:05 code.cpp -> ./../../../main.cpp
-rw-rw-r-- 1 wind wind   76 Nov 22 19:07 makefile
[wind@starry-sky Debug]$ stat ..
  File: ‘..’
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
Device: fd01h/64769d	Inode: 1445093     Links: 3
Access: (0775/drwxrwxr-x)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-22 19:06:22.673376236 +0800
Modify: 2024-11-22 19:02:53.262079147 +0800
Change: 2024-11-22 19:02:53.262079147 +0800
 Birth: -
[wind@starry-sky Debug]$ cd ..
[wind@starry-sky x64]$ stat .
  File: ‘.’
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
Device: fd01h/64769d	Inode: 1445093     Links: 3
Access: (0775/drwxrwxr-x)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-22 19:06:22.673376236 +0800
Modify: 2024-11-22 19:02:53.262079147 +0800
Change: 2024-11-22 19:02:53.262079147 +0800
 Birth: -
[wind@starry-sky x64]$ cd ..
[wind@starry-sky bin]$ stat x64
  File: ‘x64’
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
Device: fd01h/64769d	Inode: 1445093     Links: 3
Access: (0775/drwxrwxr-x)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-22 19:06:22.673376236 +0800
Modify: 2024-11-22 19:02:53.262079147 +0800
Change: 2024-11-22 19:02:53.262079147 +0800
 Birth: -
[wind@starry-sky bin]$ 
```

只需要查看`stat x64`结果即可。

那现在问题就来了，这个`x64`文件夹里存储的是什么？你可能觉得有些奇怪。还能存什么？存`makefile and code.cpp`了，至于`. and ..`可能你不太确定，但前面两个文件肯定是有的。不过很抱歉的是，我要的不是这个答案。我想问的是`x64`文件对应的数据块中存的是什么？我可以告诉你，在`x64`数据块中存储的并不是`makefile and code.cpp`的文件本体，而是一份索引，或者说映射关系，关于`x64`内部文件的文件名和其对应`inode`编号的映射关系。

比如我现在处于`bin`目录下，当我输入`cd x64`的指令时，系统会先找到`bin`文件的数据块，读取里面的映射关系，从而找到`x64`文件的`inode`编号，有了编号，就可以在物理层面上真正找到`x64`了。

这就可以解决一些问题。比如，为什么一个目录下不能有同名文件，因为那样会破坏文件名与`inode`编号的映射关系，`inode`编号都找不到，那就在物理层面上找不到文件，既然文件都找不到，那这文件创建有什么意义呢？为什么对目录没有写权限，就不能在其中创建新文件？因为没有写权限，就无法在改目录的数据块中写入新的文件名和`inode`编号映射关系，`inode`编号都找不到，那这个文件就没有存在意义了，所以系统拒绝这种请求。为什么对目录没有读权限，就看不到该目录下的文件呢？因为读不了改目录数据块中的内容，所以我就找不到映射关系了，甚至连文件名都看不到。至于`x`权限，没有这个，base会直接拦住你，和这里关系不大。

现在我们的逻辑已经完成了一大半了，就像某些数学证明题，已经证明了`k`项这样，那`k+1`项就一定那样，接下来就要证明首项了。

从绝对路径的角度来说，不管你那文件树有多花里胡哨，总要有一个根目录`/`，大不了可以直接从`/`那里开始找，一层一层地往深处找，就能找到指定的路径了。每个目录中的`.`就是这种情况，`.`实际就是`bash`工作路径的另一种指代，而`bash`工作路径则是从根目录到当前目录的绝对路径。不过为了效率，对于用户常使用的路径，系统可能会把其中的信息提前加载到内存中，这样就不用真去物理层了。

从相对路径的角度来说，Linux的所有目录下都有一个代码层面上硬性规定的`..`，它是上一级文件夹的一种引用，可以借助于它找到上一级文件夹的`inode`编号（通过），就像之前那样

```shell
[wind@starry-sky Debug]$ ll -a
total 12
drwxrwxr-x 2 wind wind 4096 Nov 22 19:07 .
drwxrwxr-x 3 wind wind 4096 Nov 22 19:02 ..
lrwxrwxrwx 1 wind wind   19 Nov 22 19:05 code.cpp -> ./../../../main.cpp
-rw-rw-r-- 1 wind wind   76 Nov 22 19:07 makefile
[wind@starry-sky Debug]$ stat ..
  File: ‘..’
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
Device: fd01h/64769d	Inode: 1445093     Links: 3
Access: (0775/drwxrwxr-x)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-22 19:06:22.673376236 +0800
Modify: 2024-11-22 19:02:53.262079147 +0800
Change: 2024-11-22 19:02:53.262079147 +0800
 Birth: -
[wind@starry-sky Debug]$ cd ..
[wind@starry-sky x64]$ stat .
  File: ‘.’
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
Device: fd01h/64769d	Inode: 1445093     Links: 3
Access: (0775/drwxrwxr-x)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-22 19:06:22.673376236 +0800
Modify: 2024-11-22 19:02:53.262079147 +0800
Change: 2024-11-22 19:02:53.262079147 +0800
 Birth: -
[wind@starry-sky x64]$ cd ..
[wind@starry-sky bin]$ stat x64
  File: ‘x64’
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
Device: fd01h/64769d	Inode: 1445093     Links: 3
Access: (0775/drwxrwxr-x)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-22 19:06:22.673376236 +0800
Modify: 2024-11-22 19:02:53.262079147 +0800
Change: 2024-11-22 19:02:53.262079147 +0800
 Birth: -
[wind@starry-sky bin]$ 
```

## link

接下来说说链接，它分为两种，软链接和硬链接，我们将在上面知识的基础上理解它们。

`ln`可以建立链接，默认为硬链接，带上`-s`后为软链接

```shell
[wind@starry-sky Debug]$ ll
total 4
lrwxrwxrwx 1 wind wind 19 Nov 22 19:05 code.cpp -> ./../../../main.cpp
-rw-rw-r-- 1 wind wind 76 Nov 22 19:07 makefile
[wind@starry-sky Debug]$ rm code.cpp
[wind@starry-sky Debug]$ ls
makefile
[wind@starry-sky Debug]$ ln -s ./../../../main.cpp code.cpp
[wind@starry-sky Debug]$ ls
code.cpp  makefile
[wind@starry-sky Debug]$ echo "hello link" > m.txt
[wind@starry-sky Debug]$ ls
code.cpp  makefile  m.txt
[wind@starry-sky Debug]$ ll
total 8
lrwxrwxrwx 1 wind wind 19 Nov 22 21:07 code.cpp -> ./../../../main.cpp
-rw-rw-r-- 1 wind wind 76 Nov 22 19:07 makefile
-rw-rw-r-- 1 wind wind 11 Nov 22 21:08 m.txt
[wind@starry-sky Debug]$ ln m.txt txt
[wind@starry-sky Debug]$ ll
total 12
lrwxrwxrwx 1 wind wind 19 Nov 22 21:07 code.cpp -> ./../../../main.cpp
-rw-rw-r-- 1 wind wind 76 Nov 22 19:07 makefile
-rw-rw-r-- 2 wind wind 11 Nov 22 21:08 m.txt
-rw-rw-r-- 2 wind wind 11 Nov 22 21:08 txt
[wind@starry-sky Debug]$ ll -i
total 12
1445098 lrwxrwxrwx 1 wind wind 19 Nov 22 21:07 code.cpp -> ./../../../main.cpp
1445099 -rw-rw-r-- 1 wind wind 76 Nov 22 19:07 makefile
1445100 -rw-rw-r-- 2 wind wind 11 Nov 22 21:08 m.txt
1445100 -rw-rw-r-- 2 wind wind 11 Nov 22 21:08 txt
[wind@starry-sky Debug]$ ls -i ./../../../main.cpp
1445095 ./../../../main.cpp
[wind@starry-sky Debug]$
```

我们可以看到`code.cpp`和`main.cpp`是两个相互独立的文件，它们各有各的`inode`编号，而`m.txt and txt`的`inode`编号是相同的，这就是软硬链接的区别：软链接是一个独立的文件，具有自己的`inode`编号。硬链接不是独立文件，没有自己的`inode`编号，并且它会增加引用数，即`m.txt`的引用数从`1`变为`2`。

我们先谈硬链接。所谓硬链接，就是在目录，在此场景下就是`Debug`目录文件的数据块中新增一个文件名和`inode`编号的映射关系，于是在硬链接建立之后，该目录下`m.txt and txt`都指向同一个`inode`编号，这里的引用数其实描述的就是有多少个文件名指向这个`inode`编号，或者说，有多少个入口可以找到`inode`编号。

此时`m.txt and txt`具有相同的行为，`rm`之后减少引用数，当引用数为0时，就意味着已经没有入口可以找到该文件了，它就失去了存在意义，于是系统就会删除它。

```shell
[wind@starry-sky Debug]$ ls
code.cpp  makefile  m.txt  txt
[wind@starry-sky Debug]$ ll -i
total 12
1445098 lrwxrwxrwx 1 wind wind 19 Nov 22 21:07 code.cpp -> ./../../../main.cpp
1445099 -rw-rw-r-- 1 wind wind 76 Nov 22 19:07 makefile
1445100 -rw-rw-r-- 2 wind wind 11 Nov 22 21:08 m.txt
1445100 -rw-rw-r-- 2 wind wind 11 Nov 22 21:08 txt
[wind@starry-sky Debug]$ rm m.txt
[wind@starry-sky Debug]$ ll -i
total 8
1445098 lrwxrwxrwx 1 wind wind 19 Nov 22 21:07 code.cpp -> ./../../../main.cpp
1445099 -rw-rw-r-- 1 wind wind 76 Nov 22 19:07 makefile
1445100 -rw-rw-r-- 1 wind wind 11 Nov 22 21:08 txt
[wind@starry-sky Debug]$
```

软链接则是记录了文件的路径，通过这个路径就可以找到文件，因此没有增加找到`inode`编号的入口，引用计数不增加。

软链接与原文件的地位不相同，当原文件被删除后，软链接就会因为找不到文件路径而报错，相当于Windows快捷方式

![image-20241122213554541](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411222135710.png)

`unlink`可以用来删除链接文件

```shell
[wind@starry-sky Debug]$ unlink code.cpp
[wind@starry-sky Debug]$ ls
makefile  txt
[wind@starry-sky Debug]$ 
```

下面说说软硬链接的应用场景。

软链接好说，就是建立一个快捷方式。对于一个项目来说，其文件结构是有要求的，可执行程序应该放在某些特定目录下，但为了让用户可以更便捷地运行程序，可以在他桌面上建一个快捷方式。对于Linux来说，如果想让自己写的程序变成系统指令，直接放自己的程序不太好，会造成命令污染，容易分不清默认路径下的那些程序是自己写的，那些又是系统自己的。这时就可以在默认路径下建立一个软链接，实际上，默认路径下有些程序就是软链接。

```shell
[wind@starry-sky Debug]$ ls
a.out  main.c  makefile  txt
[wind@starry-sky Debug]$ pwd
/home/wind/projects/raw_file_system/bin/x64/Debug
[wind@starry-sky Debug]$ sudo ln -s /home/wind/projects/raw_file_system/bin/x64/Debug/a.out /usr/bin/link.exe
[wind@starry-sky Debug]$ ls /usr/bin/link.exe
/usr/bin/link.exe
[wind@starry-sky Debug]$ link.exe
hello link
[wind@starry-sky Debug]$ ./a.out
hello link
[wind@starry-sky Debug]$ sudo unlink /usr/bin/link.exe
[wind@starry-sky Debug]$
```

注意要使用绝对路径和`root`权限。

现在我们看看硬链接，硬链接其实用的很少，最起码对于用户而言是这样的。

```shell
[wind@starry-sky Debug]$ ls
a.out  main.c  makefile  txt
[wind@starry-sky Debug]$ rm main.c txt a.out
[wind@starry-sky Debug]$ ls
makefile
[wind@starry-sky Debug]$ stat .
  File: ‘.’
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
Device: fd01h/64769d	Inode: 1445094     Links: 2
Access: (0775/drwxrwxr-x)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-23 09:27:45.750142015 +0800
Modify: 2024-11-23 09:27:37.158801628 +0800
Change: 2024-11-23 09:27:37.158801628 +0800
 Birth: -
[wind@starry-sky Debug]$ touch txt
[wind@starry-sky Debug]$ ll -i txt
1445088 -rw-rw-r-- 1 wind wind 0 Nov 23 09:32 txt
[wind@starry-sky Debug]$ mkdir dir
[wind@starry-sky Debug]$ ll -di dir
1445095 drwxrwxr-x 2 wind wind 4096 Nov 23 09:32 dir
[wind@starry-sky Debug]$ stat .
  File: ‘.’
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
Device: fd01h/64769d	Inode: 1445094     Links: 3
Access: (0775/drwxrwxr-x)  Uid: ( 1002/    wind)   Gid: ( 1002/    wind)
Access: 2024-11-23 09:27:45.750142015 +0800
Modify: 2024-11-23 09:32:41.687866999 +0800
Change: 2024-11-23 09:32:41.687866999 +0800
 Birth: -
[wind@starry-sky Debug]$ cd dir
[wind@starry-sky dir]$ ll -ai
total 8
1445095 drwxrwxr-x 2 wind wind 4096 Nov 23 09:32 .
1445094 drwxrwxr-x 3 wind wind 4096 Nov 23 09:32 ..
[wind@starry-sky dir]$
```

创建一个普通文件，其默认计数为1，而创建一个新目录，其默认计数为2，这是因为在这个新目录里面还有一个隐藏目录`.`，这个点就是硬链接，而且由于新目录下还有一个`..`，所以`Debug`的引用计数也由2变为了3。

为什么说用户很少用呢？因为系统不允许用户对目录进行硬链接，这是写在代码层的，`root`也无法对目录进行硬链接。这是为了文件系统成环，因为Linux文件系统是树状结构，它不允许成环，成环容易成问题，比如在某个中间节点对根目录创建一个硬链接，在执行某些指令时，比如`find`，或者其它那些会递归式访问目录的指令，它就停不下来了，因为成环了，所以进入那个中间节点的硬链接后就会重新回到根目录，就始终达不到递归终止条件了。

至于`. and ..`，那是写在代码层的，其设计目的是主要是为了能够回到上级目录或者更便捷地指代所在目录，而在写那些递归访问目录的程序时，开发者都会将`. and ..`视为特例忽略，这样就不会出现无限递归问题，当然，软链接应该也是特例。

## others

操作系统无论干什么事，总离不开物理内存的参与，为此，下面我们将浅谈一下Linux的内存管理，这样才好把原始文件系统和动态文件系统整合到一块。

Linux将物理内存划分成大小相同的块，由于内存主要与硬盘进行数据交流，所以内存块的大小往往与硬盘数据块的大小相同，默认为4KB，合4096字节，我们把这种内存块叫做“页框”，与此同时，在此种场景下，数据块也往往被称为“页帧”，由于页帧和页框大小相同，所以就便于物理内存和硬盘之间的数据交换。

物理内存的块设计有两层含义，一是硬盘上的数据是一块一块地往内存中加载的，二是内存块和数据块一样也会出现存不满的现象。一块一块地加载是为了考虑硬盘的效率问题，因为硬盘的读写效率较低，所以一块一块地加载对硬盘的比较友好，同时，这也是局部性原理的一种实现方式，如果用户对某处数据进行访问，系统就会默认用户大概率也会对周围数据进行访问，所以尽管可能用户现在只改了几个字节，但系统仍然会把4KB都加载进来以此来提高效率。

为了对页框进行管理，系统会为每个页框创建相对应的`struct page`对象，作为对应页框的抽象描述，比如对于4GB来说，其含有1048576个页框，对这些页框进行描述，就能产生1048576个`struct page`对象，再把这些对象统一放在物理内存的某个特定区域，就能形成一个元素个数为1048576，元素类型为`struct page`的数组，这样对物理内存的管理就可以抽象成对这个数组的管理。既然是数组，那就拥有下标，我们把这些下标称为对应页框的页号，至于页号与页框的联系，则可以直接通过页框起始地址除以页框大小的方式进行（不过在代码中实际看不到除法，因为出于硬件效率的考虑，代码更多的是用按位与取代除法，例如`address` & 0xFFFF F000，这怎么理解呢？因为页框起始地址是页框大小的整数倍，而页框大小是4KB，或者说是12个比特位，所以如果一个地址是页框起始地址的话，它的低十二位肯定都是0，那把任意地址&0xFFFF F000就相当于抹除了它的低十二位，所以得到的就是对应页框的起始地址；而如果一个地址的低十二位中含有1，就说明它不是正好与页框的大小对齐，而是在页框内部，所以低十二位就可以被视为页框内部的偏移量。总之，根据物理地址是很容易找到其所在的页框的，另外，在线程章节我们会重新说这个。）

让我们看看Linux原码中的`struct page`

![image-20241124204826537](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411242048715.png)

```c
/*
 * Each physical page in the system has a struct page associated with
 * it to keep track of whatever it is we are using the page for at the
 * moment. Note that we have no way to track which tasks are using
 * a page, though if it is a pagecache page, rmap structures can tell us
 * who is mapping it.
 */
struct page {
	unsigned long flags;		/* Atomic flags, some possibly
					 * updated asynchronously */
	atomic_t _count;		/* Usage count, see below. */
	union {
		atomic_t _mapcount;	/* Count of ptes mapped in mms,
					 * to show when page is mapped
					 * & limit reverse map searches.
					 */
		struct {		/* SLUB */
			u16 inuse;
			u16 objects;
		};
	};
	union {
	    struct {
		unsigned long private;		/* Mapping-private opaque data:
					 	 * usually used for buffer_heads
						 * if PagePrivate set; used for
						 * swp_entry_t if PageSwapCache;
						 * indicates order in the buddy
						 * system if PG_buddy is set.
						 */
		struct address_space *mapping;	/* If low bit clear, points to
						 * inode address_space, or NULL.
						 * If page mapped as anonymous
						 * memory, low bit is set, and
						 * it points to anon_vma object:
						 * see PAGE_MAPPING_ANON below.
						 */
	    };
#if USE_SPLIT_PTLOCKS
	    spinlock_t ptl;
#endif
	    struct kmem_cache *slab;	/* SLUB: Pointer to slab */
	    struct page *first_page;	/* Compound tail pages */
	};
	union {
		pgoff_t index;		/* Our offset within mapping. */
		void *freelist;		/* SLUB: freelist req. slab lock */
	};
	struct list_head lru;		/* Pageout list, eg. active_list
					 * protected by zone->lru_lock !
					 */
	/*
	 * On machines where all RAM is mapped into kernel address space,
	 * we can simply calculate the virtual address. On machines with
	 * highmem some memory is mapped into kernel virtual memory
	 * dynamically, so we need a place to store that address.
	 * Note that this field could be 16 bits on x86 ... ;)
	 *
	 * Architectures with slow multiplication can define
	 * WANT_PAGE_VIRTUAL in asm/page.h
	 */
#if defined(WANT_PAGE_VIRTUAL)
	void *virtual;			/* Kernel virtual address (NULL if
					   not kmapped, ie. highmem) */
#endif /* WANT_PAGE_VIRTUAL */
#ifdef CONFIG_WANT_PAGE_DEBUG_FLAGS
	unsigned long debug_flags;	/* Use atomic bitops on this */
#endif

#ifdef CONFIG_KMEMCHECK
	/*
	 * kmemcheck wants to track the status of each byte in a page; this
	 * is a pointer to such a status block. NULL if not tracked.
	 */
	void *shadow;
#endif
};
```

这里额外说一下，由于每个页框都有对应的`struct page`对象，所以`struct page`对象有很多，这意味着`struct page`绝对不能太大，否则会非常影响内存效率。你看它都用上联合体了。

其中最重要的`flags`，它是一个比特位级参数，根据某个位是否被标记为1去判断这个块是否被使用，使用后是否被修改，释放时要不要写回硬盘之类的信息，除此之外还有`_count`字段，是一个引用计数，描述现在有多少个计算机资源再使用这个页框，比如父子进程共用的代码段数据，就会被标记为2或者更多（取决于子进程有多少个）

除此之外还有`struct list_head lru;`，这个结构体实际上是一个节点，链表或者队列或者其他数据结构的节点，如果这个页表需要被刷新，`lru`的指针就会在刷新队列里排队，就像进程控制块那种排队方式。

![绘图1](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202410191417075.png)

![image-20241124211209016](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411242112214.png)

除此之外，系统对于需要多个页框才能容纳的大块数据，会采用一种名为伙伴系统算法的方式进行管理。伙伴系统算法是一套方法，主要涵盖内存分配、内存释放和伙伴块寻找等功能。系统会将大块数据分成可被页框独立存储的一个个小块，随后系统将这些小块逐一存储到空闲页框中，并通过某种方法确定逻辑上（即在大块数据中）相邻的块之间的伙伴关系。当一个小块被释放时（从应用层来看，即其起始地址被 `free`），系统会将该小块的 `page` 对象相关标志位置为“空闲，可被覆写”状态，然后通过小块与小块之间的伙伴关系找到下一个小块，并将其也置为“空闲”状态，直到找不到下一个小块为止。通过小块之间的伙伴关系，系统无需直接维护所有小块的关系，仅需维护逻辑相邻块之间的伙伴关系，就能在逻辑上将所有小块组织成线性结构。

对于一些较小的内核级对象，如 `struct task_struct`、`struct files_struct`、`struct file`、`struct mm_struct` 等，它们在内核中频繁使用。当这些对象不再需要时，例如进程终止时，`struct task_struct` 并不会立即被释放，而是会被标记为“可覆写”状态。这样，当新的进程创建时，空闲的 `struct task_struct` 可以被重新使用，从而避免了频繁的内存分配和释放。我们称这种内存管理策略为 "slab 分配器"。

------------

由于硬盘需要频繁访问，尤其是系统文件，因此一些常用数据（如 `super block`、`Group Descriptor Table`、`Block Bitmap`、`inode Bitmap` 等）在开机时会被提前加载到内存中。这些数据虽然不大，但由于频繁使用，因此会保存在内存中以提高效率。

当用户打开一个文件时，系统会根据提供的文件路径找到该文件对应的 `inode` 编号，然后通过已加载的文件系统信息找到对应的 `inode` 对象。接着，系统会将 `inode` 中的大部分信息以 `struct inode` 的形式加载到内存中，并将指向 `struct inode` 的指针填充到相应的 `struct file` 结构中。因此，`struct file` 只存储少量的文件管理属性，而大多数文件属性则由 `struct inode` 存储。

<video src="https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411251011533.mp4"></video>

![image-20241125101605083](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411251016430.png)

接下来我们看看`struct file`里面的内核缓冲区长什么样。

![image-20241125102242565](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411251022796.png)

`struct file`内部还有一个指向`struct address_space`的指针

而在`struct address_space`内部，有一棵树，描述着存储文件内容的页号

![image-20241125102617002](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411251026827.png)

![image-20241125102938822](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411251029523.png)

这种树称为基数树或者基树，是一种比较少见的数据结构。

![image-20241125104007336](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411251040551.png)

基树的节点中有一个指针数组`slots`，其类型为`void*`，这意味着它可以指向不同类型的地址。

基树是一定深度的多叉树。根节点中`void* []`的每一个元素都指向下一层节点。

在下图中，我们假设指针数组的元素个数为3。深度为2。

![image-20241125105447916](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411251054008.png)

而在叶节点中，其`void*`指向的不是树的其它节点，而是`struct page`对象

![image-20241125105831321](https://md-wind.oss-cn-nanjing.aliyuncs.com/md/202411251058401.png)

而每个`page`都描述着一个页框，这些页框就是内核级的缓冲区。

如果想要指向更多`page`就需要增加深度和增大`void* []`的元素个数。

这里的基树实际上是一种字典树，什么是字典树呢？就拿上图举例，你可以认为这三个节点分别代表`abc`三个字符，现在让我们扩大一下上面基树的深度，把它的深度变为4，这样，第一层就有一个节点，第二层就有三个节点，第三层就有九个节点，第四层就有二十七个节点，第四层是叶节点层，它们指向其它对象。

- 第一层有 1 个节点，代表根节点。
- 第二层有 3 个节点，分别代表字符 `a`、`b` 和 `c`。
- 第三层有 9 个节点，代表所有两字符组合，如 `aa`、`ab`、`ac` 等。
- 第四层有 27 个节点，代表所有三字符组合，如 `aaa`、`aab`、`abc` 等。

那么就可以用`abc`这个字符串指代一个对象，先在第二层找到节点`a`，接着在第三层找到节点`b`，第四层找到节点`c`，之后就能找到这个对象。

对于一个 10MB 的文件，当页框大小为 4KB 时，文件会被分割成 2560 个小块。通过伙伴算法，我们可以根据每个小块的起始块，确定其他小块的伙伴关系。因此，我们可以为每个小块进行逻辑上的编号，编号区间是 [1, 2560]。

由于这些编号都是整型数，可以使用 32 位来表示，每个编号需要 4 字节（32 位）。因此，我们可以用基树节点（`void* []`）的数量为 16 来构建一个深度为 9 的字典树。（16——十六进制，9——32比特位用十六进制表示有8个位）

例如，要查找逻辑编号为 2356 的块，首先将其转换为十六进制 `0x00000934`。然后，在字典树的第二层，我们首先找到表示字符 `0` 的节点，接着在该节点下找到表示字符 `0` 的第三层节点，以此类推。最终，在第六层的 `0` 节点中，我们会找到指向第七层的 `9` 节点，再到 `3` 和 `4` 节点，最终得到一个指针，指向物理内存中的页框 `page`。

当进程修改文件时，系统根据文件的偏移量计算出对应的逻辑编号，从而定位到相应的物理内存页框。至于具体的刷新任务，CPU 并不直接处理，它仅下达 I/O 指令。实际的 I/O 操作由 ADM 芯片及其管理的 I/O 线程子系统负责，刷新任务由它们执行。关于线程的更多细节，我们会在后续内容中进一步讨论。

系统内部存在大量 I/O 请求，针对这些请求，系统将需要写回硬盘的文件 `page` 对象指针封装在 `struct request` 中，并将 `request` 指针加入 I/O 队列，等待硬盘响应。硬盘在接收到请求后，会对内部的 `page` 进行排序，将同一柱面的 I/O 请求集中处理，从而优化响应效率。

# end
