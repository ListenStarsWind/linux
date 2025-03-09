#include <pthread.h>   // 线程库的头文件
#include<iostream>
#include<string>
#include<vector>
#include<unistd.h>
#include<thread>
#include<cstdio>
#include<ctime>
#include<cstdio>
#include<cerrno>
#include<cstring>

using namespace std;

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

// thread_local void*a = nullptr;
// thread_local string b("cdsccf");

// struct Args
// {
//     string _id;
//     int _num;
//     Args(int id, int num)
//         :_id("thread-" + to_string(id))
//         ,_num(num)
//         {}
// };

// void* pthread_behavior(void* p)
// {
//     auto* args = static_cast<Args*>(p);
//     cout << "start"<< endl;
//     pthread_detach(pthread_self());
//     sleep(5);
//     cout << "end"<<endl;
//     return nullptr;
// }

// int main()
// {
//     srand((unsigned int)time(nullptr));
//     vector<pthread_t> threads;
//     vector<Args*> va;
//     int num = 5;
//     for(int i = 1; i <= num; ++i)
//     {
//         pthread_t id;
//         auto* p = new Args(i, rand());
//         va.push_back(p);
//         pthread_create(&id, nullptr, pthread_behavior, p);
//         threads.push_back(id);
//     }

//     // // 确保都分离了
//     // sleep(1);
//     // for(auto thread: threads)
//     // {
//     //     int result = pthread_join(thread, nullptr);
//     //     if(result == 0)
//     //     {
//     //         printf("%s: %ld\n", "汇合成功", thread);
//     //     }
//     //     else
//     //     {
//     //         printf("%s: %ld", "汇合失败", thread);
//     //         printf("   %s:%s\n", "失败原因", strerror(result));
//     //     }
//     // }

//     // 考虑到线程可能创建失败,  释放在主线程阶段或许更安全
//     for(auto p : va)
//     {
//         delete p;
//     }

//     return 0;
// }

// void* pthread_behavior(void* p)
// {
//     int count = 5;
//     while(count--)
//     {
//         // pthread_self能返回自身的线程id
//         printf("%p\n", pthread_self());
//         sleep(1);
//     }
//     return nullptr;
// }

// int main()
// {
//     pthread_t tid;
//     pthread_create(&tid, nullptr, pthread_behavior, nullptr);
//     pthread_join(tid, nullptr);
//     return 0;
// }

// void threadrun()
// {
//     while(true)
//     {
//         cout << "这是使用C++线程库创建的新线程"<<endl;
//         sleep(1);
//     }
// }

// int main()
// {
//     thread t1(threadrun);
//     t1.join();
//     return 0;
// }

// void* thread_behavior(void* p)
// {
//     int& end = *static_cast<int*>(p);
//     int* ret = new int();
//     for(int start = 1; start <= end; ++start)
//     {
//         *ret += start;
//     }
//     // 确保有足够空闲时间让pthread_cancel发挥作用
//     sleep(2);
//     pthread_exit(static_cast<void*>(ret));
// }

// int main()
// {
//     pthread_t thread;
//     int end = 100;
//     pthread_create(&thread, nullptr, thread_behavior, &end);
//     pthread_cancel(thread);
//     int* ret = nullptr;
//     pthread_join(thread, (void**)&ret);
//     printf("%d\n", ret);
//     return 0;
// }

// void* thread_behavior(void* x)
// {
//     while(true)
//     {
//         sleep(2);
//     }
// }

// int main()
// {
//     pthread_t tid;
//     pthread_create(&tid, nullptr, thread_behavior, nullptr);
//     printf("%ul\n", tid);
//     printf("%p\n", tid);
//     return 0;
// }

// int num = 0;

// void* thread_behavior(void* x)
// {
//     string& str = *static_cast<string*>(x);
//     while(true)
//     {
//         cout << str << num << endl;
//         sleep(2);
//     }
// }

// int main()
// {
//     pthread_t tid;
//     auto p = new string();
//     string& str = *p;
//     pthread_create(&tid, nullptr, thread_behavior, p);
//     while(true)
//     {
//         int flag = num & 3;
//         switch(flag)
//         {
//             case 0:
//             str = "Never gonna give you up               ";break;
//             case 1:
//             str = "Never gonna let you down              ";break;
//             case 2:
//             str = "Never gonna run around and desert you ";break;
//             case 3:
//             str = "Never gonna make you cry              ";break;
//         }
//         ++num;
//         sleep(2);
//     }
//     return 0;
// }


// void* thread_behavior(void* x)
// {
//     while(true)
//     {
//         cout << "副线程# pid:"<<getpid()<<endl;
//         sleep(1);
//     }
// }

// int main()
// {
//     pthread_t tid;
//     pthread_create(&tid, nullptr, thread_behavior, nullptr);
//     while(true)
//     {
//         cout << "主线程# pid:"<<getpid()<<endl;
//         sleep(1);
//     }

//     return 0;
// }