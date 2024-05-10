// 编译时增加参数 -levent
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <event.h>
#include <event2/event.h>
#include <sys/resource.h>	
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

struct event_base *ev_base;

// 每个fd 一个结构体 send_buf保存需要echo的内容
#define BUFLEN 1024
typedef struct ConnectStat_
{
	int fd;
	char send_buf[BUFLEN];
	struct event *ev;
}ConnectStat;

void accept_connection(int fd, short events, void *data);
void do_welcome_handler(int fd, short events, void *data);
void do_echo_handler(int fd, short events, void *data);
void do_echo_response(int fd, short events, void *data);

// 设置fd为非阻塞 -> 如果fd的数据还没有准备好或者无法立即进行操作，函数会立即返回而不是阻塞等待
void set_nonblock(int fd)
{
	int fl = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, fl|O_NONBLOCK);
}

// 启动listenfd
int startup()
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd<0)
	{
		perror("socket");
		exit(1);
	}

	// 设置fd重用
	int opt = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 服务器端地址
	struct sockaddr_in server_address;
	server_address.sin_port = htons(80);
	server_address.sin_family  = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind(listen_fd, (struct sockaddr *)&server_address, sizeof(server_address));
	if(ret<0)
	{
		perror("bind");
		exit(1);		
	}

	ret = listen(listen_fd, 5);
	if(ret<0)
	{
		perror("listen");
		exit(1);			
	}
	return listen_fd;
}

// 初始化fd的连接状态结构体
ConnectStat *stat_init(int fd)
{
	ConnectStat *temp = (ConnectStat *)calloc(1, sizeof(ConnectStat));
	if(temp==NULL)
	{
		fprintf(stderr, "malloc failed:%s\n", strerror(errno));
		return NULL;
	}
	temp->fd = fd;
	return temp;
}

// 接受新连接 fd和events的传递是由libevent封装的 其余参数由事件的最后一个参数 回调函数的参数传递的
void accept_connection(int fd, short events, void *data)
{
	// 客户端地址
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int connect_fd = accept(fd, (struct sockaddr *)&client_addr, &len);
	if(connect_fd>0)
	{
		printf("new client: %s: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		// 有新连接创建时 一起创建他的连接状态结构体
		ConnectStat *stat = stat_init(connect_fd);
		// 设置非阻塞
		set_nonblock(connect_fd);
		// 创建事件
		struct event *ev = event_new(ev_base, connect_fd, EV_WRITE, do_welcome_handler, (void *)stat);
		stat->ev = ev;
		// 注册事件
		event_add(ev, NULL);
	}
}

// 写 welcome信息
void do_welcome_handler(int fd, short events, void *data)
{
	ConnectStat *stat = (ConnectStat *)data;

	const char *welcome = "welcome.\n";
	int wlen = strlen(welcome);

	// 1 将welcome信息发给用户
	int s = write(fd, welcome, wlen);
	if(s>0)
	{
		// 更改为读事件
		event_set(stat->ev, fd, EV_READ, do_echo_handler, (void*)stat);
		stat->ev->ev_base = ev_base; // 更改事件会清空事件结合 -> 必须重置事件集合
		event_add(stat->ev, NULL);
	}
	else if(s==0)
	{
		fprintf(stderr, "connection fd: %d has been closed\n", fd);
		event_free(stat->ev); // 自动删除监听事件 并free event
		close(fd);
		free(stat);
	}
	else
	{
		fprintf(stderr, "write failed fd: %d, reason: %s\n", fd, strerror(errno));
	}
}

// 读echo信息
void do_echo_handler(int fd, short events, void *data)
{
	ConnectStat *stat = (ConnectStat *)data;
	
	char *p = stat->send_buf;
	*p ++ = '-';
	*p ++ = '>';
	// 1 从用户读数据 到准备发送的buffer中
	// BUFLEN - (p - stat->send_buf) - 1 == buffer的剩余大小 - 1
	int s = read(fd, p, BUFLEN - (p - stat->send_buf) - 1);
	p[s] = '\0';
	if(s>0)
	{
		printf("received from client: %s\n", p);
		if(strncasecmp(p, "quit", 4)==0)
		{
			fprintf(stderr, "connection fd: %d has been closed by quit\n", fd);
			event_free(stat->ev); // 自动删除监听事件 并free event
			close(fd);
			free(stat);
			return;
		}
		// 更改为写事件
		event_set(stat->ev, fd, EV_WRITE, do_echo_response, (void*)stat);
		stat->ev->ev_base = ev_base; // 更改事件会清空事件结合 -> 必须重置事件集合
		event_add(stat->ev, NULL);
	}
	else if(s==0)
	{
		fprintf(stderr, "connection fd: %d has been closed\n", fd);
		event_free(stat->ev); // 自动删除监听事件 并free event
		close(fd);
		free(stat);
	}
	else
	{
		fprintf(stderr, "read failed fd: %d, reason: %s\n", fd, strerror(errno));
	}
}

// 写 echo信息
void do_echo_response(int fd, short events, void *data)
{
	ConnectStat *stat = (ConnectStat *)data;

	int len = strlen(stat->send_buf);
	
	// 1 将buffer内容发给用户
	int s = write(fd, stat->send_buf, len);
	if(s>0)
	{
		// 更改为读事件
		event_set(stat->ev, fd, EV_READ, do_echo_handler, (void*)stat);
		stat->ev->ev_base = ev_base; // 更改事件会清空事件结合 -> 必须重置事件集合
		event_add(stat->ev, NULL);
	}
	else if(s==0)
	{
		fprintf(stderr, "connection fd: %d has been closed\n", fd);
		event_free(stat->ev); // 自动删除监听事件 并free event
		close(fd);
		free(stat);
	}
	else
	{
		fprintf(stderr, "write failed fd: %d, reason: %s\n", fd, strerror(errno));
	}
}

int main(int argc, char const *argv[])
{
	// 1 创建listenfd
	int listen_fd = startup();

	// 2 创建事件集
	ev_base = event_base_new();
	
	// 3 创建事件
	struct event *ev_listen = event_new(ev_base, listen_fd, EV_READ|EV_PERSIST, accept_connection, NULL);

	// 4 添加事件
	event_add(ev_listen, NULL);

	// 5 不断等待事件到来并使用回调函数处理
	event_base_dispatch(ev_base);
	return 0;
}