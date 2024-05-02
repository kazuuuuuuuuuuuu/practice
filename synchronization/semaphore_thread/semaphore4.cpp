#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>

using std::cout;
using std::endl;

// only one fruit can be put into the plate
union fruit
{
	int apple;
	long long orange;
};

sem_t empty;
sem_t apple_full;
sem_t orange_full;

sem_t cout_mutex;

// the thread function -> using structure pointer to pass the whole parameter list
void *father(void *arg)
{
	fruit *plate = reinterpret_cast<fruit *>(arg);

	for(int i=0;i<5;i++)
	{
		sem_wait(&cout_mutex);
		cout << "father is making an apple:" << i  << endl;
		sem_post(&cout_mutex);

		sem_wait(&empty);

		sem_wait(&cout_mutex);
		cout << "father puts the apple into the plate:" << i << endl;	
		sem_post(&cout_mutex);
		plate->apple = i;

		sem_post(&apple_full);	
	}
}

void *mother(void *arg)
{
	fruit *plate = reinterpret_cast<fruit *>(arg);

	for(int i=0;i<5;i++)
	{
		sem_wait(&cout_mutex);
		cout << "mother is making an orange:" << i + 10000 << endl;
		sem_post(&cout_mutex);

		sem_wait(&empty);

		sem_wait(&cout_mutex);
		cout << "mother puts the orange into the plate: " << i + 10000 << endl;
		sem_post(&cout_mutex);
		plate->orange = i + 10000;

		sem_post(&orange_full);
	}
}

void *daughter(void *arg)
{
	fruit hold;
	fruit *plate = reinterpret_cast<fruit *>(arg);
	
	for(int i=0;i<5;i++)
	{
		sem_wait(&apple_full);

		hold = *plate;
		sem_wait(&cout_mutex);
		cout << "daughter takes one apple from plate: " << hold.apple << endl;
		sem_post(&cout_mutex);

		sem_post(&empty);

		sem_wait(&cout_mutex);
		cout << "daughter eats one apple: " << hold.apple << endl;
		sem_post(&cout_mutex);		
	}
}

void *son(void *arg)
{
	fruit hold;
	fruit *plate = reinterpret_cast<fruit *>(arg);

	for(int i=0;i<5;i++)
	{
		sem_wait(&orange_full);

		hold = *plate;
		sem_wait(&cout_mutex);
		cout << "son takes one orange from the plate: " << hold.orange << endl;
		sem_post(&cout_mutex);

		sem_post(&empty);

		sem_wait(&cout_mutex);
		cout << "son eats one orange: " << hold.orange << endl;
		sem_post(&cout_mutex);		
	}
}

int main(int argc, char const *argv[])
{
	fruit plate;
	pthread_t tid[4];
	sem_init(&empty, 0, 1);
	sem_init(&cout_mutex, 0, 1);
	sem_init(&orange_full, 0, 0);
	sem_init(&apple_full, 0, 0);


	pthread_create(tid, NULL, father, reinterpret_cast<void *>(&plate));
	pthread_create(tid+1, NULL, mother, reinterpret_cast<void *>(&plate));
	pthread_create(tid+2, NULL, daughter, reinterpret_cast<void *>(&plate));
	pthread_create(tid+3, NULL, son, reinterpret_cast<void *>(&plate));

	for(int i=0;i<4;i++)
	{
		pthread_join(tid[i], NULL);
	}

	sem_destroy(&empty);
	sem_destroy(&cout_mutex);
	sem_destroy(&apple_full);
	sem_destroy(&orange_full);
	return 0;
}