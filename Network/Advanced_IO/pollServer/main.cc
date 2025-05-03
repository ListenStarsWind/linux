#include"poll_server.hpp"
#include<memory>

using namespace std;

int main()
{
    unique_ptr<PollServer> server(new PollServer());
    server->init();
    server->start();

    return 0;
}