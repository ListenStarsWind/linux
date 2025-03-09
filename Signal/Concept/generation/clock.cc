#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<signal.h>
#include<iostream>
#include<string>

using namespace std;

int main(int argc, char* argv[])
{
    pid_t id = fork();
    if(id == 0)
    {
        cout<<"child-pid: "<<getpid()<<endl;
        // 子进程执行流
        // raise(argv[1][1] - '0');  // 给自己发送一个信号
        int a = 1 / 0;
        exit(0);
    }
    else
    {
        // 父进程执行流
        int state = 0;
        waitpid(id, &state, 0);
        if(state & 0x7f)
        {
            cout<<"进程异常退出, 错误码为"<<(state & 0x7f)<<endl;
            if((state >> 7) & 1)
            cout<<"生成核心转储文件"<<endl;
            else
            cout<<"未生成核心转储文件"<<endl;
        }
        else
        {
            cout<<"进程正常退出, 退出码为"<<((state >> 8) & 0xff)<< endl;
        }
    }
    return 0;
}


void handler(int event)
{
    cout << "接收到信号: " << event << endl;
    unsigned int n = alarm(5);
    cout << "剩余时间  : "<< n <<"s"<<endl;
}


// int main()
// {
//     signal(SIGALRM, handler);

//     // 设定一个五秒后响的闹钟
//     alarm(50);

//     int count = 0;
//     while(true)
//     {
//         cout<<count<<"s"<<endl;
//         if(count == 7) raise(SIGALRM);
//         count++;
//         sleep(1);
//     }

//     return 0;
// }