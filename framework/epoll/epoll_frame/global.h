#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <sys/time.h>
#include <sys/resource.h>	
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

#define COMM_OK		  (0)
#define COMM_ERROR	 (-1)
#define COMM_NOMESSAGE	 (-3)
#define COMM_TIMEOUT	 (-4)
#define COMM_SHUTDOWN	 (-5)
#define COMM_INPROGRESS  (-6)
#define COMM_ERR_CONNECT (-7)
#define COMM_ERR_DNS     (-8)
#define COMM_ERR_CLOSING (-9)

// 调试信息等级
#define DEBUG_LEVEL 0
#define DEBUG_ONLY 8
#define debug(m, n) if(m>=DEBUG_LEVEL&&n<=DEBUG_ONLY) printf

typedef void PF(int, void *); // 函数指针声明 PF *pt; 
// typedef void (*PF)(int, void *);  -> 函数指针声明 PF pt; 

// 一个fd相关的信息 包含读处理函数 写处理函数 超时处理函数
typedef struct _fde
{
	unsigned int type;
	u_short local_port;
	u_short remote_port;
	struct in_addr local_addr;

	char ipaddr[16]; // dotted decimal address of peer

	PF *read_handler;
	void *read_data; // read_handler函数的参数
	PF *write_handler;
	void *write_data; // write_handler函数的参数
	PF *timeout_handler;
	void *timeout_data; // timeout_handler函数的参数
	time_t timeout; 
}fde;

extern fde *fd_table; // fd = 1 的fde信息保存在 fd_table[1]
extern int Biggest_FD; // 该数组的大小

// 系统时间 -> 全局变量
extern struct timeval current_time;
extern double current_dtime;
extern time_t sys_curtime;

// epoll 接口 供内部使用
void do_epoll_init(int max_fd);
void do_epoll_shutdown();
void epollSetEvents(int fd, int need_read, int need_write);
int do_epoll_select(int msec);

// 框架外围接口
void comm_close(int fd);
void comm_init(int max_fd);
int comm_select(int msec); // epoll wait
// inline函数定义写在.h文件中
inline void comm_call_handlers(int fd, int read_event, int write_event)
{
	fde *F = &fd_table[fd];

	if(F->read_handler&&read_event)
	{
		PF *hdl = F->read_handler;
		void *hdl_data = F->read_data;
		debug(5, 8) ("comm_call_handlers(): calling read handler on fd = %d\n", fd);
		hdl(fd, hdl_data);
	}

	if(F->write_handler&&write_event)
	{
		PF *hdl = F->write_handler;
		void *hdl_data = F->write_data;
		debug(5, 8) ("comm_call_handlers(): calling write handler on fd = %d\n", fd);
		hdl(fd, hdl_data);
	}
}
void commUpdateReadHandler(int fd, PF *handler, void *data);
void commUpdateWriteHandler(int fd, PF *handler, void *data);

const char *xstrerror();

time_t getCurrentTime();
void comm_select_shutdown();
int commSetTimeout(int fd, int timeout, PF *handler, void *data);

#endif