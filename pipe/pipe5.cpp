#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

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

	pid = fork();
	if(pid<0)
	{
		fprintf(stderr, "fork failed\n");
		exit(1);
	}
	else if(pid==0)
	{
		close(fd[1]);
		// reading from 0 equals to reading from fd[0]
		// 将标准输入重定向到管道读端
		dup2(fd[0], 0);
		close(fd[0]);
		
		char *argv[10];
		char argv1[10];
		strcpy(argv1, "./od");
		argv[0] = argv1;
		argv[1] = NULL;

		if(execve(argv[0], argv, environ)<0)
		{
			fprintf(stderr, "execve failed: %s", strerror(errno));
			exit(1);
		}
	}
	else
	{
		close(fd[0]);

		strcpy(buff1, "hello!");
		write(fd[1], buff1, strlen(buff1)+1);
		close(fd[1]);
	}
	if(pid>0)
	{
		wait(NULL);
	}
	return 0;
}