#include"shmm.hpp"

int main()
{
    // 创建并映射共享地址
    key_t key = ftok(PATHNAME, KEYHINT);
    int md = shmget(key, SHMM_SIZE, IPC_CREAT);
    char* ptr = (char*)shmat(md, nullptr, 0);

    int fd = open(FIFO_NAME, O_WRONLY); // 打开管道写端

    // 通信
    while(1)
    {
        printf("Please enter: ");
        fgets(ptr, SHMM_SIZE, stdin);
        write(fd, "c", sizeof(char));
    }

    close(fd);  // 关闭写端
    shmdt(ptr); // 解除映射关系

    return 0;
}