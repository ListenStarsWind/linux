#include <string>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <system_error>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

#define BUFFSIZE 4096

enum{
    SOCKETERRO = 1,
    CONNECTERRO = 2,
};

struct sockaddr_in args_parsing(char** argv)
{
    int i = 0;
    for(; argv[i]; ++i);
    if(i != 3)
    {
        cout << "parsing error"<<endl;
        exit(1);
    }
    sockaddr_in result;
    memset(&result, 0, sizeof(result));
    result.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &result.sin_addr);
    result.sin_port = htons(stoi(argv[2]));
    return result;
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        throw system_error(errno, system_category(), "create socket error");
    }
    return sockfd;
}

void link_to(const int& sockfd, const struct sockaddr_in& server)
{
    if(connect(sockfd, reinterpret_cast<const struct sockaddr*>(&server), static_cast<socklen_t>(sizeof(server))) != 0)
    {
        throw system_error(errno, system_category(), "connect error");
    }
}

void get_message(string& message)
{
    cout << "Please Enter# ";
    getline(cin, message);
}

void push(const int& sockfd, const string& message)
{
    ssize_t len = write(sockfd, message.c_str(), message.size());
    if(len < 0)
    {
        throw system_error(errno, system_category(), "push error");
    }
}

string pull(const int& sockfd)
{
    char buffer[BUFFSIZE];
    ssize_t len = read(sockfd, buffer, sizeof(buffer) - 1);
    if(len < 0)
    {
        throw system_error(errno, system_category(), "pull error");
    }
    buffer[len] = 0;
    string result(buffer);
    return result;
}

int comm_prep(const struct sockaddr_in &server)
{
    int sockfd = 0;
    int count = 10;
    while (count--)
    {
        try
        {
            sockfd = create_socket();
            link_to(sockfd, server);
            return sockfd;
        }
        catch (const std::system_error &e)
        {
            cerr << e.what() << endl;
        }
        cout << "Reconnecting, please wait..."<<endl;
        sleep(2);
    }
    cout << "Unable to connect, exiting..."<<endl;
    exit(0);
}

int main(int argc, char *argv[])
{
    // 客户端不由我们亲自绑定

    auto server = args_parsing(argv);

    string message;
    while (true)
    {
        int sockfd = comm_prep(server);
        get_message(message);

        try
        {
            push(sockfd, message);
            auto response = pull(sockfd);
            cout << response << endl;
        }
        catch(const std::system_error &e)
        {
            cerr << e.what() << endl;
        }

        close(sockfd);
    }
    return 0;
}
