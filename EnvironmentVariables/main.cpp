#include<iostream>
#include<unistd.h>

int main()
{
	extern char** environ;
	int i = 0;
	for (; environ[i]; i++)
	{
		std::cout << environ[i] << std::endl;
	}
	return 0;
}