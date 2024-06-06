#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void myhandle(int sig)
{
	printf("catch a  signal: %d\n", sig);
}

int main(int argc, char const *argv[])
{
	struct sigaction newact;
	struct sigaction oldact;

	newact.sa_handler = myhandle;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0 | SA_RESETHAND;
	sigaction(SIGINT, &newact, &oldact);

	while(1)
	{
		sleep(1);
		printf("sleep 1 second.\n");
	}
	return 0;
}