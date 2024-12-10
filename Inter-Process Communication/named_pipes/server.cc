#include"comm.h"
#include<sys/types.h>


using namespace std;

int main()
{
    // 创建命名管道
    if(mkfifo(FIFO_NAME, MODE) != 0)
    {
        perror("failed mkfifo");
        exit(FIFO_CREATE_ERROR);
    }

    // 建立信道
    int fd = open(FIFO_NAME, O_RDONLY);
    if(fd < 0)
    {
        perror("failed open");
        exit(FIFO_OPEN_ERROR);
    }

    cout<<"The server is now able to receive information."<<endl;

    // 数据处理
    char buffer[1024];
    while(1)
    {
        buffer[0] = 0;
        ssize_t  n = read(fd, buffer, sizeof(buffer) - 1);
        if(n > 0)
        {
            buffer[n] = 0;
            cout<<"client say# "<<buffer<<endl;
        }
        else if(n == 0)
        {
            cout<<"client quit, me too."<<endl;
            break;
        }
        else
        {
            perror("failed read");
            exit(FIFO_READ_ERROR);
        }
    }


    // 善后操作
    close(fd);
    if(unlink(FIFO_NAME) != 0)
    {
        perror("failed unlink");
        exit(FIFO_REMOVE_ERROR);
    }

    return 0;
}