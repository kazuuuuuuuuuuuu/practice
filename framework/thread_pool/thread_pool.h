#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "thread.h"

#define DEFAULT_THREADS_NUM 4
#define DEFAULT_QUEUE_NUM 65535

typedef struct thread_task_s thread_task_t;
typedef struct thread_pool_s thread_pool_t;

// the task node 
struct thread_task_s
{
	thread_task_t *next; // point to the next node
	uint_t id; // task id
	void *ctx; // point to the parameters for the callback function
	void (*handler)(void *data); // point to the callback function 
};

// the task queue
typedef struct 
{
	thread_task_t *first;
	thread_task_t **last; // point to the "next" pointer of the last task node
}thread_pool_queue_t;

#define thread_pool_queue_init(q) (q)->first = NULL;(q)->last = &(q)->first; // &(q)->first == &( (q)->first )

// the thread pool
struct thread_pool_s
{
	pthread_mutex_t mtx;
	pthread_cond_t cond;

	thread_pool_queue_t queue;
	int_t waiting; // the number of tasks to be processed
	
	char *name;
	uint_t threads; // the number of threads
	int_t max_queue; // the max length of the queue
};

thread_task_t *thread_task_alloc(size_t size);
int_t thread_task_post(thread_pool_t *tp, thread_task_t *task);
thread_pool_t *thread_pool_init();
void thread_pool_destroy(thread_pool_t *tp);

#endif