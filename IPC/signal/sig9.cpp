#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

void myhandler(int sig)
{
	printf("catch a signal: %d\n", sig);
	printf("catch end: %d\n", sig);
}

int main(int argc, char const *argv[])
{
	struct sigaction act, act2;

	act.sa_handler = myhandler;
	// 控制信号处理函数期间信号的阻塞
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);

	sigset_t proc_sig_mask, old_mask;
	sigemptyset(&proc_sig_mask);
	sigaddset(&proc_sig_mask, SIGINT);

	// 控制进程信号的阻塞
	sigprocmask(SIG_BLOCK, &proc_sig_mask, &old_mask);
	sleep(5);
	printf("has deleted SIGINT from process sig mask\n");
	sigprocmask(SIG_UNBLOCK, &proc_sig_mask, &old_mask);

	while(1)
	{

	}
	return 0;
}