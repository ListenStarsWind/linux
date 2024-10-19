#include<iostream>
#include<unistd.h>
#include<stdlib.h>

int main()
{
	pid_t id = fork();
	if (id == 0)
	{
		while (1)
		{
			std::cout << "child-> pid::" << getpid() << "  ppid:" << getppid() << std::endl;
			sleep(1);
		}
	}
	else
	{
		int cir = 8;
		while (cir--)
		{
			std::cout << "parent-> pid::" << getpid() << "  ppid:" << getppid() << std::endl;
			sleep(1);
		}
	}
	return 0;
}