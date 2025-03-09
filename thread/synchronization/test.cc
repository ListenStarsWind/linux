#include"blocking_queue.hpp"
#include"task.hpp"
#include<cstdio>
#include<unistd.h>
#include<time.h>
#include<stdlib.h>
#include<iostream>
#include<string.h>

using namespace std;

void* producer(void* _p)
{
    pthread_detach(pthread_self());
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
        q->emplace(left, op, right);
        sleep(1);
    }
}

void* consumer(void* _p)
{
    pthread_detach(pthread_self());
    blocking_queue<task>* q = static_cast<blocking_queue<task>*>(_p);
    while(true)
    {
        auto task = q->front();
        auto result = task();
        cout << "[consumer]<"<<pthread_self()<<">info: ";
        if(result.second == 0)
            cout << "The result is "<< result.first<<endl;
        else
            cout << "error! "<<strerror(result.second)<<endl;
    }
}

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