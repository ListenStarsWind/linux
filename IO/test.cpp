#include"windio.h"

int main()
{
	const char* str = "hello fwrite\n";
	FILE* pf = fopen("log.txt", "a");
	int cunt = 5;
	while (cunt--)
	{
		fwrite(str, strlen(str), 1, pf);
		sleep(1);
	}
	fclose(pf);
}