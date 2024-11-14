#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>

typedef const char* message;
typedef int file_descriptor;
typedef file_descriptor fd;

#define FILENAME "logbook.txt"
#define GREETING "hello linux\n"
#define PERMISSION 0666

void function4()
{
	fprintf(stdout, "out\n");
	fprintf(stdout, "out\n");
	fprintf(stdout, "out\n");
	fprintf(stdout, "out\n");

	fprintf(stderr, "error\n");
	fprintf(stderr, "error\n");
	fprintf(stderr, "error\n");
	fprintf(stderr, "error\n");

}

void function3()
{
	fd handle = open(FILENAME, O_RDONLY);
	if (handle < 0)
	{
		perror("failed open");
		return;
	}

	dup2(handle, 0);

	char buffer[1024];
	ssize_t sz = read(0, buffer, sizeof(buffer) - 1);
	if (sz < 0)
	{
		perror("failed read");
		return;
	}
	buffer[sz] = '\0';

	printf(buffer);

	close(handle);
}

void function2()
{
	fd handle = open(FILENAME, O_CREAT | O_WRONLY | O_TRUNC, PERMISSION);
	if (handle < 0)
	{
		perror("failed open");
		return;
	}

	dup2(handle, 1);

	int count = 5;
	message sentence = GREETING;
	while (count--)
	{
		//write(1, sentence, strlen(sentence));
		printf("%s", sentence);
	}

	close(handle);
}

void function1()
{
	close(1);

	fd handle = open(FILENAME, O_CREAT| O_WRONLY| O_TRUNC, PERMISSION);
	if (handle < 0)
	{
		perror("failed open");
		return ;
	}

	//fprintf(stdout, "fd->%d\n", handle);
	
	int count = 5;
	message sentence = GREETING;
	while (count--)
	{
		write(1, sentence, strlen(sentence));
	}

	close(handle);
}

class function
{
	typedef void (*operation)();
public:
	void operator()() { _action(); }
private:
	operation _action = function4;
};

int main()
{
	function process;
	process();
	return 0;
}