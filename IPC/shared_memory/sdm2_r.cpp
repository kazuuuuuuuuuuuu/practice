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

int main(int argc, char const *argv[])
{
	struct Conn_stat stat = {0, "127.0.0.1"};

	// 1 open the file
	const char *shm_name = "my_shm";
	int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
	if(shm_fd<0)
	{
		fprintf(stderr, "shm_open failed\n");
		exit(1);
	}

	// 2 map the file into memory
	struct Conn_stat *ptr = (struct Conn_stat *)mmap(NULL, sizeof(struct Conn_stat), PROT_READ, MAP_SHARED, shm_fd, 0);	
	if(ptr==MAP_FAILED)
	{
		fprintf(stderr, "mmap failed\n");
		exit(1);	
	}
	close(shm_fd);

	// 3 read from the shared memory
	int i = 0;
	while(i<10)
	{
		printf("ip = %s, count: %d\n", ptr->ip, ptr->count);
		sleep(1);
		i ++;
	}

	// 4 unmap
	int ret = munmap(ptr, sizeof(struct Conn_stat));
	if(ret<0)
	{
		fprintf(stderr, "munmap failed\n");
		exit(1);		
	}

	// 5 delete the file
	shm_unlink(shm_name);	
	return 0;
}