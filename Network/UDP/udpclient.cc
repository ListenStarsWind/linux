#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;

#define SOCKET_ERR       1
#define BUFFER_SIZE      1024

struct push_args
{
    int _fd;
    struct sockaddr_in _server;
};

void* push(void* args)
{
    push_args* p_ = reinterpret_cast<push_args*>(args);
    int& sockfd = p_->_fd;
    struct sockaddr_in& remote = p_->_server;
    string message;

    while(true)
    {
        cout << "Pleace Enter@ ";
        getline(cin, message);

        if(sendto(sockfd, message.c_str(), message.size(), 0, reinterpret_cast<const sockaddr*>(&remote), static_cast<socklen_t>(sizeof(remote))) == -1)
        {
            cout << " sendto error: " << strerror(errno);
        }
    }
    return nullptr;
}

void* pull(void* args)
{
    int& sockfd = *(reinterpret_cast<int*>(args));
    char buffer[BUFFER_SIZE];
    while(true)
    {
        struct sockaddr_in temp;
        socklen_t size = sizeof(temp);
        ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&temp), reinterpret_cast<socklen_t *>(&size));
        if (len < 0)
        {
            cout << " recvfrom error: " << strerror(errno) << endl;
            continue;
        }
        buffer[len] = '\0';
        cerr << buffer;
    }
    return nullptr;
}


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    string remote_ip(argv[1]);
    in_port_t remote_port = stoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        cout << "socket create error: "<< strerror(errno)<<endl;
        exit(SOCKET_ERR);
    }
    cout << "socket create success, sockfd: "<<sockfd<<endl;

    // 交由系统自动绑定

    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons(remote_port);
    remote.sin_addr.s_addr = inet_addr(remote_ip.c_str());

    struct push_args i;
    i._fd = sockfd;
    i._server = remote;


    // // 输出重定向
    // int fd = open("/dev/pts/11", O_WRONLY);
    // dup2(fd, 2);

    pthread_t in, out;
    pthread_create(&out, nullptr, push, &i);
    pthread_create(&in, nullptr, pull, &sockfd);

    pthread_join(in, nullptr);
    pthread_join(out, nullptr);


    // string message;
    // char buffer[BUFFER_SIZE];
    // while(true)
    // {
    //     cout << "Pleace Enter@ ";
    //     getline(cin, message);

    //     if(sendto(sockfd, message.c_str(), message.size(), 0, reinterpret_cast<const sockaddr*>(&remote), static_cast<socklen_t>(sizeof(remote))) == -1)
    //     {
    //         cout << " sendto error: " << strerror(errno);
    //     }

    //     struct sockaddr_in temp;
    //     socklen_t size = sizeof(temp);
    //     ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&temp), reinterpret_cast<socklen_t *>(&size));
    //     if (len < 0)
    //     {
    //         cout << " recvfrom error: " << strerror(errno) << endl;
    //         continue;
    //     }
    //     buffer[len] = '\0';
    //     cout << buffer;
    // }

    close(sockfd);


    return 0;
}