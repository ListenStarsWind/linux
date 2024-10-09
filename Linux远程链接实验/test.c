#include<stdio.h>
#include<unistd.h>

int main()
{
    int i = 10;
    while (i >= 0)
    {
        printf("%-2d\r", i);
        fflush(stdout);
        sleep(1);
        i--;
    }
    printf("\n");
    return 0;
}