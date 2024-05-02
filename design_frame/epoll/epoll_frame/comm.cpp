#include "global.h"

// 外部变量的定义
fde *fd_table = NULL; // fd = 1 的信息保存在 fd_table[1]
int Biggest_FD = 1024; // 该数组的大小

struct timeval current_time;
double current_dtime;
time_t sys_curtime;

static int MAX_POLL_TIME = 1000;

// 更新当前系统时间
time_t getCurrentTime()
{
	gettimeofday(&current_time, NULL);
	// 更高精度的时间
	current_dtime = (double) current_time.tv_sec + (double) current_time.tv_usec / 1000000.0;
	// 低精度的时间
	return sys_curtime = current_time.tv_sec;
}

const char *xstrerror()
{
	// BUFSIZ常量是I/O缓冲区的大小
	static char xstrerror_buf[BUFSIZ];
	const char *errmsg;

	errmsg = strerror(errno);

	if(!errmsg||!*errmsg)
	{
		errmsg = "Unknown error";
	}

	snprintf(xstrerror_buf, BUFSIZ, "(%d) %s", errno, errmsg);
	return xstrerror_buf;
}

void comm_init(int max_fd)
{
	if(max_fd>0) Biggest_FD = max_fd;
	fd_table = (fde *)calloc(Biggest_FD, sizeof(fde));
	do_epoll_init(Biggest_FD);
}

void comm_close(int fd)
{
	assert(fd>0);
	fde *F = &fd_table[fd];
	if(F) memset((void *)F, 0, sizeof(fde));
	epollSetEvents(fd, 0, 0);
	close(fd);
}

void comm_select_shutdown()
{
	do_epoll_shutdown();
	if(fd_table) free(fd_table);
}

void commUpdateReadHandler(int fd, PF *handler, void *data)
{
	fd_table[fd].read_handler = handler;
	fd_table[fd].read_data = data;

	epollSetEvents(fd, 1, 0);
}

void commUpdateWriteHandler(int fd, PF *handler, void *data)
{
	fd_table[fd].write_handler = handler;
	fd_table[fd].write_data = data;

	epollSetEvents(fd, 0, 1);	
}

// 设置fde结构体中的超时回调函数及其参数 以及超时的deadline
int commSetTimeout(int fd, int timeout, PF *handler, void *data)
{
	assert(fd>=0&&fd<Biggest_FD);
	
	debug(5, 3) ("commSetTimeout: FD %d timeout: %d\n", fd, timeout);	

	fde *F = &fd_table[fd];

	if(timeout<0)
	{
		F->timeout_handler = NULL;
		F->timeout_data = NULL;
		return F->timeout = 0;
	}

	F->timeout_handler = handler;
	F->timeout_data = data;
	
	return F->timeout = sys_curtime + (time_t) timeout;
}

static void checkTimeouts()
{
	fde *F = NULL;
	PF *callback;

	// 遍历fa_table
	for(int fd=0;fd<=Biggest_FD;fd++)
	{
		F = &fd_table[fd];

		if(F->timeout==0)
			continue;
		if(F->timeout>sys_curtime)
			continue;

		// 当前时间已经超过设置的deadline -> 已超时
		if(F->timeout_handler) // 设置超时函数的话就使用该回调函数
		{
			debug(5, 5) ("checkTimeouts: FD %d call timeout handler\n", fd);
			callback = F->timeout_handler;
			F->timeout_handler = NULL;
			callback(fd, F->timeout_data);
		}
		else // 否则就直接关掉
		{
			debug(5, 5) ("checkTimeouts: FD %d forcing comm_close\n", fd);
			comm_close(fd);
		}
	}
}

int comm_select(int msec)
{
	// static 只初始化一次 永远存在
	static double last_timeout = 0.0;
	debug(5, 3) ("comm_select: timeout %d\n", msec);

	if(msec>MAX_POLL_TIME)
		msec = MAX_POLL_TIME;

	// 使这个检查时间永远为1s
	// 如果当前时间距离上次检查的时间已经超过1s -> 检查一遍是否有超时的fd
	if(last_timeout+0.999<current_dtime)
	{
		last_timeout = current_dtime;
		checkTimeouts();
	}
	else
	{
		// 没过1s 看还差多少 -> 通过msec等待时间加到1s
		int max_timeout = (last_timeout + 1.0 - current_dtime) * 1000;
		if(max_timeout<msec)
			msec = max_timeout;
	}

	int rc = do_epoll_select(msec);
	// 更新当前时间
	getCurrentTime();

	if(rc==COMM_TIMEOUT)
		debug(5, 8) ("comm_select: time out\n");

	return rc;
}