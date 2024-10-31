#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

void f()
{
	// 检查发现
	// 前置事项完不成
	// 那就不用去做
	printf("Impossible");
	_exit(127);
}

int main()
{
	f();
	return 0;
}