#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<string.h>

void task1()
{
 //之后学习进程通信之后
 //可以让子进程报告进度
 //从而显示进度
}

void task2()
{
  //学了日志系统之后
  //可以顺便写写日志
}

void task3()
{
  //打印网络信息
}

void other()
{
  printf("Other operations\n");
}
// 总之，只是在等子进程的时候
// 顺手做点任务而已
// 所以，一般都是小任务
// 复杂度不高的
// 是一些没有子进程重要
// 的小任务

//对于C语言来说
//使用函数指针的方式对它们进行统一管理

//以宏的方式，定义函数指针元素个数
#define TASK_NUM 10

//对函数指针类型进行类型重命名
typedef void (*task)();

//定义一个函数指针数组
task task_arry[TASK_NUM];

//初始化
void Init_task()
{
  int i = 0;
  for(; i < TASK_NUM ; i++)
  {
    task_arry[i] = NULL;
  }
  return;
}

//添加任务
int push_task(task t)
{
 //寻找数组中空的位置
 int pos = 0;
 for(; pos < TASK_NUM; pos++)
 {
   //如果内容不为空，就跳过
  if(task_arry[pos]) continue;
  else break;
 } 
 if(pos == TASK_NUM)
   return -1;
 task_arry[pos] = t;
 return 0;
}

//执行任务
void execute_task()
{
 int pos = 0;
 for(pos =0; pos < TASK_NUM; pos++)
 {
   if(task_arry[pos]) task_arry[pos]();
 }
}


int main()
{
  printf("begin\n");
  printf("parent process ID->%d\n", getpid()); 
  int count = 5;
  while(count--)
  {
    pid_t id = fork();

    if(id == 0)
    {
      // child
      int i = 10;
      while(i--)
      {
        printf("child process: pid->%d i->%d\n", getpid(), i);
        sleep(1);
      }
      exit(0);
    }
    else if(id > 0)
    {
      printf("Successfully created, child process PID->%d\n", id);
    }
    else 
    {
      perror("failed");
    }
    sleep(1);
  }

  //sleep(4);
  printf("wait begin\n");
  Init_task();
  push_task(other);
  push_task(task1);
  push_task(task2);
  push_task(task3);
  count = 5;
  while(count)
  {

    int status = 0;
    pid_t ret = waitpid(-1, &status, WNOHANG);

    if(ret > 0)
    {
      count--;
      printf("Reclaim process resources:%d\n", ret);
      if(WIFEXITED(status))
      {
        printf("Normal exit, code : %d\n", WEXITSTATUS(status));
      }
      else 
      {
        printf("Exception Interrupt, code : %d\n", status & 0x7f);
      }

    }
    else if(ret < 0)
    {
      perror("failed wait");
    }
    else 
    {
     execute_task(); 
    }
    sleep(1);
  }
  printf("end\n");
  return 0;
}

//int main()
//{
//	pid_t id = fork();
//	if (id < 0)
//	{
//		perror("fork failure:");
//	}
//	else if (id == 0)
//	{
//		int count = 3;
//		while (count--)
//		{
//			printf("child process: pid->%d ppid->%d count:%d------\n", getpid(), getppid(), count);
//			sleep(1);
//		}
//    sleep(10);
//		exit(0);
//	}
//	else
//	{
//		int count = 5;
//		while (count--)
//		{
//			printf("parent process: pid->%d child pid->%d count:%d\n", getpid(), id, count);
//			sleep(1);
//		}
// 
//    sleep(10);
//    printf("waitpid begin\n");
//    int status = 0;
//    pid_t ret = waitpid(id, &status, 0);
//    printf("wait->%d\n",ret);
//    if(WIFEXITED(status))
//    {
//      printf("Normal exit, code : %d\n", WEXITSTATUS(status));
//    }
//    else 
//    {
//      printf("Exception Interrupt, code : %d\n", status & 0x7f);
//    }
//    //printf("Error message : %d\n", status & 0x7f);
//    //printf("Exit code : %d\n", WEXITSTATUS(status));
//    printf("waitpid end\n");
//    sleep(3);
//	}
//	return 0;
//}
