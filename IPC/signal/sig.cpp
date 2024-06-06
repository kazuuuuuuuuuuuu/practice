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
	signal(SIGINT, myhandle);
	signal(SIGUSR2, myhandle);
	signal(SIGUSR1, myhandle);
	while(1)
	{
		sleep(1);
		printf("sleep for one second\n");
	}
	return 0;
}