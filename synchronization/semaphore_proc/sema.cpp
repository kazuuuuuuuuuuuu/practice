#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void) 
{
	int i;
	pid_t pid = fork();
	for (i=0; i<5; i++) {
		
		/* 模拟临界区----begin */
		printf("Process(%d) In\n", getpid());		
		sleep(1);
		printf("Process(%d) Out\n", getpid());
        /* 模拟临界区----end */ 
  
		sleep(1);
	}

	return 0;
}