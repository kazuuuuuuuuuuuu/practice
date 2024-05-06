#include <fcntl.h>
#include <unistd.h>

// create a daemon process
int daemon()
{
	// 1 fork()
	switch(fork())
	{
	case -1:
		return -1;
	case 0:
		break;
	default:
		// the parent process exit immediatly without cleaning
		_exit(0);
	}

	// 2 Create a new session -> to disconnect association with terminal 
	if(setsid()==-1)
		return -1;

	// 3 change working directory -> to avoid effecting file system
	(void) chdir("/");

	// 4 redirect to /dev/null
	int fd;
	if((fd=open("/dev/null", O_RDWR, 0))!=-1)
	{
		// (void) -> return value should be ignored
		(void)dup2(fd, STDIN_FILENO);
		(void)dup2(fd, STDOUT_FILENO);
		(void)dup2(fd, STDERR_FILENO);
		if(fd>2)
			(void)close(fd);
	}
	return 0;
}