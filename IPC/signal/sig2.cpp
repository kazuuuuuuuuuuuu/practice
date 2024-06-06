#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void myhandle(int sig)
{
	printf("catch a sig: %d\n", sig);
}

int main(int argc, char const *argv[])
{
	// set signal handler
	signal(SIGCHLD, myhandle);

	pid_t pid;
	pid = fork();
	if(pid<0)
	{
		printf("error in fork()\n");
	}
	else if(pid==0) // child process
	{
		exit(1);
	}
	else
	{
		printf("pid: %d, ", getpid());
		printf("(i am the parent)\n");
	}

	while(1)
	{
		sleep(1);
		
		printf("sleep for one second\n");
	}
	return 0;
}