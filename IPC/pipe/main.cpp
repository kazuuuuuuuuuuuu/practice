#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	int fd;
	char buff[1024];

	// scanf reads from stdin
	// sscanf reads from a specified string
	sscanf(argv[1], "%d", &fd);
	read(fd, buff, sizeof(buff));
	printf("child process: \n");
	printf("process %d received info: %s\n", getpid(), buff);
	return 0;
}