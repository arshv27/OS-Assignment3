#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

char buf[8192];
char name[3];
char *echoargv[] = { "echo", "ALL", "TESTS", "PASSED", 0 };
int stdout = 1;
#define TOTAL_MEMORY (1 << 20) + (1 << 18)

void
mem(void)
{
	void *m1 = 0, *m2, *start;
	uint cur = 0;
	uint count = 0;
	uint total_count;
	int pid;

	// printf(1, "mem test\n");

	m1 = malloc(4096);
	if (m1 == 0)
		goto failed;
	start = m1;

	while (cur < TOTAL_MEMORY) {
		m2 = malloc(4096);
		if (m2 == 0)
			goto failed;
		*(char**)m1 = m2;
		((int*)m1)[2] = count++;
		// printf(1,"M1: %d, count: %d\n", m1, count);
		m1 = m2;
		cur += 4096;
	}
	// printf(1,"out of while loop");
	((int*)m1)[2] = count;
	total_count = count;

	count = 0;
	m1 = start;

	while (count != total_count) {
		// printf(1,"count: %d\n", count);
		if (((int*)m1)[2] != count)
			goto failed;
		m1 = *(char**)m1;
		count++;
	}
	// printf(1,"out of 2nd while loop");
	if (swap(start) != 0)
		printf(1, "failed to swap %p\n", start);
	// printf(1,"Forking\n");
	pid = fork();
	// printf(1,"PID: %d\n", pid);
	// printf(1,"Done sForking\n");
	if (pid == 0){
		count = 0;
		m1 = start;
	
		while (count != total_count) {
			// printf(1,"M1: %d, count: %d\n", m1, count);
			if (((int*)m1)[2] != count)
				goto failed;
			m1 = *(char**)m1;
			count++;
		}
		// printf(1,"Out of while loop\n");
		exit();
	}
	else if (pid < 0)
	{
		printf(1, "fork failed\n");
	}
	else if (pid > 0)
	{
		// printf(1, "Parents PID = %d\n", pid);
		wait();
	}

	printf(1, "mem ok %d\n", bstat());
	exit();
failed:
	printf(1, "test failed!\n");
	exit();
}

int
main(int argc, char *argv[])
{
	printf(1, "memtest starting\n");
	mem();
	return 0;
}
