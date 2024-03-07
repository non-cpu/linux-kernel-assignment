#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/mman.h>

uint8_t *Operation;
uint8_t *compiled_code;

void sharedmem_init();
void sharedmem_exit();
void drecompile_init();
void drecompile_exit();
void *drecompile(uint8_t *func);
void measure_execution_time(int (*func)(int a));

int main(void)
{
	int (*func)(int a);
	int i;

	sharedmem_init();
	drecompile_init();

	func = (int (*)(int a))drecompile(Operation);

	measure_execution_time(func);

	drecompile_exit();
	sharedmem_exit();

	return 0;
}

void sharedmem_init()
{
	int segment_id = shmget(1234, PAGE_SIZE, IPC_CREAT | S_IRUSR | S_IWUSR);
	Operation = (uint8_t *)shmat(segment_id, NULL, 0);
}

void sharedmem_exit()
{
	shmdt(Operation);
}

void drecompile_init(uint8_t *func)
{
	compiled_code = (uint8_t *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void drecompile_exit()
{
	munmap(compiled_code, PAGE_SIZE);
}

void *drecompile(uint8_t *func)
{
	memcpy(compiled_code, func, PAGE_SIZE);

#ifdef dynamic
	optimize();
#endif

	mprotect(compiled_code, PAGE_SIZE, PROT_READ | PROT_EXEC);

	return compiled_code;
}

void optimize()
{
	int idx_s = 0;
	int idx_t = 0;
	int data = NULL;
	int prev_operation = NULL;

	uint8_t *new_compiled_code = (uint8_t *)malloc(PAGE_SIZE);

	while (idx_s < PAGE_SIZE)
	{
		switch (compiled_code[idx_s])
		{
		case 0x83:
			if (compiled_code[idx_s + 1] == 0xC0 || compiled_code[idx_s + 1] == 0xE8)
			{
				if (compiled_code[idx_s + 1] == prev_operation)
				{
					data += compiled_code[idx_s + 2];
				}
				else
				{
					if (prev_operation != NULL)
					{
						new_compiled_code[idx_t++] = data & 0xFF;
					}

					new_compiled_code[idx_t++] = compiled_code[idx_s];
					new_compiled_code[idx_t++] = compiled_code[idx_s + 1];
					data = compiled_code[idx_s + 2];

					prev_operation = compiled_code[idx_s + 1];
				}

				idx_s += 3;

				break;
			}

		case 0x6B:
			if (compiled_code[idx_s + 1] == 0xC0)
			{
				if (compiled_code[idx_s] == prev_operation)
				{
					data *= compiled_code[idx_s + 2];
				}
				else
				{
					if (prev_operation != NULL)
					{
						new_compiled_code[idx_t++] = data & 0xFF;
					}

					new_compiled_code[idx_t++] = compiled_code[idx_s];
					new_compiled_code[idx_t++] = compiled_code[idx_s + 1];
					data = compiled_code[idx_s + 2];

					prev_operation = compiled_code[idx_s];
				}

				idx_s += 3;

				break;
			}

		default:
			if (prev_operation != NULL)
			{
				new_compiled_code[idx_t++] = data & 0xFF;
				prev_operation = NULL;
			}

			new_compiled_code[idx_t++] = compiled_code[idx_s++];
			prev_operation = NULL;
		}
	}

	memcpy(compiled_code, new_compiled_code, PAGE_SIZE);

	free(new_compiled_code);
}

void measure_execution_time(int (*func)(int a))
{
	int result;

	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (int i = 0; i < 10000; i++)
	{
		result = func(1);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

	printf("total execution result: %d, time: %.9f sec\n", result, time);
}
