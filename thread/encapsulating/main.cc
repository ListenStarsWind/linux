#include "thread.hpp"
#include <iostream>
#include <unistd.h>

using namespace std;

void f()
{
    while (true)
    {
        cout << "shabc" << endl;
        sleep(1);
    }
}

int main()
{
    thread t(f);
    t.run();
    while (true);
    return 0;
}