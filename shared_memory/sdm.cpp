#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

struct Conn_stat
{
	int count;
	char ip[64];	
};

// system v shared memory
int main(int argc, char const *argv[])
{
	void *shm = NULL;
	int shmid = 0;
	int i = 0;
	struct Conn_stat stat = {0, "127.0.0.1"};

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

	// 3 write content to the shared memory
	struct Conn_stat *p = (struct Conn_stat *)shm;
	memcpy(p, &stat, sizeof(struct Conn_stat));

	while(i<10)
	{
		p->count ++;
		sleep(1);
		i ++;
	}

	// 4 detach the shared memory
	if(shmdt(shm)==-1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(3);		
	}

	return 0;
}