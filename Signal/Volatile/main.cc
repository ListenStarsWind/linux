#include<iostream>
#include<signal.h>
#include<sys/types.h>
#include<unistd.h>

using namespace std;

int flag = 1;

void handler(int event)
{
    flag = 0;
    cout << endl << "flag = " << flag << endl;
}

int main()
{
    cout << "pid: "<<getpid()<<endl;
    signal(2, handler);
    while(flag);

    cout << "after" << endl;

    return 0;
}