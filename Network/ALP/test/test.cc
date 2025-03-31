#include"Socket.hpp"

int main()
{
    wind::Socket_Server s("8888");
    s.init();
    s.run();
    return 0;
}