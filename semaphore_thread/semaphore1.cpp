#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>

// synchronization between two threads -> run in a certain order
// global variable
sem_t close_door;
sem_t stop_car;

using std::endl;
using std::cout;

// thread function
void *driver(void *arg)
{
	for(int i=0;i<5;i++)
	{
		sem_wait(&close_door);
		cout << "driver Starts the vehicle: " << i << endl;

		cout << "driver drives the vehicle: " << i << endl;

		cout << "driver stops the vehicle at the station: " << i << endl;
		sem_post(&stop_car);		
	}
}

void *conductor(void *argc)
{
	for(int i=0;i<5;i++)
	{
		cout << "conductor closes the door: " << i << endl;
		sem_post(&close_door);

		cout << "conductor sells tickets: " << i << endl;

		sem_wait(&stop_car);
		cout << "conductor opens the door: " << i << endl;	
	}
}

int main(int argc, char const *argv[])
{
	pthread_t tid[2];
	// initialize two semaphores to 0
	sem_init(&close_door, 0, 0);
	sem_init(&stop_car, 0, 0);

	// start thread
	pthread_create(tid, NULL, driver, NULL);
	pthread_create(tid+1, NULL, conductor, NULL);

	// recycle threads
	for(int i=0;i<2;i++)
	{
		pthread_join(tid[i], NULL);
	}

	sem_destroy(&close_door);
	sem_destroy(&stop_car);
	return 0;
}