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
