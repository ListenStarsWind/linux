#include<sys/types.h>
#include<signal.h>
#include<unistd.h>
#include<iostream>

using namespace std;

void handler(int event)
{
    cout<<"接收到一个信号: "<<event<<endl;
    sleep(1);
}

int main()
{
    signal(SIGSEGV, handler);
    cout<<"pid:"<<getpid()<<endl;
    cout<<"exception before"<<endl;
    sleep(1);

    int* p = nullptr;
    *p = 1;

    sleep(1);
    cout<<"exception after"<<endl;

    return 0;
}