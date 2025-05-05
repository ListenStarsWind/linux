#include "epoll_server.hpp"

#include<memory>

using namespace std;

int main()
{
    unique_ptr<EpollServer> epoll_svr(new EpollServer());
    epoll_svr->epoll_server_init_();
    epoll_svr->epoll_server_statrt();
    return 0;
}