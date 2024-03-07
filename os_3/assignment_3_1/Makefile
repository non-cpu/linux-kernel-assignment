CC = gcc
CFLAGS = -w -DMAX_PROCESSES=8
RM = rm -f

all: numgen fork thread

numgen: numgen.c
	$(CC) $(CFLAGS) numgen.c -o numgen
fork: fork.c
	$(CC) $(CFLAGS) fork.c -o fork
thread: thread.c
	$(CC) $(CFLAGS) -pthread thread.c -o thread
clean:
	$(RM) numgen fork thread
