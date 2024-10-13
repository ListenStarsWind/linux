#include<stdio.h>

void f3()
{
	printf("%s\n", "=");
}

void f2()
{
	printf("%s\n", "+");
	f3();
}

void f1()
{
	printf("%s\n", "-");
	f2();
}

int cumulative(int t)
{
	int ret = 0;
	int cir = 1;
	for (; cir <= t; cir++)
	{
		ret += cir;
	}
	f1();
	return ret;
}

int main()
{
	printf("%s\n", "debug begin");
	int top = 100;
	int sum = cumulative(top);
	printf("sum:%d\n",sum);
	printf("%s\n","debug end");
	return 0;
}