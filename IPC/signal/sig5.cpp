#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int workflag = 0;
// 信号处理函数像普通函数那样被调用 修改全局变量 只不过调用时是被动调用 而不是主动
void work_up_handle(int sig)
{
	workflag = 1;
}

void work_down_handle(int sig)
{
	workflag = 0;
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
		char *msg;

		struct sigaction act;
		
		act.sa_flags = 0;
		act.sa_handler = work_up_handle;
		sigemptyset(&act.sa_mask);
		// 替代signal 设置信号处理函数
		sigaction(SIGUSR1, &act, 0);

		act.sa_handler = work_down_handle;
		sigaction(SIGUSR2, &act, 0);

		while(1)
		{
			if(workflag)
			{
				msg = "CHILD PROCESS WORK!\n";
			}
			else
			{
				msg = "child process work!\n";
			}

			printf("%s", msg);
			sleep(1);
		}
	}
	else 
	{
		while(1)
		{
			c = getchar();
			if(c == 'A')
			{
				// 发信号给子进程
				kill(pid, SIGUSR1);
			}
			else if(c == 'a')
			{
				kill(pid, SIGUSR2);
			}
		}
	}
	return 0;
}