#include "shmm.hpp"
#include<iostream>

using namespace wind;
using namespace std;

int main()
{
    Log log;
    channel shmm(PreferExisting);
    while(1)
    {
        string buf;
        cout<<"Please enter# ";
        cin >> buf;
        shmm.write(buf.c_str());
        sleep(1);
    }
    log(Info, "b quit...");
    return 0;
}