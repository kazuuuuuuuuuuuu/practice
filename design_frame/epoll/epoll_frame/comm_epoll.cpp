#include "global.h"
#include <sys/epoll.h>

#define MAX_EVENTS 256 // 一次可以处理的最大的事件数

static int kdpfd;
static struct epoll_event events[MAX_EVENTS];
static int epoll_fds = 0;
static unsigned int *epoll_state; // 保存每个epoll的事件类型

// 把epoll的控制信息转换成字符
// 将epfd中的事件做修改int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
// op 取值 EPOLL_CTL_ADD， EPOLL_CTL_DEL， EPOLL_CTL_MOD
static const char *epolltype_atoi(int x)
{
	switch(x)
	{
	case EPOLL_CTL_ADD:
		return "EPOLL_CTL_ADD";
	case EPOLL_CTL_DEL:
		return "EPOLL_CTL_DEL";
	case EPOLL_CTL_MOD:
		return "EPOLL_CTL_MOD";
	default:
		return "UNKNOWN_EPOLLCTL_OP";
	}
}

// 为linux中可以忽略的错误 不影响程序运行
static int ignoreErrno(int ierrno)
{
	switch(ierrno)
	{
	case EINPROGRESS:
	case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
	case EAGAIN:
#endif
	case EALREADY:
	case EINTR:
#ifdef ERESTART
	case ERESTART:
#endif
		return 1;
	default:
		return 0;
	}
}

void do_epoll_init(int max_fd)
{
	kdpfd = epoll_create(max_fd);
	if(kdpfd<0)
	{
		fprintf(stderr, "do_epoll_init: epoll_create(): %s\n", xstrerror());
	}
	epoll_state = (unsigned int*)calloc(max_fd, sizeof(unsigned int));
}

void do_epoll_shutdown()
{
	close(kdpfd);
	kdpfd = -1;
	if(epoll_state) free(epoll_state);
}

void epollSetEvents(int fd, int need_read, int need_write)
{
	assert(fd>=0);
	debug(5, 8) ("commSetEvents(fd=%d)\n", fd);

	/*
	库中已定义的相关结构体 
	一个事件包括事件的类型和数据
	struct epoll_event
	{
		_uint32_t events; 事件类型
		epoll_data_t data; 数据
	};

	数据是个union 可选择不同类型
	typedef union epoll_data
	{
		void *ptr; // 数据可以是一个指针
		int fd; 
		uint32_t u32;
		uint64_t u64;
	}epoll_data_t;
	*/

	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = 0;
	ev.data.fd = fd;

	if(need_read)
	{
		ev.events |= EPOLLIN;
	}

	if(need_write)
	{
		ev.events |= EPOLLOUT;
	}

	// 如果有读写
	if(need_read||need_write)
	{
		ev.events |= (EPOLLHUP | EPOLLERR);
	}

	// 如果fd当前的事件类型（epoll_state[fd]）和要修改的事件不相同 -> 需要修改
	if(ev.events != epoll_state[fd])
	{
		int epoll_ctl_type = 0;
		if(ev.events==0) // ev.events 为空 -> 不是读也不是写 -> 删除
		{
			epoll_ctl_type = EPOLL_CTL_DEL;
		}
		else if(epoll_state[fd])
		{
			epoll_ctl_type = EPOLL_CTL_MOD;
		}
		else // epoll_state[fd] == 0
		{
			epoll_ctl_type = EPOLL_CTL_ADD;
		}

		epoll_state[fd] = ev.events;

		if(epoll_ctl(kdpfd, epoll_ctl_type, fd, &ev)<0)
		{
			debug(5, 1) ("commSetEvents: epoll_ctl(%s): failed on fd = %d: %s\n", epolltype_atoi(epoll_ctl_type), fd, xstrerror());
		}

		switch(epoll_ctl_type) // epoll_fds 为epoll当前监听的fd的总数
		{
		case EPOLL_CTL_ADD:
			epoll_fds ++;
			break;
		case EPOLL_CTL_DEL:
			epoll_fds --;
			break;
		default:
			break;
		}
	}
}

int do_epoll_select(int msec)
{
	int num = epoll_wait(kdpfd, events, MAX_EVENTS, msec); // msec 为每次检查的间隔时间
	if(num<0)
	{
		if(ignoreErrno(errno))
		{
			return COMM_OK;
		}

		debug(5, 1) ("comm_select: epoll failure: %s\n", xstrerror());
		return COMM_ERROR;
	}
	else if(num==0)
	{
		return COMM_TIMEOUT;
	}
	else
	{
		struct epoll_event *cevents;
		int i;
		// 遍历events数组
		for(i=0, cevents=events;i<num;i++, cevents++)
		{
			int fd = cevents->data.fd;
			// 分别检查 event 到底是读操作 还是写操作
			comm_call_handlers(fd, cevents->events & EPOLLIN, cevents->events & EPOLLOUT); 
		}
		return COMM_OK;
	}
}
