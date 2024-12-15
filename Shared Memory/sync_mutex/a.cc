#include"shmm.hpp"

int main()
{
    // 创建并映射共享地址
    key_t key = ftok(PATHNAME, KEYHINT);
    int md = shmget(key, SHMM_SIZE, IPC_CREAT | IPC_EXCL | SHMM_Mode);
    char* ptr = (char*)shmat(md, nullptr, 0);

    mkfifo(FIFO_NAME, MODE);            // 创建命名管道
    int fd = open(FIFO_NAME, O_RDONLY); // 打开管道读端

    // 通信
    while(1)
    {
        char c;
        ssize_t n = read(fd, &c, sizeof(char));
        if(n == 0 || n < 0) break;
        printf("Message received: %s", ptr);
    }

    close(fd);  // 关闭读端
    shmdt(ptr); // 解除映射关系
    unlink(FIFO_NAME);             // 删除管道文件
    shmctl(md, IPC_RMID, nullptr); // 删除共享内存

    return 0;
}