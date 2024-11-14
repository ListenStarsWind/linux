#include <stdio.h>			//fopen
#include <unistd.h>			//chdir
#include <sys/types.h>		//getpid
#include <sys/stat.h>       //open
#include <fcntl.h>          //open
#include <string.h>			//strlen
#include <sys/stat.h>       //umask

int main()
{
	char buffer[256] = {0};
	ssize_t sz = read(0, buffer, sizeof(buffer) - 1);
	if (sz < 0)
	{
		perror("failed read");
		return 1;
	}
	buffer[sz] = '\0';

	write(1, buffer, strlen(buffer));

	return 0;
}

//int main()
//{
//	const char* message = "hello linux\n";
//	write(stdout->_fileno, message, strlen(message));
//	write(stderr->_fileno, message, strlen(message));
//	return 0;
//}

//int main()
//{
//	int fd1 = open("logbook.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
//	int fd2 = open("logbook.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
//	int fd3 = open("logbook.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
//	int fd4 = open("logbook.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
//	printf("fd1-> %d\n", fd1);
//	printf("fd2-> %d\n", fd2);
//	printf("fd3-> %d\n", fd3);
//	printf("fd4-> %d\n", fd4);
//
//	/*if (fd < 0)
//	{
//		perror("failed open");
//		return fd;
//	}
//
//	const char* memssage = "abcde\n";
//	write(fd, memssage, strlen(memssage));
//
//	close(fd);*/
//	return 0;
//}


//// 比特位级别传参
//#define ONE (1<<0)   //1
//#define TWO (1<<0)	 //2
//#define THREE (1<<0) //4
//#define FOUR (1<<0)  //8
//
//
//void show(int flags)
//{
//	if (flags & ONE) printf("function 1\n");
//	if (flags & TWO) printf("function 2\n");
//	if (flags & THREE) printf("function 3\n");
//	if (flags & FOUR) printf("function 4\n");
//}
//
//
//int main()
//{
//	printf("\nONE->\n");
//	show(ONE);
//
//	printf("\nTWO->\n");
//	show(TWO);
//
//	printf("\nONE|TWO->\n");
//	show(ONE|TWO);
//
//	printf("\nONE|TWO|THREE->\n");
//	show(ONE|TWO|THREE);
//
//	printf("\nONE|THREE->\n");
//	show(ONE|THREE);
//
//	printf("\nTHREE|FOUR->\n");
//	show(THREE|FOUR);
//
//	return 0;
//}


// 回顾C接口
//int main()
//{
//	/*pid_t id = getpid();
//	printf("pid-> %d\n", id);*/
//
//	FILE* pf = fopen("logbook.txt", "a");
//	if (pf == NULL)
//	{
//		perror("failed fopen");
//		return 1;
//	}
//
//	//const char* message = "abcd";
//	const char* message = "hello C I/O Interfaces";
//	//fwrite(message, strlen(message) + 1, sizeof(char), pf);
//	fprintf(stderr, "%s %d\n", message, 1024);
//
//	fclose(pf);
//
//	return 0;
//}