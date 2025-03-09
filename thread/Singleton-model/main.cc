#include"thread_pool.hpp"
#include"task.hpp"
#include<unistd.h>
#include<ctime>
#include<iostream>

using namespace std;

int main()
{
    srand((unsigned int)time(nullptr));
    auto p = thread_pool<task>::getInstance();
    p->start();
    while(true)
    {
        sleep(1);
        int left = rand() % 10;
        int right = rand() % 10;
        int op = rand() % 4;
        p->push(left, op, right);
    }
    return 0;
}