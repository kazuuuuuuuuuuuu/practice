#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

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
	ret = semctl(semid, 0, SETVAL, semun); //set
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

	string content = "student \n";
	if(argc>1)
	{
		ret = sem_initial(semid);
		if(ret<0)
		{
			fprintf(stderr, "sem_initial failed\n");
			exit(1);		
		}	
		content = "teacher\n";	
	}

	std::ofstream file("text1.txt", std::ios::app | std::ios::out);
    if (!file.is_open()) 
    {
        std::cerr << "can not open" << std::endl;
        exit(1);
    }	

	for (i=0; i<5; i++) 
	{
		// p	
		if(sem_p(semid)==-1)
		{
			exit(1);
		}

		/* 模拟临界区----begin */
		file << content;
		file.flush();
		cout << content;
		sleep(2);
        /* 模拟临界区----end */ 
 
		// v
  		if(sem_v(semid)==-1)
		{
			exit(1);
		}
		sleep(1);
	}
	file.close();
	return 0;
}