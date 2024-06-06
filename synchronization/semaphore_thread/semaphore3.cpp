#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>

// producer-consumer problems (bounded-buffer)

// global variable
sem_t empty; // = buff_size
sem_t full; // = 0

// used for control the critical zone 
sem_t mutex1; // mutex for the buffer
sem_t mutex_cout; // mutex for cout -> it is also a public resource
// circular buffer
int out = 0;
int in = 0;
int buff_size = 3;

using std::cout;
using std::endl;

// thread function
void *producer(void *arg)
{
	int *buf = reinterpret_cast<int *>(argc);
	for(int i=0;i<5;i++)
	{
		sem_wait(&mutex_cout);
		cout << "producer " << pthread_self() % 100 << " is working " << i << "th time" << endl;
		sem_post(&mutex_cout);

		sem_wait(&empty);
		
		// dont expand the critical zone
		sem_wait(&mutex1);

		sem_wait(&mutex_cout);
		cout << "producer " << pthread_self() % 100 << " puts the product into the buffer, product: " << i << endl;
		sem_post(&mutex_cout);
		
		buf[in] = i;
		in = (in+1) % buff_size;
		sem_post(&mutex1);

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
		
		sem_wait(&mutex1);

		product = buf[out];
		out = (out+1) % buff_size;

		sem_wait(&mutex_cout);
		cout << "consumer " << pthread_self() % 100 << " picks ["<< product << "] from the buffer " << i << "th time" << endl;
		sem_post(&mutex_cout);

		sem_post(&mutex1);

		sem_post(&empty);

		sem_wait(&mutex_cout);
		cout << "consumer " << pthread_self() % 100 << " is consuming a product: " << product << ", " << i << "th time" << endl;
		sem_post(&mutex_cout);
	}
}

int main(int argc, char const *argv[])
{
	pthread_t tid[4];
	int buf[buff_size];
	sem_init(&empty, 0, buff_size);
	sem_init(&full, 0, 0);
	sem_init(&mutex1, 0, 1);
	sem_init(&mutex_cout, 0, 1);

	for(int i=0;i<2;i++)
	{
		pthread_create(tid+i, NULL, producer, reinterpret_cast<void *>(buf));
	}
	for(int i=2;i<4;i++)
	{
		pthread_create(tid+i, NULL, consumer, reinterpret_cast<void *>(buf));
	}
	for(int i=0;i<4;i++)
	{
		pthread_join(tid[i], NULL);
	}

	sem_destroy(&full);
	sem_destroy(&empty);
	sem_destroy(&mutex1);
	sem_destroy(&mutex_cout);
	return 0;
}