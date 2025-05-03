#include "epoll_server.hpp"

#include<memory>

using namespace std;

int main()
{
    unique_ptr<EpollServer> epoll_svr(new EpollServer());
    epoll_svr->init();
    epoll_svr->statrt();
    return 0;
}