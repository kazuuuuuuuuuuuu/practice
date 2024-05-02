#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
	int pipe_fd[2];
	char *message = "hello pipe!\n";
	char buffer[100];

	if(pipe(pipe_fd)<0)
	{
		fprintf(stderr, "pipe failed\n");
		exit(1);
	}
	// 文件描述符fd被复制了一份到子进程
	if(fork()==0)
	{
		sleep(3);
		// 同步问题由系统解决
		write(pipe_fd[1], message, strlen(message)+1);
		close(pipe_fd[1]);
	}
	else
	{
		printf("parent: prepare to read\n");

		read(pipe_fd[0], buffer, sizeof(buffer));
		close(pipe_fd[0]);
		printf("parent: received message: %s", buffer);
		wait(NULL);
	}
	return 0;
}