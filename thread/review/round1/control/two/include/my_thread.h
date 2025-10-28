#pragma once
#define _GNU_SOURCE

#include <asm/ldt.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>

#include <cstring>

// 应用层线程结构体
typedef struct {
    pid_t tid;              // 内核轻量级进程ID
    pid_t child_tid;        // 线程退出标志位
    void* (*start)(void*);  // 应用级任务函数
    void* arg;              // 应用级任务参数
    void* tls_men;          // 局部存储段地址
    void* result;           // 应用级任务结果
    int joined;             // 是否已经合并
} my_thread_t;

// 面向内核的任务函数
int __start_routine(void* arg);

// 面向应用的线程创建
int my_thread_create(my_thread_t* newthread, void* (*start_routine)(void*), void* arg);
