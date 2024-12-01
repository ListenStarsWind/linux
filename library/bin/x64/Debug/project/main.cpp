#include"windmath.h"
#include"windlog.h"
#include"windprint.h"
#include<stdio.h>

int main()
{
  int n = div(10, 0);
  if(winderrno == 0)
    printf("10/0=%d\n", n);
  else
    printf("failed div, errno:%d\n",winderrno);

  print();
  Log("A system is never complete.");
  return 0;
}
