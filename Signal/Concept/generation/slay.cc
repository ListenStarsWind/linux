#include <sys/types.h>
#include <signal.h>
#include<unistd.h>
#include<iostream>

using namespace std;

void handler(int event)
{
        cout << "process get a signal:" << event << endl;
}

int main()
{
    signal(SIGABRT, handler);
    int count = 5;
    while (true)
    {
        cout << "This is a process. pid:" << getpid() << endl;
        sleep(1);
        count--;
        if(!count)
        {
            abort();
        }
    }

    return 0;
}