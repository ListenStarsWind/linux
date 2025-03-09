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
