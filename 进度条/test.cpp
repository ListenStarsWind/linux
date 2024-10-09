#include"ProgressBar.h"


int f()
{
	static int i = -1;
	++i;
	return i;
}

void ProgressBarTest1()
{
	wind::ProgressBar i;
	i.print(f);
}

int main()
{
	ProgressBarTest1();
	return 0;
}