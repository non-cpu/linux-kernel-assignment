#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define MAX_PROCESSES 10000

int policies[3] = {SCHED_OTHER, SCHED_FIFO, SCHED_RR};
int priorities[3] = {99, 50, 1};
int nice_values[3] = {-20, 0, 19};

void setProcessScheduling(int policy_idx, int priority_idx, int nice_idx)
{
    struct sched_param param;
    param.sched_priority = priorities[priority_idx];
    sched_setscheduler(0, policies[policy_idx], &param);
    setpriority(PRIO_PROCESS, 0, nice_values[nice_idx]);
}

int createChildrenAndSum(int idx, int policy_idx, int priority_idx, int nice_idx)
{
    int result;

    setProcessScheduling(policy_idx, priority_idx, nice_idx);

    if (idx >= MAX_PROCESSES - 1)
    {
        int offset = idx - (MAX_PROCESSES - 1);

        char filename[20];
        sprintf(filename, "temp/%d", offset);
        FILE *f_read = fopen(filename, "r");
        fscanf(f_read, "%d", &result);
        fclose(f_read);

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
            exit(createChildrenAndSum(child_idx, policy_idx, priority_idx, nice_idx));
        }
        else
        {
            child_idx++;
            child2 = fork();

            if (child2 == -1)
            {
                perror("fork failed");
            }
            else if (child2 == 0)
            {
                exit(createChildrenAndSum(child_idx, policy_idx, priority_idx, nice_idx));
            }
            else
            {
                int status1, status2;
                waitpid(child1, &status1, 0);
                waitpid(child2, &status2, 0);
                
                return 0;
            }
        }
    }
}

int main()
{
    int result;
    struct timespec start, end;

    int policy_idx, priority_idx, nice_idx;

    for (policy_idx = 0; policy_idx < 3; policy_idx++)
    {
        for (priority_idx = 0; priority_idx < 3; priority_idx++)
        {
            for (nice_idx = 0; nice_idx < 3; nice_idx++)
            {
                system("sync");
                system("echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null");

                clock_gettime(CLOCK_MONOTONIC, &start);

                result = createChildrenAndSum(0, policy_idx, priority_idx, nice_idx);

                clock_gettime(CLOCK_MONOTONIC, &end);

                double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

                printf("policy: %d, priority: %d, nice: %d, time: %f\n", policies[policy_idx], priorities[priority_idx], nice_values[nice_idx], elapsed_time);
            }
        }
    }

    return 0;
}
