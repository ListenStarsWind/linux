#include"TcpServer.hpp"
#include"ServerCal.hpp"

using namespace std;

int main()
{
    calculator cal;
    tcpserver s(cal);
    s.init();
    s.run();
    return 0;
}