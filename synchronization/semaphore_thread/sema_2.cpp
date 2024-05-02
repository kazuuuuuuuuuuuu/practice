#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
// test for semaphore > 1
sem_t road;


// thread function
void *cars(void *arg)
{
	printf("%u intend to pass the fork\n", pthread_self());
	sleep(1);

	// p(road)
	sem_wait(&road);
	printf("%u is at the fork\n", pthread_self());
	sleep(1);

	printf("%u have passed the fork\n", pthread_self());
	sleep(1);

	// v(road)
	sem_post(&road);

	pthread_exit(0);
}

int main(int argc, char const *argv[])
{
	pthread_t tid[5];
	sem_init(&road, 0, 3);
	// create threads
	for(int i=0;i<5;i++)
	{
		pthread_create(tid+i, NULL, cars, NULL);
	}

	// recycle threads
	for(int i=0;i<5;i++)
	{
		pthread_join(tid[i], NULL);
	}

	sem_destroy(&road);
	return 0;
}