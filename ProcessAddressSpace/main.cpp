#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

int main()
{
	int val = 6;
	int cir = 6;
	pid_t id = fork();
	if (id == 0)
	{
		while (1)
		{
			printf("I am child. pid->%d ppid->%d\n", getpid(), getppid());
			printf("val:%d val->%p\n", val, &val);
			printf("===================================\n");
			sleep(1);
			cir--;
			if (cir == 0)
				val = 8;
		}
	}
	else
	{
		while (1)
		{
			// 不考虑错误，默认成功
			printf("I am parent. pid->%d ppid->%d\n", getpid(), getppid());
			printf("val:%d val->%p\n", val, &val);
			printf("===================================\n");
			sleep(1);
		}
	}
	return 0;
}