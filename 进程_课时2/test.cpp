#include<iostream>    
#include<unistd.h>    
    
int main()    
{    
    std::cout << "I am a process, my PID is " << getpid() << "," << "PPID is " << getppid() << "." << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    pid_t id = fork();
    if (id == 0)
    {
        while (1)
        {
            std::cout << "I am a child process, my PID is " << getpid() << "," << "PPID is " << getppid() << "." << std::endl;
            std::cout << "=======================================================" << std::endl;
            sleep(1);
        }
    }
    else if (id > 0)
    {
        while (1)
        {
            std::cout << "I am a parent process, my PID is " << getpid() << "," <<"PPID is "<<getppid()<<"." <<std::endl;
            std::cout << "=======================================================" << std::endl;
            sleep(1);
        }
    }
    else
    {
        std::cout<<"Error!" << std::endl;
    }
  return 0;    
}