#include "csapp.h"
#define MAXARGS 128

// function prototypes
void eval(char *cmdline);
// 第二个参数 -> char *的指针 -> 元素为char *的数组
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int int main()
{
	// 用于存放 command line
	char cmdline[MAXLINE];

	while(1)
	{
		// read
		printf("> "); // 命令行提示符
		Fgets(cmdline, MAXLINE, stdin);
		//  the end-of-file condition is encountered -> exit
		// 到达文件结尾 -> 正常退出
		if(feof(stdin))
			exit(0);

		// evaluate
		eval(cmdline);
	}
}

// parseline - parse the command line and build the argv array
// 解析以空格分隔的命令行参数 并构造传递给execve的argv数组(向量)
// 第一个参数为 1) 内置的shell名 2) 可执行目标文件
int parseline(char *buf, char **argv)
{
	// 返回值为1or0
	// 1->在后台执行该程序->shell不等待他完成
	// 0->在前台执行该程序->shell会等待他完成

	// points to first space delimiter
	char *delim;
	// argv中的元素数量
	int argc;
	// background job -> 是否在后台运行？
	int bg;

	// strlen会包括末尾的\n(不包括\0) -> 将末尾的\n替换为空格
	buf[strlen(buf)-1] = ' ';
	// 用指针的方式遍历字符串 -> 跳过前面的空格
	while(*buf&&((*buf)==' '))
		buf ++:

	// build the argv
	argc = 0;
	// 在buf中找空格 -> 找到返回指针 找不到返回null
	while((delim = strchr(buf, ' ')))
	{
		// 直接在buf字符串上操作
		// argv每个元素都是一个字符指针 -> 一个字符串
		// 指针赋值 -> 指向相同的地方
		argv[argc] = buf; 
		argc ++;
		// 以此空格位置为界 用\0分割
		*delim = '\0';
		// buf指向下一起始位置
		buf = delim + 1;
		// 跳过空格
		while(*buf&&((*buf)==' '))
			buf ++:
	}

	// 最后的位置指向NULL
	argv[argc] = NULL;

	if(argc==0)
		return 1;

	// 检查最后一个argument的第一个字符是否是& -> 将结果传递给bg -> 用于返回
	// 是的话在后台执行 并且删除该参数 -> 这个并不是实际运行时需要的参数
	if( (bg = (*argv[argc-1]=='&') ) !=0)
	{
		argv[argc-1] = NULL;
		argc --;
	}

	return bg;
}

// evaluate a command line
void eval(char *cmdline)
{
	// 参数数组
	char *argv[MAXARGS];
	char buf[MAXLINE];
	int bg;
	pid_t pid;

	strcpy(buf, cmdline);
	// 从经过整理的的命令函输入中 生成参数数组
	// 返回是否后台执行的flag
	bg = parseline(buf, argv);
	if(argv[0]==NULL)
		return;

	if(!builtin_command(argv))
	{
		// 用子进程执行
		if( (pid = Fork()) == 0)
		{
			if(execve(argv[0], argv, environ)<0)
			{
				// 运行失败
				printf("%s : command not found. \n", argv[0]);
				exit(0);
			}
		}

		// parent waits for foreground job to terminate
		// 1 -> background, 0 -> foregound
		if(!bg)
		{
			int status;
			// shell挂起 -> 等待进程号为pid的进程完成
			if(waitpid(pid, &status, 0)<0)
				unix_error("waitfg: waitpid error");

		}
		else
			printf("%d %s", pid, cmdline);
	}
	return;
}

int builtin_command(char **argv)
{
	// the result will be 0, if those match -> ! negates the result to 1
	// 做了一个简化 实际中shell有大量的内置命令
	if(!strcmp(argv[0], "quit"))
		exit(0);
	if(!strcmp(argv[0], "&"))
		return 1;
	return 0;
}

// 上面的shell并不等待并回收在后台运行的进程
// 接下来为完整版

#include "csapp.h"
#define MAXARGS 128

// function prototypes
void eval(char *cmdline);
int parseline(char *buf, char * *argv);
int builtin_command(char * *argv);

int main()
{
	char cmdline[MAXLINE];
	while(1)
	{
		printf("> ");
		// read command line
		Fgets(cmdline, MAXLINE, stdin);
		if(feof(stdin))
			exit(0);

		// parse and execute command line
		eval(cmdline);
	}
}

int parseline(char *buf, char * *argv)
{
	char *delim;
	int argc;
	int bg;

	buf[strlen(buf)-1] = ' ';
	while(*buf && (*buf==' '))
		buf ++;

	// build argv array from the command line string
	argc = 0;
	while((delim = strchr(buf, ' ')))
	{
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while(*buf && (*buf==' '))
			buf ++;
	}

	argv[argc] = NULL;

	if(argc==0)
		return 1;

	if((bg = (*argv[argc-1])=='&')!=0)
		argv[--argc] = NULL;

	return bg;
}

int builtin_command(char * *argv)
{
	if(!strcmp(argv[0], "quit"))
		exit(0);
	if(!strcmp(argv[0], "&"))
		return 1;
	return 0;
}

