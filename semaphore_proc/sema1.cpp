#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// if not defined, define
#if defined(_GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
#else
	union semun
	{
		int val;
		struct semid_ds *buf;
		unsigned short int *array;
		struct seminfo *__buf;
	};
#endif

static int sem_initial(int semid)
{
	int ret;
	union semun semun;
	semun.val = 1; // set the semaphore to 1
	ret = semctl(semid, 0, SETVAL, semun);
	if(ret<0)
	{
		fprintf(stderr, "semctl failed\n");
	}
	return ret;
}

// p operation
static int sem_p(int semid)
{
	int ret;

	struct sembuf sembuf;
	sembuf.sem_op = -1;
	sembuf.sem_num = 0;
	sembuf.sem_flg = SEM_UNDO;

	ret = semop(semid, &sembuf, 1);
	if(ret<0)
	{
		fprintf(stderr, "semop failed\n");
	}
	return ret;
}

// v operation
static int sem_v(int semid)
{
	int ret;

	struct sembuf sembuf;
	sembuf.sem_op = 1;
	sembuf.sem_num = 0;
	sembuf.sem_flg = SEM_UNDO;

	ret = semop(semid, &sembuf, 1);
	if(ret<0)
	{
		fprintf(stderr, "semop failed\n");
	}
	return ret;
}

int main(int argc, char const *argv[])
{
	int i;
	int ret;
	int semid;

	// get the semaphore
	semid = semget((key_t)1234, 1, 0666|IPC_CREAT);
	if(semid<0)
	{
		fprintf(stderr, "semget failed\n");
		exit(1);
	}

	if(argc>1)
	{
		ret = sem_initial(semid);
		if(ret<0)
		{
			fprintf(stderr, "sem_initial failed\n");
			exit(1);		
		}		
	}

	for (i=0; i<5; i++) 
	{
		// p	
		if(sem_p(semid)==-1)
		{
			exit(1);
		}

		/* 模拟临界区----begin */
		printf("Process(%d) In\n", getpid());		
		sleep(2);
		printf("Process(%d) Out\n", getpid());
        /* 模拟临界区----end */ 
 
		// v
  		if(sem_v(semid)==-1)
		{
			exit(1);
		}
		sleep(1);
	}
	return 0;
}