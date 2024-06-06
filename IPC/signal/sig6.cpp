#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int wakeflag = 0;

// 信号处理函数的原型是有要求的
void wake_handler(int sig)
{
	wakeflag = 1;
	printf("wake_handler is working\n");
}

int main(int argc, char const *argv[])
{
	pid_t pid;
	char c;

	pid = fork();

	if(pid<0)
	{
		fprintf(stderr, "fork failed\n");
		exit(1);
	}
	else if(pid==0)
	{
		while(1)
		{
			sleep(5);
			// 发闹钟信号
			kill(getppid(), SIGALRM);			
		}
	}
	else
	{
		struct sigaction act;
		act.sa_flags = 0;
		act.sa_handler = wake_handler;
		sigemptyset(&act.sa_mask);
		// 替代signal 设置信号处理函数
		sigaction(SIGALRM, &act, 0);

		while(1)
		{
			pause(); // suspend until receiving any signal		

			if(wakeflag)
				printf("begin to work\n");			
		}
	}
	return 0;
}