#ifndef __MY_DAEMON_
#define __MY_DAEMON_

#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include"task.hpp"

void daemon(const std::string &cwd = std::string())
{
    // 创建守护进程一般也要屏蔽一些信号
    // 让守护进程不被信号打扰, 安心服务
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);

    // fs<<"tcpserver is running...."<<"\n";

    // 创建新会话
    if (fork() > 0)
    {
        // 最开始猜测可能是这里父进程挂的太快了
        // 子进程没有完全继承父进程状态
        // 但是, 可以做个实验, 证明不是这样
        // sleep(5);
        exit(0);
    }

    // fs<<"tcpserver is running...."<<"\n";


    setsid();

    // // 守护进程常被直接安装在系统上
    // // 它的各类配置文件或输出文件都会在系统的特定位置上
    // // 所以有时需要更改工作目录, 直接移到根目录下
    if (cwd.size() != 0)
    {
        chdir(cwd.c_str());
    }

    int fd = open("/dev/null", O_RDWR);
    if(fd > 0)
    {
         dup2(fd, 0);
         dup2(fd, 1);
         dup2(fd, 2);
         close(fd);
    }
}


#endif