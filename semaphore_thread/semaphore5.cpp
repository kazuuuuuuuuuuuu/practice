#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>
#include <queue>

using std::cout;
using std::endl;
using std::queue;

// put up to 3 fruits in the plate
class Plate
{
private:
	int apple_num;
	int orange_num;
	queue<int> apple_queue;
	queue<long long> orange_queue;
public:
	Plate():
	apple_num(0), orange_num(0)
	{
	}

	int pop_apple();
	long long pop_orange();
	void produce(int apple);
	void produce(long long orange);
};

int Plate::pop_apple()
{
	if(apple_num>=1)
	{
		int apple = apple_queue.front();
		apple_queue.pop();
		apple_num --;
		return apple;
	}
	else
		return -1;
}

long long Plate::pop_orange()
{
	if(orange_num>=1)
	{
		long long orange = orange_queue.front();
		orange_queue.pop();
		orange_num --;
		return orange;
	}
	else
		return -1;
}

void Plate::produce(int apple)
{
	apple_queue.push(apple);
	apple_num ++;
}

void Plate::produce(long long orange)
{
	orange_queue.push(orange);
	orange_num ++;
}

sem_t empty;
sem_t apple_full;
sem_t orange_full;

sem_t plate_mutex;
sem_t cout_mutex;

// thread function
void *father(void *arg)
{
	Plate *plate = reinterpret_cast<Plate *>(arg);

	for(int i=0;i<5;i++)
	{
		sem_wait(&cout_mutex);
		int apple = i;
		cout << "father is making an apple:" << apple  << endl;
		sem_post(&cout_mutex);

		sem_wait(&empty);

		sem_wait(&cout_mutex);
		cout << "father puts the apple into the plate:" << apple << endl;	
		sem_post(&cout_mutex);

		sem_wait(&plate_mutex);
		plate->produce(apple);
		sem_post(&plate_mutex);

		sem_post(&apple_full);	
	}
}

void *mother(void *arg)
{
	Plate *plate = reinterpret_cast<Plate *>(arg);

	for(int i=0;i<5;i++)
	{
		sem_wait(&cout_mutex);
		long long orange = i + 10000;
		cout << "mother is making an orange:" << orange  << endl;
		sem_post(&cout_mutex);

		sem_wait(&empty);

		sem_wait(&cout_mutex);
		cout << "mother puts the orange into the plate:" << orange << endl;	
		sem_post(&cout_mutex);

		sem_wait(&plate_mutex);
		plate->produce(orange);
		sem_post(&plate_mutex);

		sem_post(&orange_full);	
	}
}

void *daughter(void *arg)
{
	int apple;
	Plate *plate = reinterpret_cast<Plate *>(arg);
	
	for(int i=0;i<5;i++)
	{
		sem_wait(&apple_full);

		sem_wait(&plate_mutex);
		apple = plate->pop_apple();
		sem_post(&plate_mutex);
		sem_wait(&cout_mutex);
		cout << "daughter takes one apple from plate: " << apple << endl;
		sem_post(&cout_mutex);

		sem_post(&empty);

		sem_wait(&cout_mutex);
		cout << "daughter eats one apple: " << apple << endl;
		sem_post(&cout_mutex);		
	}
}

void *son(void *arg)
{
	long long orange;
	Plate *plate = reinterpret_cast<Plate *>(arg);
	
	for(int i=0;i<5;i++)
	{
		sem_wait(&orange_full);

		sem_wait(&plate_mutex);
		orange = plate->pop_orange();
		sem_post(&plate_mutex);
		sem_wait(&cout_mutex);
		cout << "son takes one orange from plate: " << orange << endl;
		sem_post(&cout_mutex);

		sem_post(&empty);

		sem_wait(&cout_mutex);
		cout << "son eats one orange: " << orange << endl;
		sem_post(&cout_mutex);		
	}
}

int main(int argc, char const *argv[])
{
	Plate plate;
	pthread_t tid[4];
	sem_init(&empty, 0, 3);
	sem_init(&cout_mutex, 0, 1);
	sem_init(&plate_mutex, 0, 1);
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
	sem_destroy(&cout_mutex);
	sem_destroy(&apple_full);
	sem_destroy(&orange_full);
	return 0;
}