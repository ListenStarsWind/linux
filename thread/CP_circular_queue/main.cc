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