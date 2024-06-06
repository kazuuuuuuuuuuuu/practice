#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

void myhandle(int sig)
{
	printf("catch a signal: %d\n", sig);
	for(int i=0;i<10;i++)
	{
		sleep(1);
	}
	printf("catch end: %d\n", sig);
}

int main(int argc, char const *argv[])
{
	struct sigaction act, act2;

	act.sa_handler = myhandle;
	sigemptyset(&act.sa_mask);
	// 加入屏蔽集的信号 排队 串行执行
	// 否则将中断 先处理新的信号
	sigaddset(&act.sa_mask, SIGUSR1);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);

	act2.sa_handler = myhandle;
	sigemptyset(&act2.sa_mask);
	act2.sa_flags = 0;
	sigaction(SIGUSR1, &act2, 0);

	while(1)
	{

	}
	return 0;
}