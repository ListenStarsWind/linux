#include<signal.h>
#include<unistd.h>
#include<iostream>

using namespace std;

// 将sigset_t类型变量以二进制序列的形式打印出来
void print(const sigset_t& information)
{
    for(int i = 31; i >= 0; i--)
    {
        if(sigismember(&information, i) == 1)
            cout << 1;
        else
            cout << 0;
    }
    cout<<endl;
}

void handler(int event)
{
    cout<<"================================"<<endl;
    sigset_t pending;
    sigpending(&pending);
    print(pending);
    exit(0);
}

int main(int argc, char* argv[])
{
    // 解析要处理的信号
    int signaln = stoi(argv[1] + 1);

    // 定义并初始化用户层的sigset_t变量, 作为待输入位图信息的载体
    sigset_t set, oset;
    sigemptyset(&set);

    // 将位图信息写入载体
    sigaddset(&set, signaln);

    // 将位图信息由用户层传送入内核层
    sigprocmask(SIG_SETMASK, &set, &oset);   // 屏蔽信号

    // 轮询pending, 间隔1秒, 进行10次
    for(size_t i = 0; i < 10; i++)
    {
        sigset_t pending;
        sigpending(&pending);
        print(pending);

        // 让第五秒处于信号被记录状态
        if(i == 3)
            raise(signaln);

        sleep(1);
    }

    // 覆写信号的默认行为
    signal(signaln, handler);

    // 恢复原有的block
    sigprocmask(SIG_SETMASK, &oset, nullptr);

    // 信号处理

    return 0;
}