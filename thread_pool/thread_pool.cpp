#include "thread_pool.h"

static void thread_pool_exit_handler(void *data);
static void *thread_pool_cycle(void *data);
static int_t thread_pool_init_default(thread_pool_t *tpp, char *name);

// global variable
static uint_t thread_pool_task_id = 0; 
static int debug = 0;

// initialize the pool
thread_pool_t * thread_pool_init()
{
	int err;
	
	// 1 allocate memory
	thread_pool_t *tp = NULL;
	tp = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
	if(tp==NULL)
	{
		fprintf(stderr, "thread_pool_init: calloc failed\n");
		return NULL;
	}

	// 2 set the value 
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

	// 3 Start each thread to run the thread cycle
	pthread_attr_t attr;
	err = pthread_attr_init(&attr);
	if(err!=0)
	{
		fprintf(stderr, "pthread_attr_init failed: %s\n", strerror(errno));
		free(tp);
		(void) thread_mutex_destroy(&tp->mtx);
		(void) thread_cond_destroy(&tp->cond);
		return NULL;
	}

	// detach those threads from the main thread
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

	pthread_t tid; // all threads use the same tid
	for(uint_t n=0;n<tp->threads;n++)
	{
		err = pthread_create(&tid, &attr, thread_pool_cycle, tp);
		if(err!=0)
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
		tpp->waiting = 0;

		tpp->name = strdup(name?name:"default"); // copy string and return the pointer to the string
		tpp->threads = DEFAULT_THREADS_NUM;
		tpp->max_queue = DEFAULT_QUEUE_NUM;
		
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
		// 1 acquire the mutex
		if(thread_mutex_lock(&tp->mtx)!=OK)
		{
			return NULL;
		}

		// 2 no tasks in the queue -> wait for the signal
		while(tp->queue.first==NULL)
		{
			if(thread_cond_wait(&tp->cond, &tp->mtx)!=OK)
			{
				(void) thread_mutex_unlock(&tp->mtx);
				return NULL;
			}
		}

		// 3 acquire a task node from the linked list
		tp->waiting --;
		task = tp->queue.first;
		tp->queue.first = task->next;
		if(tp->queue.first==NULL)
		{
			tp->queue.last = &tp->queue.first;
		}

		// 4 release the mutex
		if(thread_mutex_unlock(&tp->mtx)!=OK)
		{
			return NULL;
		}
		
		// 5 run that task
		if(debug) fprintf(stderr, " run task #%lu in thread pool \"%s\"\n", task->id, tp->name);
		task->handler(task->ctx);
		if(debug) fprintf(stderr, " complete task #%lu in thread pool \"%s\"\n", task->id, tp->name);

		// 6 release the task
		task->next = NULL;
		free(task);
		task = NULL;
	}
}

// allocate memory for the task node and its parameter structure
thread_task_t *thread_task_alloc(size_t size) // the size of the parameter structure
{
	thread_task_t *task;
	task = (thread_task_t *)calloc(1, sizeof(thread_task_t) + size); // calloc -> allocate memory and initialize it to 0
	if(task==NULL)
	{
		return NULL;
	}
	task->ctx = task + 1; // point to the parameters structure
	return task;
}

// push the task to the task queue
int_t thread_task_post(thread_pool_t *tp, thread_task_t *task)
{
	// 1 acquire mutex
	if(thread_mutex_lock(&tp->mtx)!=OK)
	{
		return ERROR;
	}

	// 2 if it reachs the maximum -> exit
	if(tp->waiting>=tp->max_queue)
	{
		(void) thread_mutex_unlock(&tp->mtx);
		fprintf(stderr, "thread_pool \"%s\" queue overflow: %ld tasks wating\n", tp->name, tp->waiting);
		return ERROR;
	}

	// 3 set remaining task info
	task->id = thread_pool_task_id ++;
	task->next = NULL;

	// 4 signal to the threads
	if(thread_cond_signal(&tp->cond)!=OK)
	{
		(void) thread_mutex_unlock(&tp->mtx);
		return ERROR;		
	}

	// 5 push the task to the queue
	*(tp->queue.last) = task;
	tp->queue.last = &task->next;
	
	// 6 update info 
	tp->waiting ++;

	// 7 release the mutex
	(void) thread_mutex_unlock(&tp->mtx);
	
	if(debug)
	{
		fprintf(stderr, "thread_pool \"%s\" added a task: %lu\n", tp->name, task->id);
	}
	return OK;
}

// post the task to close threads
void thread_pool_destroy(thread_pool_t *tp)
{
	volatile uint_t lock; // volatile -> Every access will read the latest value from memory

	// 1 create a task
	thread_task_t task;
	memset(&task, 0, sizeof(thread_task_t));
	task.handler = thread_pool_exit_handler;
	task.ctx = (void *) &lock;

	// 2 post the task -> each task close one thread
	for(uint_t n=0;n<tp->threads;n++)
	{
		lock = 1;

		if(thread_task_post(tp, &task)!=OK)
		{
			return;
		}

		while(lock)
		{
			// if lock is not modified to 0 by thread_pool_exit_handler
			// make the main thread to relinquish (give up) the CPU
			// wait the thread to kill itself and set the lock to 0
			sched_yield();
		}
	}

	// 3 destory mutex and cond
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