#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#define MAX_PROCESSES 10000

int main()
{
    mkdir("temp", 0777);

    srand(time(NULL));
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        char filename[20];
        sprintf(filename, "temp/%d", i);
        FILE *f_write = fopen(filename, "w");
        fprintf(f_write, "%d", 1 + rand() % 9);
        fclose(f_write);
    }

    return 0;
}
