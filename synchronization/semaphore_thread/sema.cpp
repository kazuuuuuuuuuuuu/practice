#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

// global var
static int ticket_amount = 2;
sem_t mutex; // semaphore = 1 equtes mutex;


void *ticket_agent(void *arg)
{
	sem_wait(&mutex); // p(mutex)

	// critical section
	int t = ticket_amount;
	if(t>0)
	{
		printf("one ticket sold\n");
		t --;
	}
	else
	{
		printf("tickets have sold out\n");
	}
	ticket_amount = t;

	sem_post(&mutex); // v(mutex)
	pthread_exit(0);
}

int main(int argc, char const *argv[])
{
	pthread_t ticket_agent_id[2];

	sem_init(&mutex, 0, 1); // semaphore = 1 -> mutax
	
	for(int i=0;i<2;i++)
	{
		pthread_create(ticket_agent_id+i, NULL, ticket_agent, NULL);
	}

	for(int i=0;i<2;i++)
	{
		pthread_join(ticket_agent_id[i], NULL);
	}

	printf("the left ticket is %d\n", ticket_amount);
	sem_destroy(&mutex); // release resource
	return 0;
}