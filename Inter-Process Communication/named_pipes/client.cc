#include"comm.h"


using namespace std;
int main()
{
    int fd = open(FIFO_NAME, O_WRONLY);
    if(fd < 0)
    {
        perror("failed open");
        exit(FIFO_OPEN_ERROR);
    }

    cout<<"The client is now able to send messages."<<endl;

    string buffer;
    while(1)
    {
        cout<<"Please input# ";
        getline(cin, buffer); // 不把空格作为分隔符
        if(write(fd, buffer.c_str(), buffer.size()) < 0)
        {
            perror("failed wwirte");
            exit(FIFO_WRITE_ERROR);
        }
    }

    close(fd);
    return 0;
}