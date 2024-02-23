#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
// the pipe using in the same process
int main(int argc, char const *argv[])
{
	int fd[2];
	int ret;
	char buff1[1024];
	char buff2[1024];

	ret = pipe(fd);
	if(ret<0)
	{
		fprintf(stderr, "pipe failed\n");
		exit(1);
	}

	strcpy(buff1, "Hello!");
	write(fd[1], buff1, strlen(buff1)+1);
	printf("send info: %s\n", buff1);

	memset(buff2, 0, 1024);
	read(fd[0], buff2, sizeof(buff2));
	printf("received info: %s\n", buff2);
	return 0;
}