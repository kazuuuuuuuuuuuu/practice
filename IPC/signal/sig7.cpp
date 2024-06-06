#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>


int wakeflag = 0;

// 信号处理函数的原型是有要求的
void wake_handler(int sig)
{
	wakeflag = 1;
}

int main(int argc, char const *argv[])
{
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = wake_handler;
	sigemptyset(&act.sa_mask);
	// 替代signal 设置信号处理函数
	sigaction(SIGALRM, &act, 0);

	int ret;
	// 设置闹钟 给自己发闹钟信号
	ret = alarm(5);
	printf("begin, time = %ld\n", time((time_t*)0));
	if(ret<0)
	{
		fprintf(stderr, "alarm failed\n");
		exit(1);
	}
	while(1)
	{
		pause(); // suspend until receiving any signal		

		if(wakeflag)
			printf("waker up, time = %ld\n", time((time_t*)0));			

		ret = alarm(5);
		if(ret<0)
		{
			fprintf(stderr, "alarm failed\n");
			exit(1);
		}		
	}
	
	return 0;
}