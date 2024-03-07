#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef MAX_PROCESSES
#define MAX_PROCESSES 8
#endif

int main()
{
    FILE *f_write = fopen("temp.txt", "w");

    int i;
    pid_t pid;

    for (i = 0; i < MAX_PROCESSES * 2; i++)
    {
        pid = fork();

        if (pid == -1)
        {
            printf("can't, fork\n");
            exit(0);
        }
        else if (pid == 0)
        {
            fprintf(f_write, "%d\n", i + 1);
            exit(0);
        }

        wait(NULL);
    }

    fclose(f_write);

    return 0;
}