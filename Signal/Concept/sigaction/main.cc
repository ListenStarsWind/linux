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
    static bool flag = true;
    cout <<"{"<<endl;
    sigset_t pending, block;
    sigpending(&pending);                           // 获取pending表
    sigprocmask(SIG_SETMASK, nullptr, &block);      // 获取block表   第二个参数为空时, 第一个参数无意义

    if(flag)
    {
        raise(2);                           // 向自己发送二号信号
        flag = false;
    }

    cout <<"    pending: ";
    print(pending);
    cout <<"    block  : ";
    print(block);
    cout<<"}"<<endl;
    cout << endl;
}

void SetToNull(struct sigaction& act)
{
    sigset_t set;
    sigemptyset(&set);
    act.sa_handler = nullptr;
    act.sa_sigaction = nullptr;
    act.sa_mask = set;
    act.sa_flags = 0;
    act.sa_restorer = nullptr;
}

int main()
{
    // 对一号和二号信号进行阻塞
    sigset_t set, oset;
    sigemptyset(&set);

    sigaddset(&set, 1);
    sigaddset(&set, 2);

    sigprocmask(SIG_SETMASK, &set, &oset);

    // 对自己发送一号信号
    raise(1);

    // 对一号信号和二号信号进行捕捉
    struct sigaction act;
    SetToNull(act);
    act.sa_handler = handler;
    act.sa_mask = set;
    sigaction(1, &act, nullptr);
    sigaction(2, &act, nullptr);

    // 对一号信号和二号信号取消阻塞
    sigprocmask(SIG_SETMASK, &oset, nullptr);

    // 信号处理

    return 0;
}

// int main(int argc, char* argv[])
// {
//     // 解析要处理的信号
//     int signaln = stoi(argv[1] + 1);

//     // 定义并初始化用户层的sigset_t变量, 作为待输入位图信息的载体
//     sigset_t set, oset;
//     sigemptyset(&set);

//     // 将位图信息写入载体
//     sigaddset(&set, signaln);

//     // 将位图信息由用户层传送入内核层
//     sigprocmask(SIG_SETMASK, &set, &oset);   // 屏蔽信号

//     // 轮询pending, 间隔1秒, 进行10次
//     for(size_t i = 0; i < 10; i++)
//     {
//         sigset_t pending;
//         sigpending(&pending);
//         print(pending);

//         // 让第五秒处于信号被记录状态
//         if(i == 3)
//             raise(signaln);

//         sleep(1);
//     }

//     // 覆写信号的默认行为
//     struct sigaction act;
//     SetToNull(act);
//     act.sa_handler = handler;
//     sigaction(signaln, &act, nullptr);

//     // 恢复原有的block
//     sigprocmask(SIG_SETMASK, &oset, nullptr);

//     // 信号处理

//     return 0;
// }