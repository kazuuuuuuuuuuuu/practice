#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>

struct Conn_stat
{
	int count;
	char ip[64];	
};

int main(int argc, char const *argv[])
{
	void *shm = NULL;
	struct Conn_stat *stat = NULL;
	int shmid;

	// 1 create a shared memory
	shmid = shmget((key_t)1234, sizeof(struct Conn_stat), 0666|IPC_CREAT);
	if(shmid<0)
	{
		fprintf(stderr, "shmget failed\n");
		exit(1);
	}

	// 2 mount the shared memory to the virtual memory
	shm = shmat(shmid, (void *)0, 0);
	if(shm==(void *)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(2);
	}
	printf("the shared memory attached at %p\n", shm);	

	stat = (struct Conn_stat *)shm;

	// 3 read from the shared memory
	int i = 0;
	while(i<10)
	{
		printf("ip = %s, count: %d\n", stat->ip, stat->count);
		sleep(1);
		i ++;
	}

	// 4 detach the shared memory
	if(shmdt(shm)==-1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(3);		
	}

	// 5 delete the shared memory
	if(shmctl(shmid, IPC_RMID, 0)==-1)
	{
		fprintf(stderr, "shmctl failed: %s\n", strerror(errno));
		exit(3);				
	}

	return 0;
}