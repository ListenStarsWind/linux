#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

void f()
{
	// ��鷢��
	// ǰ�������겻��
	// �ǾͲ���ȥ��
	printf("Impossible");
	_exit(127);
}

int main()
{
	f();
	return 0;
}