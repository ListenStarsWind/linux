#include"tcpserver.hpp"
#include<iostream>

#include<fstream>

using namespace std;

void print_help() {
    cout << "Usage: ./tcpserver -<port_number>" << endl;
    cout << "Options:" << endl;
    cout << "  -<port_number>    Specify the port number (1-65535)" << endl;
    cout << "Example:" << endl;
    cout << "  ./tcpserver -8888" << endl;
}

// ! 存在平台差异, Windows不适用
int16_t get_port(char **argv)
{
    int i = 0;
    for (; argv[i]; ++i);

    if (i != 2)
    {
        print_help();
        exit(wind::PARAMERROR);
    }

    int temp = stoi(argv[1] + 1);
    if (temp < 1 || temp > 65535)
    {
        print_help();
        exit(wind::PARAMERROR);
    }

    return static_cast<int16_t>(temp);
}

int main(int argc, char* argv[])
{
    // 如果加在最前面, 就不会有问题
    daemon();
    wind::tcpserver tcp_svr(get_port(argv));
    tcp_svr.init();
    tcp_svr.run();
    return 0;
}