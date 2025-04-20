//Privilege denial at the system level
//系统层面的权限拦截

#include<stdio.h>

int main()
{
  char* str = "hello world";
  printf("%p\n",str);
  *str = 'H';
  return 0;
}
