#include<iostream>
#include<cstring>   // C++里也有对应的C头文件，此处头文件原型为string.h
#include<unistd.h>
#include<cstdio>
#include<sys/types.h>
#include<sys/wait.h>


#define R 1 << 0
#define W 1 << 1

using namespace std;

#define BUFFER_SIZE 4096

// 父进程任务集
class parent_task
{
    public:
    // 父进程执行流
    void operator()(int arr[2], int id)
    {
        close(arr[1]);
        const int& read_fd = arr[0];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 读取信息
        int loop_counter = 1;
        while(1)
        {
            buffer[0] = 0;
            ssize_t end = read(read_fd, buffer, sizeof(buffer) - 1);

            // // 查看read的返回情况
            // cout<<end<<endl;

            // 异常处理
            if(end > 0)
            {
                buffer[end] = 0;
                // cout<<buffer<<"  "<<loop_counter<<endl;
                cout<<buffer<<endl;
            }
            else if(end == 0)
            {
                // 写端已经关闭，通信结束
                break;
            }
            else
            {
                // 未知错误，也跳出循环
                break;
            }

            sleep(6);

            close(read_fd);

            cout<<"The read end has been closed."<<endl; // 读端已经关闭
            break;

            // // loop_counter+=2;
            // // loop_counter++;
        }

        sleep(8);  // 创建一个窗口期观察子进程的僵死状态
        int status = 0;
        pid_t i = waitpid(id, &status, 0);

        // 在ProcessWaiting.md中对waitpid的使用和返回有详细介绍
        if(WIFEXITED(status))
        {
            printf("Normal exit, code : %d\n", WEXITSTATUS(status));
        }
        else
        {
            printf("Exception Interrupt, code : %d\n", status & 0x7f);
        }
        exit(0);
    }
};

// 子进程任务集
class child_task
{
    public:
    // 子进程执行流
    void operator()(int arr[2])
    {
        close(arr[0]);
        const int& write_fd = arr[1];
        char buffer[BUFFER_SIZE];  // 用户缓冲区

        // 发出信息
        int count = 1;
        while(count)
        {
            // 生成一条信息
            // buffer[0] = 0; // 在C/C++中字符串结尾为'\0'，此处暗示该缓冲区是字符串的载体
            // pid_t id = getpid();
            // snprintf(buffer, sizeof(buffer), "%s:: pid:%d count:%d", "Here is the child process", id, count);
            snprintf(buffer, sizeof(buffer), "%s", "c");

            // 将信息写到管道中
            write(write_fd, buffer, strlen(buffer));

            sleep(10);
            cout<<"The write end is preparing for the next write operation."<<endl; // 写段准备下一次写入

            // cout<<count<<endl;

            // if(count == 65536)
            // {
            //     cout<<"ready"<<endl;
            // }
            // else if(count > 65536)
            // {
            //     cout<<"go"<<endl;
            // }

            if(count == 5)
                break;

            count++;
            sleep(1);
            // cout<<buffer<<endl;  检查
        }

        exit(0);
    }
};

// 这是一个管道测试程序
// 其中子进程用于写，父进程用于读
int main()
{
    // 信道的建立
    int arr[2] = {0};
    int i = pipe(arr);
    if(i == -1)
    {
        perror("failed pipe");
        return 1;
    }

    pid_t id = fork();
    if(id < 0 )
    {
        perror("failed fork");
        return 2;
    }

    parent_task parent_process;
    child_task child_process;

    if(id == 0)
        child_process(arr);
    else
        parent_process(arr, id);

    return 0;
}