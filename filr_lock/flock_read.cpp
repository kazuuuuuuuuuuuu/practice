#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FILE_NAME "locktest.txt"

int flock_set(int fd, int type)
{
	printf("flock_set is running, pid = %d\n", getpid());

	struct flock flock;
	memset(&flock, 0, sizeof(struct flock));

	// 1 get flock
	fcntl(fd, F_GETLK, &flock);
	printf("F_RDLCK: %d\n", F_RDLCK);
	printf("F_WRLCK: %d\n", F_WRLCK);
	printf("F_UNLCK: %d\n", F_UNLCK);	
	printf("Lock type for now: %d\n", flock.l_type);
	printf("l_pid for now: %d\n", flock.l_pid);

	// 2 set flock
	flock.l_type = type;
	flock.l_whence = SEEK_SET;
	flock.l_start = 0;
	flock.l_len = 0;   
	flock.l_pid = -1;
	if(fcntl(fd, F_SETLKW, &flock)<0)
	{
		printf("set flock failed\n");
		return -1;
	}

	switch(flock.l_type)
	{
	case F_RDLCK:
		printf("now read lock is set by %d\n", getpid());
		break;
	case F_WRLCK:
		printf("now write lock is set by %d\n", getpid());
		break;
	case F_UNLCK:
		printf("now lock is released by %d\n", getpid());
		break;
	}
	return 0;
}

int main(int argc, char const *argv[])
{
	int fd = open(FILE_NAME, O_RDWR|O_CREAT, 0666);
	if(fd<0)
	{
		printf("open failed\n");
		exit(-1);
	}

	flock_set(fd, F_RDLCK);
	getchar();
	flock_set(fd, F_UNLCK);

	close(fd);
	return 0;
}