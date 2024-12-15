#include "shmm.hpp"
#include<iostream>

using namespace wind;
using namespace std;

int main()
{
    Log log;
    channel shmm(ForceNew);
    // sleep(3);
    // while(1)
    // {
    //     cout<<"Message received# "<<shmm.read()<<endl;
    //     sleep(1);
    // }

    struct shmid_ds shmds = shmm.getStatu();
    cout<<"shm size:"<<shmds.shm_segsz<<endl;
    cout<<"shm nattch:"<<shmds.shm_nattch<<endl;
    printf("shm key:%x\n", shmds.shm_perm.__key);
    cout<<"shm mode:"<<shmds.shm_perm.mode<<endl;

    log(Info, "a quit...");
    return 0;
}