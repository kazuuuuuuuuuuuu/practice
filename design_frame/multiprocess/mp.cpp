//#define _GNU_SOURCE (enabled by default) // inform the compiler of including support for GNU extension features. -> set cpu affinity
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

int daemon()
{
	// 1 fork()
	switch(fork())
	{
	case -1:
		return -1;
	case 0:
		break;
	default:
		// the parent process exit immediatly without cleaning
		_exit(0);
	}

	// 2 Create a new session -> to disconnect association with terminal 
	if(setsid()==-1)
		return -1;

	// 3 change working directory -> to avoid effecting file system
	(void) chdir("/");

	// 4 redirect to /dev/null -> devour everything
	int fd;
	if((fd=open("/dev/null", O_RDWR, 0))!=-1)
	{
		// (void) -> return value should be ignored
		(void)dup2(fd, STDIN_FILENO); // redirect std to /dev/null
		(void)dup2(fd, STDOUT_FILENO);
		(void)dup2(fd, STDERR_FILENO);
		if(fd>2)
			(void)close(fd); // closing fd does not effect stdin, out, err 
	}
	return 0;
}

// function poiter renaming -> spawn_proc_pt the function pointer type
// void * data -> universal pointer ->  the type of parameter varies based on the callback function
typedef void (*spawn_proc_pt) (void *data);
static void worker_process_cycle(void *data);
static void start_worker_processes(int n);
pid_t spawn_process(spawn_proc_pt proc, void *data, const char *name);

int main(int argc, char const *argv[])
{
	// became a daemon process -> detach from terminal (optional)
	daemon();
	
	start_worker_processes(4);
	
	// the master process manages child processes here
	
	wait(NULL);
	printf("success\n");
	return 0;
}

void start_worker_processes(int n)
{
	int i;
	for(i=n-1;i>=0;i--)
	{
		// worker_process_cycle is a function pointer -> the tast should be completed by the child process
		// (intptr_t) -> convert i from int to int *, (void *) -> convert it to void * 
		spawn_process(worker_process_cycle, (void *)(intptr_t) i, "worker process");
	}
}

// start a process and execute that function pointed by the function pointer
pid_t spawn_process(spawn_proc_pt proc, void *data, const char *name)
{
	pid_t pid;
	pid = fork();

	switch(pid)
	{
	case -1:
		fprintf(stderr, "fork() failed while spawning: %s\n", name);
		return -1;
	// worker process 
	case 0: 
		(*proc)(data);
		exit(EXIT_SUCCESS); // child process should end safely
	default:
		break;
	}
	printf("start process:%s %d\n", name, pid);
	return pid;
}

// bind the process with a cpu
static void worker_process_init(int worker)
{
	cpu_set_t cpu_affinity;
	CPU_ZERO(&cpu_affinity);
	CPU_SET(worker %CPU_SETSIZE, &cpu_affinity);
	// set affinity -> 0 indicates current process/thread that needs to be delt with
	if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_affinity)==-1)
	{
		fprintf(stderr, "sched_setaffinity() failed, worker: %d\n", worker);
	}
}

void worker_process_cycle(void *data)
{
	int worker = (intptr_t) data;
	// bind the process with a cpu
	worker_process_init(worker);

	// work
	for (int i = 0; i < 3; ++i)
	{
		sleep(10);
		printf("pid %d is working\n", getpid());
	}
}