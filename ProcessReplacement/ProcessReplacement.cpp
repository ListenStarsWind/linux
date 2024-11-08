#include"CentralControl.h"

#ifdef SUBSTITUTION_EXPERIMENT

#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

int main()
{
	//extern char** environ;
	//putenv("PARENT_ENV=666666");
	cout << "replacement before: " << "pid->" << getpid() << endl;
	pid_t id = fork();

	if (id == 0)
	{
		sleep(2);
		cout << "child process: pid->" << getpid() << " ppid->" << getppid();
		//execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
		//execlp("ls", "ls", "-a", "-l", NULL);

		//// 严谨地说，应该用const char* const的类型，但exev只支持char* const
		//char* const myargv[] = {"out", "-a", "-b", "-c", "-d", NULL};
		//execv("./out", myargv);

		////真正执行的是脚本的解释器->bash
		//char* const myargv[] = { "bash", "test.sh", NULL };
		//execv("/usr/bin/bash", myargv);

		/*char* const myargv[] = { "python", "test.py", NULL };
		execv("/usr/bin/python", myargv);*/

		//使用environ传递全套环境变量

		char* const env[] = {"MYVAL=11111", "MYPATH=/usr/bin/xxx", NULL};
		execle("./out", "out", "-a", "-b", "-c", "-d", NULL, env);

		perror("exec failed");
		exit(1);
	}
	else if (id > 0)
	{
		pid_t ret = waitpid(id, NULL, 0);
		if (ret > 0)
		{
			cout << "reclaim process resources:" << ret << endl;
		}
		else if (ret < 0)
		{
			perror("waitpid failed");
		}
	}
	else
	{
		perror("fork failed");
	}

	cout << "replacement after: " << "pid->" << getpid() << endl;

	sleep(2);

	return 0;
}

#endif