#include<stdio.h>	  // IO_c_interface
#include<string.h>	  // strlen
#include<unistd.h>	  // IO_system_interface
#include <sys/wait.h> // waitpid

//int main()
//{
//	const char* c_str = "hello fwrite\n";
//	const char* s_str = "hello write\n";
//
//	printf("hello printf\n");
//	fflush(stdout);
//	sleep(3);
//
//	fprintf(stdout, "hello fprintf\n");
//	fwrite(c_str, strlen(c_str), 1, stdout);
//	sleep(3);
//
//	write(1, s_str, strlen(s_str));
//	sleep(5);
//
//	pid_t id =fork();
//	if (id > 0)
//	{
//		waitpid(id, NULL, 0);
//		sleep(4);
//	}
//
//	return 0;
//}