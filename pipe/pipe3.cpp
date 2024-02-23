#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// pipe using in the child and parent process
int main(int argc, char const *argv[])
{
	int fd[2];
	int ret;
	char buff1[1024];
	char buff2[1024];
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
		// read
		close(fd[1]);
		memset(buff2, 0,  1024);
		read(fd[0], buff2, sizeof(buff2));
		printf("process %d received info: %s[%p]\n", getpid(), buff2, buff2);
	
		sleep(5);

		// read again
		printf("process %d read again\n", getpid());
		
		memset(buff2, 0,  1024);
		int len = 0;
		// 关闭所有写端 包括父子进程 读操作就不会阻塞 直接返回0
		len = read(fd[0], buff2, sizeof(buff2));

		printf("process %d received info: %s[%p], len : %d\n", getpid(), buff2, buff2, len);
		exit(0);
	}
	else
	{
		strcpy(buff1, "hello! child!");
		write(fd[1], buff1, strlen(buff1)+1);
		printf("process %d sent info: %s\n", getpid(), buff1);
		close(fd[1]);
	}

	if(pid>0)
	{
		//int status;
		//waitpid(pid, &status, 0);
		wait(NULL);
	}
	return 0;
}