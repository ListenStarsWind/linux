#include<iostream>
#include<unistd.h>

int main()
{
	while (1)
	{
		std::cout << "process->  PID:" << getpid() << std::endl;
		sleep(1);
	}
	return 0;
}