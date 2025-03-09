#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>
#include<iostream>
#include<vector>

using namespace std;

class processTable
{
    static vector<pid_t> _v;

    public:
    static void push(pid_t id)
    {
        _v.push_back(id);
    }

    static pid_t tail()
    {
        return _v[_v.size() - 1];
    }

    static void pop()
    {
        _v.pop_back();
    }

    static void clear()
    {
        _v.clear();
    }
};

vector<pid_t> processTable::_v;

void handler(int event)
{
    static bool flag = true;
    cout<<"这里是"<<getpid()<<"↓"<<endl;
    cout << "接收到一个信号: " << event << endl;
   while(true)
   {
        pid_t id = waitpid(-1, nullptr, WNOHANG);
        if(id <= 0) break;
        cout << "回收子进程资源: "<<id<<endl;
   }

    // 模拟信号处理过程中, 同时有多个进程退出
    if(flag)
    {
        flag = false;
        int count = 4;
        while(count--)
        {
            pid_t id = processTable::tail();
            processTable::pop();

            cout<<"子进程"<< id <<"即将退出"<<endl;
            kill(id, 9);
        }
    }

   cout<<endl;
}

int main()
{
    // 覆写信号
    signal(17, handler);

    // 屏蔽信号
    sigset_t set, oset;
    sigemptyset(&set);
    sigaddset(&set, 17);
    sigprocmask(SIG_SETMASK, &set, &oset);

    cout << "这里是父进程: "<<getpid()<<endl;
    int count = 5;
    while(count--)
    {
        pid_t id = fork();
        if(id == 0)
        {
            processTable::clear();
            cout << "这里是子进程: "<<getpid()<<endl;
            while(true);
            exit(0);
        }
        else
        {
            processTable::push(id);
        }
    }

    sleep(1);
    pid_t tail = processTable::tail();
    processTable::pop();
    cout<<"子进程"<<tail<<"即将退出"<<endl;
    kill(tail, 9);
    sleep(2);
    sigprocmask(SIG_SETMASK, &oset, nullptr);
    // 信号处理

    // 确保有足够的处理时间
    while(true);

    return 0;
}
