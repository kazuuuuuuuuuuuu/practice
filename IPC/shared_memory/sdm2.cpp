#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<string.h>

struct Conn_stat
{
	int count;
	char ip[64];	
};

// file mapping -> shared memory
int main(int argc, char const *argv[])
{
	struct Conn_stat stat = {0, "127.0.0.1"};

	// 1 open the file
	const char *shm_name = "my_shm";
	int shm_fd = shm_open(shm_name, O_CREAT|O_RDWR, 0666);
	if(shm_fd<0)
	{
		fprintf(stderr, "shm_open failed\n");
		exit(1);
	}

	// 2 set the file size
	ftruncate(shm_fd, sizeof(struct Conn_stat));

	// 3 map the file into memory
	struct Conn_stat *ptr = (struct Conn_stat *)mmap(NULL, sizeof(struct Conn_stat), PROT_WRITE, MAP_SHARED, shm_fd, 0);	
	if(ptr==MAP_FAILED)
	{
		fprintf(stderr, "mmap failed\n");
		exit(1);	
	}
	close(shm_fd);

	// 4 write to the shared memory
	memcpy(ptr, &stat, sizeof(struct Conn_stat));
	int i = 0;
	while(i<10)
	{
		ptr->count ++;
		sleep(1);
		i ++;
	}

	// 5 unmap
	int ret = munmap(ptr, sizeof(struct Conn_stat));
	if(ret<0)
	{
		fprintf(stderr, "munmap failed\n");
		exit(1);	
	}
	return 0;
}