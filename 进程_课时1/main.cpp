#include<iostream>
#include <unistd.h>

int main()
{
	int cir = 0;
	while (1)
	{
		std::cout << "It's a process." << cir << std::endl;
		++cir;
		sleep(1);
	}
	return 0;
}