void eval(char *cmdline)
{
	char *argv[MAXARGS];
	char buf[MAXLINE];
	int bg;
	pid_t pid;

	strcpy(buf, cmdline);
	bg = parseline(buf, argv);
	if(argv[0]==NULL)
		return;

	if(!builtin_command(argv))
	{
		// child process executes the command line
		if((pid=Fork())==0)
		{
			if(execve(argv[0], argv, environ)<0)
			{
				printf("%s: command not found.\n", argv[0]);
				exit(0);
			}
		}

		if(!bg)
		{
			int status;
			if(waitpid(pid, &states, 0)<0)
				unix_error("waitfg: waitpid error");
		}
		else
			printf("%d, %s", pid, cmdline);
	}
	return;
}

// ---------------------------------------------------------------------------------------
void eval(char *cmdline)
{
	char *argv[MAXARGS]; // argument array
	char buf[MAXLINE]; // holds modified command line
	int bg; // should the job run in bg or fg? background or foreground?
	pid_t pid; // process id

	// buf 将会被修改 所以先把原cmdline复制到buf之中
	strcpy(buf, cmdline);
	bg = parse_line(buf, argv);
	if(argv[0]==NULL)
		return;

	if(!builtin_command(argv))
	{
		sigset_t mask_one, prev_one;
		Sigemptyset(&mask_one);
		// build the mask that only blocks the child signal
		Sigaddset(&mask_one, SIGCHLD);

		// block child signal
		Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
		if((pid=Fork())==0)
		{
			// unblock child signal in child process
			Sigprocmask(SIG_SETMASK, &prev_one, NULL);

			// set the process group id to its own pid
			Setpgid(0, 0);

			if(execve(argv[0], argv, environ)<0)
			{
				printf("%s: command not found.\n", argv[0]);
				exit(0);
			}
		}

		sigset_t mask_all, prev_all;
		// build the mask that blocks all signals
		Sigfillset(&mask_all);

		// save job info
		Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
		Jid new_jid = new_job(pid, cmdline, !bg);
		Sigprocmask(SIG_SETMASK, &prev_all, NULL);

		// reap the child
		if(!bg)
		{
			set_fg_pid(pid);
			while(get_fg_pid())
				// temporarily lift the blocking until a signal is received
				sigsuspend(&prev_one); 
		}
		else
			printf("[%d] %d %s \t %s\n", new_jid, pid, "Running", cmdline);

		// unblock child signal
		Sigprocmask(SIG_SETMASK, &prev_one, NULL);
	}
	return;
}

// if argv[0] is a builtin command, run it and return true
// else return false
int builtin_command(char * *argv)
{
	if(!strcmp(argv[0], "quit"))
		exit(0);
	if(!strcmp(argv[0], " &"))
		return 1;
	if(!strcmp(argv[0], "jobs"))
	{
		printf_jobs();
		return 1;
	}

	// > fg -> 用户在shell中键入了fg指令
	// 将进程放在前台执行 -> shell 将等待他暂停或结束
	if(!strcmp(argv[0], "fg"))
	{
		int id;
		// format : fg %ddd or fg ddd 
		if((id=parse_id(argv[1]))!=-1&&argv[2]==NULL)
		{
			sigset_t mask_one, prev_one;
			Sigemptyset(&mask_one);
			Sigaddset(&mask_one, SIGCHLD);

			Sigprocmask(SIG_BLOCK, &maask_one, &prev_one);

			pid_t pid = id;
			// if param is jid -> '%ddd'
			if(argv[1][0]=='%')
			{
				JobPtr jp = find_job_by_jid(id);
				pid = jp->pid;
			}
			// instruct it to continue execution
			Kill(pid, SIGCONT);
			
			set_fg_pid(pid);
			while(get_fg_pid())
				sigsuspend(&prev_one);

			Sigprocmask(SIG_SETMASK, &prev_one, NULL);
		}
		else
			printf("format error, e.g. fg %12 || fg 1498\n");

		return 1;
	}

	// > bg
	// 在后台继续运行
	if(!strcmp(argv[0], "bg"))
	{
		int id;
		// format: bg %ddd
		if((id=parse_id(argv[1]))!=-1&&argv[2]==NULL)
		{
			pid_t pid = id;
			// jid parameter
			if(argv[1][0]=='%')
			{
				JobPtr jp = find_job_by_jid(id);
				pid = jp->pid;
			}
			// 在后台执行
			// 虽然更改了 前台进程id 但是shell不等他结束 所以还是在后台执行
			Kill(pid, SIGCONT);
		}
		else
			printf("format error, e.g. bg%12 or bg 1498\n");

		return 1;
	}
	// not a  builtin command
	return 0;
}

int parse_line(char *buf, char * *argv)
{
	char *delim;
	int argc;
	int bg;

	buf[strlen(buf)-1] = ' ';
	while(*buf && (*buf == ' '))
		buf ++;

	argc = 0;
	while((delim = strchr(buf, ' ')))
	{
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while(*buf && (*buf==' '))
			buf ++;
	}

	argv[argc] = NULL;

	if(argc==0)
		return 1;

	if((bg=(argv[argc-1]=='&'))!=0)
		argv[--argc] = NULL;

	return bg;
}

