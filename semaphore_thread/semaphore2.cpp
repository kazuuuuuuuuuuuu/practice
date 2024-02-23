#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>

// producer-consumer problem (single buffer)
// global variable
sem_t empty;
sem_t full;

using std::cout;
using std::endl;

// thread function
void *producer(void *arg)
{
	int *buf = reinterpret_cast<int *>(argc);
	for(int i=0;i<5;i++)
	{
		cout << "producer is working" << endl;
		
		sem_wait(&empty);
		cout << "producer puts the product into the buffer, product: " << i << endl;
		*buf = i;

		sem_post(&full);
	}
}

void *consumer(void *argc)
{
	int *buf = reinterpret_cast<int *>(argc);
	int product;
	for(int i=0;i<5;i++)
	{
		sem_wait(&full);
		cout << "consumer picks one from the buffer" << endl;
		product = *buf;

		sem_post(&empty);
		cout << "consumer is consuming a product: " << product << endl;
	}
}

int main(int argc, char const *argv[])
{
	pthread_t tid[2];
	// initialize the semaphore
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, 1);

	// start a thread
	int buffer;
	pthread_create(tid, NULL, producer, reinterpret_cast<void *>(&buffer));	
	pthread_create(tid+1, NULL, consumer, reinterpret_cast<void *>(&buffer));	

	for(int i=0;i<2;i++)
	{
		pthread_join(tid[i], NULL);
	}

	sem_destroy(&full);
	sem_destroy(&empty);
	return 0;
}