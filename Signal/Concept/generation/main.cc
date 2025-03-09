#include <iostream>
#include <unistd.h>
#include <signal.h>

using namespace std;

// 为了方便区分 这里把参数名取为event
// 因为信号其实也映射着某种行为
// 所以可以把信号抽象成行为
void handler(int event)
{
    if (event == SIGINT)
    {
        cout << "这是2号信号:SIGINT" << endl;
        cout << "它的默认行为是进程终止" << endl;
        cout << "现在它的行为函数被重写了" << endl;
    }
    else if (event == SIGQUIT)
    {
        cout << "这是3号信号:SIGQUIT" << endl;
        cout << "它的默认行为也是终止进程" << endl;
        cout << "现在我们不考虑区别" << endl;
    }
    else if (event == 19)
    {
        cout << "这是19号信号:SIGSTOP" << endl;
        cout << "它的默认行为是暂停进程" << endl;
        cout << "它不能被重写" << endl;
        cout << "继续信号是18" << endl;
        cout << "这是无法被执行的分支" << endl;
    }
    else if (event == SIGKILL)
    {
        cout << "这是无法被执行的分支" << endl;
        cout << "它不能被重写" << endl;
        cout << "这是9号信号:SIGKILL" << endl;
        cout << "用于强制终止进程" << endl;
        cout << "上面两个终止是请求" << endl;
        cout << "不具有强制性" << endl;
    }
    else
    {
        // 对于标准信号
        // 只有9号和19号不支持重写
        // 9号是强制终止
        // 如果被重写
        // 进程就关不掉了
        // 当某些进程出现问题时
        // 由于它很重要
        // 所以不能直接强制终止
        // 所以就暂时停止
        // 以减少损失
        cout << "process get a signal:" << event << endl;
    }
}

int main()
{
    // for (int i = 0; i <= 31; i++)
    // {
    //     signal(i, handler); // 放置一个陷阱 用于捕获信号
    // }
    while (true)
    {
        cout << "This is a process. pid:" << getpid() << endl;
        sleep(1);
    }
    return 0;
}