static int is_number_str(char *s)
{
	int len = strlen(s);
	for(int i=0;i<len;i++)
	{
		if(!isdigit(s[i]))
			return 0;
	}
	return 1;
}

int parse_id(char *s)
{
	int error = -1;
	if(s==NULL)
		return error;

	if(s[0]=='&')
	{
		// move to the next position in s
		if(!is_number_str(s+1))
			return error;
		return atoi(s+1);
	}

	if(is_number_str(s))
		return atoi(s);

	return error;
}

void test_shell()
{
	// parse id
	assert(-1 == parse_id("ns"));
	assert(-1 == parse_id("%%"));
	assert(0 == parse_id("%0"));
	assert(0 == parse_id("0"));
	assert(98 == parse_id("%98"));
	assert(98 == parse_id("98"));
}

// record the foreground process id
static volatile sig_atomic_t fg_pid;
static Job jobs[MAXJOBS];

int is_fg_pid(pid_t pid)
{
	return fg_pid == pid;
}

pid_t get_fg_pid()
{
	return fg_pid;
}

void set_fg_pid(pid_t pid)
{
	fg_pid = pid;
}

void sigchild_handler(int sig)
{
	int old_errno = errno;
	int status;
	pid_t pid;

	sigset_t mask_all, prev_all;
	Sigfillset(&mask_all);

	// exit or be stopped or continue
	while((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED))>0)
	{
		// exit normally
		if(WIFEXITED(status)||WIFSIGNALED(status))
		{
			if(is_fg_pid(pid))
				set_fg_pid(0);
			// print out the info using the safe customized function
			else
			{
				Sio_puts("pid");Sio_putl(pid);Sio_puts(" terminates\n");
			}
			Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
			del_job_by_pid(pid);
			Sigprocmask(SIG_SETMASK, &prev_all, NULL);	
		}

		// be stopped
		if(WIFSTOPPED(status))
		{
			if(is_fg_pid(pid))
				set_fg_pid(0);

			// set pid status stopped
			Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
			JobPtr jp = find_job_by_pid(pid);
			set_job_status(jp, Stopped);
			Sigprocmask(SIG_SETMASK, &prev_all, NULL);

			Sio_puts("pid");Sio_putl(pid);Sio_puts(" be stopped\n");
		}

		//	continue -> 只挂起前台运行的进程 所以恢复时 要修改 前台正在循行的id
		// 0 -> pid;
		if(WIFCONTINUED(status))
		{
			set_fg_pid(pid);
			Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
			JobPtr jp = find_job_by_pid(pid);
			set_job_status(jp, Running);
			Sigprocmask(SIG_SETMASK, &prev_all, NULL);		

			Sio_puts("pid");Sio_putl(pid);Sio_puts(" continue\n");	
		}
	}
	errno = old_errno;
}

void sigint_handler(int sig)
{
	// when fg_pid==0, stop shell itself, it'll ve a dead loop
	if(is_fg_pid(0))
	{
		Signal(SIGINT, SIG_DFL);
		Kill(getpid(), SIGINT);
	}
	else
		Kill(get_fg_pid(), SIGINT);
}

void sigstop_handler(int sig)
{
	if(is_fg_pid(0))
	{
		Signal(SIGTSTP, SIG_DFL);
		Kill(getpid(), SIGTSTP);
	}
	else
	{
		Kill(get_fg_pid(), SIGTSTP);
	}
}

JobPtr find_job_by_jid(Jid jid)
{
	return &(jobs[jid]);
}

JobPtr find_job_by_pid(pid_t pid)
{
	for(int i=0;i<MAXJOBS;i++)
	{
		Job j = jobs[i];
		if(j.using&&j.pid==pid)
			return &(jobs[i]);
	}

	return NULL;
}

void set_job_status(JobPtr jp, enum JobStatus status)
{
	if(jp)
		jp->status = status;
}

static int find_spare_jid()
{
	Jid jid = -1;
	for(itn i=0;i<MAXJOBS;i++)
	{
		if(jobs[i].using==0)
		{	
			jib = i;
			break;
		}
	}
	return jid;
}

int new_job(pid_t pid, char *cmdline, int fg)
{
	Jid jid = find_spare_jid();
	if(jid == -1)
		unix_error("no more jid to use");

	jobs[jid].jid = jid;
	jobs[jid].pid = pid;
	jobs[jid].status = Running;
	strcpy(jobs[jid].cmdline, cmdline);
	jobs[jid].using = 1;

	return jid;
}

// 通过把using改为0 -> 达成删除job的目的
void del_job_by_pid(pid_t pid)
{
	for(int i=0;i<MAXJOBS;i++)
	{
		if(jobs[i].using && jobs[i].pid==pid)
			jobs[i].using = 0;
	}
}

void print_jobs()
{
	for(int i=0;i<MAXJOBS;i++)
	{
		Job j = jobs[i];
		if(j.using)
		{
			printf(" ")
		}
	}
}

void init_jobs()
{
	memset(jobs, 0, sizeof(jobs));
}