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