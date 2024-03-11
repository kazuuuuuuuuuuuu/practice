#include "thread_pool.h"

static void thread_pool_exit_handler(void *data);
static void *thread_pool_cycle(void *data);
static int_t thread_pool_init_default(thread_pool_t *tpp, char *name);

// global variable
static uint_t thread_pool_task_id; 
static int debug = 0;

// initialize the pool
thread_pool_t * thread_pool_init()
{
	int err;
	pthread_t tid;
	uint_t n;
	pthread_attr_t attr;
	thread_pool_t *tp = NULL;

	tp = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
	if(tp==NULL)
	{
		fprintf(stderr, "thread_pool_init: calloc failed\n");
		return NULL;
	}

	thread_pool_init_default(tp, NULL);

	thread_pool_queue_init(&tp->queue);

	if(thread_mutex_create(&tp->mtx)!=OK)
	{
		free(tp);
		return NULL;
	} 

	if(thread_cond_create(&tp->cond)!=OK)
	{
		free(tp);
		(void) thread_mutex_destroy(&tp->mtx);
		return NULL;
	}

	err = pthread_attr_init(&attr);
	if(err!=0)
	{
		fprintf(stderr, "pthread_attr_init failed: %s\n", strerror(errno));
		free(tp);
		(void) thread_mutex_destroy(&tp->mtx);
		(void) thread_cond_destroy(&tp->cond);
		return NULL;
	}

	// The thread is a separate from the main thread and the main thread cannot wait with a join
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(err!=0)
	{
		fprintf(stderr, "pthread_attr_setdetachstate failed: %s\n", strerror(errno));
		free(tp);
		(void) thread_mutex_destroy(&tp->mtx);
		(void) thread_cond_destroy(&tp->cond);
		(void) pthread_attr_destroy(&attr);
		return NULL;
	}

	for(n=0;n<tp->threads;n++)
	{
		err = pthread_create(&tid, &attr, thread_pool_cycle, tp);
		if(err)
		{
			fprintf(stderr, "pthread_create failed: %s\n", strerror(errno));
			free(tp);
			(void) thread_mutex_destroy(&tp->mtx);
			(void) thread_cond_destroy(&tp->cond);
			(void) pthread_attr_destroy(&attr);
			return NULL;			
		}
	}

	(void) pthread_attr_destroy(&attr);
	return tp;
}

static int_t thread_pool_init_default(thread_pool_t *tpp, char *name)
{
	if(tpp)
	{
		tpp->threads = DEFAULT_THREADS_NUM;
		tpp->max_queue = DEFAULT_QUEUE_NUM;
		tpp->name = strdup(name?name:"default"); // copy and return the pointer to the copy
		if(debug) fprintf(stderr, "thread_pool_init, name: %s, threads: %lu, max_queue: %ld\n", tpp->name, tpp->threads, tpp->max_queue);
		return OK;
	}
	return ERROR;
}

static void *thread_pool_cycle(void *data)
{
	thread_pool_t *tp = (thread_pool_t *)data;
	int err;
	thread_task_t *task;

	if(debug) fprintf(stderr, "thread in pool \"%s\" started\n", tp->name);

	while(1)
	{
		if(thread_mutex_lock(&tp->mtx)!=OK)
		{
			return NULL;
		}

		tp->waiting --;

		while(tp->queue.first==NULL)
		{
			if(thread_cond_wait(&tp->cond, &tp->mtx)!=OK)
			{
				(void) thread_mutex_unlock(&tp->mtx);
				return NULL;
			}
		}

		// 1 acquire a task node from the linked list
		task = tp->queue.first;
		tp->queue.first = task->next;

		if(tp->queue.first==NULL)
		{
			tp->queue.last = &tp->queue.first;
		}

		if(thread_mutex_unlock(&tp->mtx)!=OK)
		{
			return NULL;
		}

		if(debug) fprintf(stderr, " run task #%lu in thread pool \"%s\"\n", task->id, tp->name);
		// 2 run that task
		task->handler(task->ctx);

		if(debug) fprintf(stderr, " complete task #%lu in thread pool \"%s\"\n", task->id, tp->name);

		task->next = NULL;

		free(task);
		task = NULL;
	}
}

// allocate memory for the task node with its parameters(structure)
thread_task_t *thread_task_alloc(size_t size) // the size of the parameters (structure) of the callback function
{
	thread_task_t *task;
	task = (thread_task_t *)calloc(1, sizeof(thread_task_t) + size); // calloc -> allocate memory and initialize it to 0
	if(task==NULL)
	{
		return NULL;
	}
	task->ctx = task + 1; // point to the parameters (structure) of the callback function
	return task;
}

// post the task to the task queue
int_t thread_task_post(thread_pool_t *tp, thread_task_t *task)
{
	// acquire mutex
	if(thread_mutex_lock(&tp->mtx)!=OK)
	{
		return ERROR;
	}

	// reach the maximum
	if(tp->waiting>=tp->max_queue)
	{
		(void) thread_mutex_unlock(&tp->mtx);
		fprintf(stderr, "thread_pool \"%s\" queue overflow: %ld tasks wating\n", tp->name, tp->waiting);
		return ERROR;
	}

	task->id = thread_pool_task_id ++;
	task->next = NULL;

	// signal
	if(thread_cond_signal(&tp->cond)!=OK)
	{
		(void) thread_mutex_unlock(&tp->mtx);
		return ERROR;		
	}

	// 1 post the task to the queue
	*(tp->queue.last) = task;
	tp->queue.last = &task->next;
	// 2 update 
	tp->waiting ++;

	(void) thread_mutex_unlock(&tp->mtx);
	
	if(debug)
	{
		fprintf(stderr, "thread_pool \"%s\" added a task: %lu\n", tp->name, task->id);
	}
	return OK;
}

void thread_pool_destroy(thread_pool_t *tp)
{
	uint_t n;
	thread_task_t task;
	volatile uint_t lock; // volatile -> Every access will read the latest value from memory

	memset(&task, 0, sizeof(thread_task_t));
	task.handler = thread_pool_exit_handler;
	task.ctx = (void *) &lock;

	for(n=0;n<tp->threads;n++)
	{
		lock = 1;

		if(thread_task_post(tp, &task)!=OK)
		{
			return;
		}

		while(lock)
		{
			// make the main thread to relinquish(give up) the CPU
			// wait the thread to kill itself and set the lock to 0
			sched_yield();
		}
	}

	(void) thread_mutex_destroy(&tp->mtx);
	(void) thread_cond_destroy(&tp->cond);	
	free(tp);
}

// close the thread
static void thread_pool_exit_handler(void *data)
{
	uint_t *lock = (uint_t *)data;
	*lock = 0;
	pthread_exit(0);
}