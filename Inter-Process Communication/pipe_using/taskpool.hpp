#include <unistd.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>

namespace wind
{
    struct channel
    {
        int _cmdfd;               // 负责发送指令的文件描述符
        pid_t _taskid;            // 子进程的pid
        std::string _processname; // 子进程的代称

        channel(int cmdfd, int taskid, const std::string &name = std::string())
            : _cmdfd(cmdfd), _taskid(taskid), _processname(name)
        {
        }
    };

    class taskpool
    {
        typedef void (*func)();

    public:
        taskpool(std::vector<func> tasks, int processnum = 5)
            : _tasks(tasks)
        {
            while (processnum--)
            {
                int pipeid[2];
                int i = pipe(pipeid);

                // 管道异常处理
                if (i != 0)
                {
                    fprintf(stderr, "%s\n", "Fatal error, pipe cannot be created.");
                    exit(1);
                }

                pid_t id = fork();
                if (id > 0)
                {
                    // 父进程流
                    close(pipeid[0]); // 关闭读端

                    // 输出提示
                    fprintf(stderr, "%s:%d %s%d\n", "子进程创建成功 pid", id, "信道为", pipeid[1]);

                    _channels.push_back(channel(pipeid[1], id));
                }
                else if (id == 0)
                {

                    for (const auto &e : _channels) // 关闭从父进程继承下来的其余兄弟进程管道
                    {
                        close(e._cmdfd);
                    }

                    // 子进程流
                    close(pipeid[1]);   // 关闭写端
                    dup2(pipeid[0], 0); // 将管道重定向为标准输入
                    close(pipeid[0]);   // 顺手关掉原文件
                    while (1)
                    {
                        // printf("%s:%d\n", "这是子进程", getpid());
                        int i;
                        ssize_t n = read(0, &i, sizeof(int));
                        printf("子进程%d所读到的字节数:%d\n", getpid(), n);
                        if (n == sizeof(int))
                        {
                            _tasks[i]();
                        }
                        else if (n == 0)
                        {
                            // 写端已经关闭
                            break;
                        }
                        else
                        {
                            fprintf(stderr, "%s\n", "Fatal error, Read failed.");
                        }
                    }
                    printf("%s:%d\n", "这个子进程即将退出", getpid());
                    exit(0);
                }
                else
                {
                    // 进程异常处理
                    fprintf(stderr, "%s\n", "Fatal error, process cannot be created.");
                }
            }
        }

        int size()
        {
            return _channels.size();
        }

        bool call(int taskcode)
        {
            static int scheduling = 0;
            if (taskcode >= 0 && taskcode < _tasks.size())
            {
                printf("向%d发送消息\n", _channels[scheduling]._taskid);
                write(_channels[scheduling]._cmdfd, &taskcode, sizeof(int));
                scheduling++;
                if (scheduling == _channels.size())
                    scheduling = 0;
                return true;
            }
            else
            {
                // 非法的任务码
                return false;
            }
        }

        ~taskpool()
        {
            for (const auto &e : _channels)
            {
                int ret = close(e._cmdfd);

                fprintf(stderr, "%s:%d\n", "信道已关闭", e._cmdfd);

                int status = 0;
                pid_t i = waitpid(e._taskid, &status, 0);

                // 在ProcessWaiting.md中对waitpid的使用和返回有详细介绍
                if (i > 0)
                {
                    // 回收成功
                    printf("回收进程资源%d\n", e._taskid);

                    // 查看子进程退出状态
                    if (WIFEXITED(status))
                    {
                        printf("正常退出, 退出码 : %d\n", WEXITSTATUS(status));
                    }
                    else
                    {
                        printf("异常退出, 退出码 : %d\n", WTERMSIG(status));
                    }
                }
                else if (i < 0)
                {
                    // waitpid自己出错了
                    fprintf(stderr, "%s\n", "Fatal error, Waitpid Failed.");
                }
                else
                {
                    // 实际上不会用到该分支，顺手写的，若是采用轮询式等待，具有返回0的可能性
                    fprintf(stderr, "%s\n", "No child process state changed.");
                }

                // int status = 0;
                // pid_t i = waitpid(e._taskid, &status, 0);
                // if (i == -1)
                // {
                //     perror("waitpid failed");
                // }
                // else if (WIFEXITED(status))
                // {
                //     fprintf(stderr, "正常退出, 退出码 : %d\n", WEXITSTATUS(status));
                // }
                // else if (WIFSIGNALED(status))
                // {
                //     fprintf(stderr, "被信号%d消除\n", WTERMSIG(status));
                // }
                // else
                // {
                //     fprintf(stderr, "未知的退出状态\n");
                // }
            }

            // int i = 0;
            // while(i <_channels.size())
            // {
            //     int status = 0;
            //     // pid_t ret = waitpid(_channels[i]._taskid, &status, 0); // 外部阻塞等待
            //     pid_t ret = waitpid(_channels[i]._taskid, &status, WNOHANG);

            // if (ret > 0)
            // {
            //     i++;
            //     printf("回收进程资源%d\n", ret);
            //     if (WIFEXITED(status))
            //     {
            //         printf("正常退出, 退出码 : %d\n", WEXITSTATUS(status));
            //     }
            //     else
            //     {
            //         printf("异常退出, 退出码 : %d\n", status & 0x7f);
            //     }
            // }
            // else if (ret < 0)
            // {
            //     perror("failed wait");
            // }
            // else
            // {
            //     // 总有一个进程轮询等待返回为0
            //     printf("未获得等待状态\n");
            // }
            //     sleep(1);
            // }
        }

    private:
        std::vector<channel> _channels;
        std::vector<func> _tasks;
    };
}

// 后记：一些额外的东西
// 三种传参类型的一般约定
// 输入：常引用
// 输出：指针
// 输入输出：引用
