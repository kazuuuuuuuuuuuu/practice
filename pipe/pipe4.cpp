#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

// pipe using with the fork() and evecve() functions
#define MAXARGS 128

int main(int argc, char const *argv[])
{
	int fd[2];
	int ret;
	char buff1[1024];
	pid_t pid;

	ret = pipe(fd);
	if(ret<0)
	{
		fprintf(stderr, "pipe failed\n");
		exit(1);
	}

	// 当fork的时候 管道的端口被复制了一份
	pid = fork();
	if(pid<0)
	{
		fprintf(stderr, "fork failed\n");
		exit(1);		
	}
	else if(pid==0)
	{
		close(fd[1]); // 关闭子进程的写端
		char *argv[MAXARGS];
		char argv1[128];
		char argv2[128];

		argv[0] = argv1;
		argv[1] = argv2;
		argv[2] = NULL;
		strcpy(argv1, "./main");
		sprintf(argv2, "%d", fd[0]);

		if (execve(argv[0], argv, environ) == -1) 
		{
			perror("execve failed");
			exit(1);
		}
	}
	else
	{
		close(fd[0]); // 关闭父进程的读端
		strcpy(buff1, "hello! pipe!");
		write(fd[1], buff1, strlen(buff1)+1);
		printf("process %d send info: %s\n", getpid(), buff1);
	}

	if(pid>0)
	{
		wait(NULL);
	}
	return 0;
}