#include "thread_pool.h"
// the parameter structure
struct test
{
	int arg1;
	int arg2;
};
// callback function
void task_hanlder1(void *data)
{
	static int index = 0;
	printf("hello, this is task_hanlder1(%d)\r\n", index++);
}

void task_hanlder2(void *data)
{
	static int index = 0;
	printf("hello, this is task_hanlder2(%d)\r\n", index++);
}

void task_hanlder3(void *data)
{
	static int index = 0;
	struct test *p = (struct test *)data;
	printf("hello, this is task_hanlder3(%d)\r\n", index++);
	printf("arg1: %d, arg2: %d\n", p->arg1, p->arg2);
}

int main(int argc, char const *argv[])
{
	thread_pool_t *tp = NULL;
	
	// 1 initilaize the thread pool
	tp = thread_pool_init();
	// 2 formulate the task
	thread_task_t *test1 = thread_task_alloc(0);
	thread_task_t *test2 = thread_task_alloc(0);
	thread_task_t *test3 = thread_task_alloc(sizeof(struct test));
	test1->handler = task_hanlder1;
	test2->handler = task_hanlder2;
	test3->handler = task_hanlder3;
	((struct test *)test3->ctx)->arg1 = 666;
	((struct test *)test3->ctx)->arg2 = 888;
	// 3 post the task to the task queue
	thread_task_post(tp, test1);
	thread_task_post(tp, test2);
	thread_task_post(tp, test3);
	// 4 destroy thread pool
	sleep(10);
	thread_pool_destroy(tp);
	return 0;
}