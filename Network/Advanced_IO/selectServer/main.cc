#include"select_server.hpp"
#include<memory>

using namespace std;

int main()
{
    unique_ptr<SelectServer> server(new SelectServer());
    server->init();
    server->start();

    return 0;
}