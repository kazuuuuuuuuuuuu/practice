daemon process -> detach from a terminal and work in the background mode
1 main:
start the task -> start_worker_processes(4) -> specifies 4 child processes
manage and recycle the child processes
2 start_worker_processes:
create a child process 4 times
3 spawn_process:
fork() -> create process
call the callback function -> worker_process_cycle
the child process exit safely
4 worker_process_cycle:
void * parameter type -> for universal uses
bind this process to a cpu -> worker_process_init
work
5 worker_process_init:
clean the structure
set the cup number
set the affinity
