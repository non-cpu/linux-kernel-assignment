#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#ifndef MAX_PROCESSES
#define MAX_PROCESSES 8
#endif

#define FILENAME "temp.txt"

void *threadFunction(void *arg)
{
    int result;

    int idx = *(int *)arg;

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

        return (void *)(size_t)result;
    }
    else
    {
        pthread_t thread1, thread2;
        int child_idx = idx * 2;

        void *temp;
        int result1, result2;

        child_idx++;
        pthread_create(&thread1, NULL, threadFunction, &child_idx);

        pthread_join(thread1, &temp);
        result1 = (int)(size_t)temp;

        child_idx++;
        pthread_create(&thread2, NULL, threadFunction, &child_idx);

        pthread_join(thread2, &temp);
        result2 = (int)(size_t)temp;

        FILE *f_write = fopen(FILENAME, "a");
        fprintf(f_write, "%d\n", result1);
        fprintf(f_write, "%d\n", result2);

        result = result1 + result2;

        if (idx == 0)
            fprintf(f_write, "%d\n", result);

        fclose(f_write);

        return (void *)(size_t)result;
    }
}

int main()
{
    int result;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int root_idx = 0;
    pthread_t root_thread;
    pthread_create(&root_thread, NULL, threadFunction, &root_idx);

    void *temp;
    pthread_join(root_thread, &temp);
    result = (int)(size_t)temp;

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("value of thread: %d\n", result);
    printf("%f\n", elapsed_time);

    return 0;
}
