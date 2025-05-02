#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<cstdio>
#include<cstring>

using namespace std;

void SetNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
    {
        cerr << "fcntl get flags error" << endl;
        return;
    }
    flags |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flags) == -1)
    {
        cerr << "fcntl set flags error" << endl;
        return;
    }
}

int main()
{
    SetNonBlock(0);
    char buffer[1024];
    while(true)
    {
        // printf("Pleace input : ");
        // fflush(stdout);
        ssize_t n = read(0, buffer, sizeof(buffer) - 1);
        if(n > 0)
        {
             buffer[n-1] = 0;  // -1是为了去掉换行符
             cout << "echo : "<<buffer<<endl;
        }
        else if (n == 0)
        {
            // 对于标准输入, 可以通过ctrl + d来结束
            cout << "read done " << endl;
            break;
        }
        else
        {
            if(errno == EWOULDBLOCK)
            {
                // 非阻塞, 没有数据可读
                cout << "read nothing" << endl;
                // do_something(); 可以做一些别的事
            }
            else
            {
                cerr<<"read error: "<<strerror(errno)<<"["<<errno<<"]"<<endl;
                break;
            }
        }
        sleep(1);
        // 这里sleep是为了让我们可以看到非阻塞的效果
    }
    return 0;
}