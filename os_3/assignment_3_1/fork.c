#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#ifndef MAX_PROCESSES
#define MAX_PROCESSES 8
#endif

#define FILENAME "temp.txt"

int createChildrenAndSum(int idx)
{
    int result;

    if (idx >= MAX_PROCESSES - 1)
    {
        int value1, value2;
        int offset = (idx - (MAX_PROCESSES - 1)) * 2;

        FILE *f_read = fopen(FILENAME, "r");
        for (int i = 0; i < offset; i++)
        {
            fscanf(f_read, "%d", &value1);
        }

        fscanf(f_read, "%d", &value1);
        fscanf(f_read, "%d", &value2);

        fclose(f_read);

        result = value1 + value2;

        exit(result);
    }
    else
    {
        pid_t child1, child2;
        int child_idx = idx * 2;

        child_idx++;
        child1 = fork();

        if (child1 == -1)
        {
            perror("fork failed");
        }
        else if (child1 == 0)
        {
            exit(createChildrenAndSum(child_idx));
        }
        else
        {
            int status1, status2;
            waitpid(child1, &status1, 0);

            child_idx++;
            child2 = fork();

            if (child2 == -1)
            {
                perror("fork failed");
            }
            else if (child2 == 0)
            {
                exit(createChildrenAndSum(child_idx));
            }
            else
            {
                waitpid(child2, &status2, 0);

                status1 = status1 >> 8;
                status2 = status2 >> 8;

                FILE *f_write = fopen(FILENAME, "a");
                fprintf(f_write, "%d\n", status1);
                fprintf(f_write, "%d\n", status2);

                result = status1 + status2;

                if (idx == 0)
                    fprintf(f_write, "%d\n", result);

                fclose(f_write);

                return result;
            }
        }
    }
}

int main()
{
    int result;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    result = createChildrenAndSum(0);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("value of fork : %d\n", result);
    printf("%f\n", elapsed_time);

    return 0;
}
