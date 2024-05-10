// ip地址和端口号使用硬编码
// libevent 使用内部自带的buffer完成读写
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <event.h>
#include <event2/event.h>
#include <assert.h> 
#include <unistd.h>
#include <fcntl.h>


#define BUFLEN 1024
typedef struct _ConnectStat
{
	struct bufferevent *bev;
	char buf[BUFLEN];
}ConnectStat;

static struct event_base *base;

int tcp_server_init();
void accept_connection(int fd, short events, void *arg);
void do_echo_request(struct bufferevent *bev, void *arg);
void event_cb(struct bufferevent *bev, short events, void *arg);

int main(int argc, char const *argv[])
{
	int listenfd = tcp_server_init();
	if(listenfd==-1)
	{
		return -1;
	}

	base = event_base_new();

	struct event *ev_listen = event_new(base, listenfd, EV_READ|EV_PERSIST, accept_connection, NULL);
	event_add(ev_listen, NULL);

	event_base_dispatch(base);

	return 0;
}

void set_nonblock(int fd)
{
	int fl = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, fl|O_NONBLOCK);
}

int tcp_server_init()
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd==-1)
	{
		perror("socket failed");
		return -1;
	}

	// 设置地址可重复使用
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(9999);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd, (struct sockaddr *)&sin, sizeof(sin))<0)
	{
		perror("bind failed");
		return -1;
	}

	if(listen(listenfd, 10)<0)
	{
		perror("listen failed");
		return -1;		
	}

	// 设置为非阻塞
	set_nonblock(listenfd);

	return listenfd;
}

void accept_connection(int fd, short events, void *arg)
{
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int connfd = accept(fd, (struct sockaddr *)&client_addr, &len);

	set_nonblock(connfd);

	printf("new client: %d\n", connfd);

	struct bufferevent *bev = bufferevent_socket_new(base, connfd, BEV_OPT_CLOSE_ON_FREE);
	ConnectStat *stat = (ConnectStat *)calloc(1, sizeof(ConnectStat));
	assert(stat!=NULL);
	stat->bev = bev;
	
	bufferevent_setcb(bev, do_echo_request, NULL, event_cb,(void *)stat);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

void do_echo_request(struct bufferevent *bev, void *arg)
{
	ConnectStat *stat = (ConnectStat *)arg;
	char *msg = stat->buf;

	// 从内部缓存拷贝到msg 不存在出错的情况
	int len = bufferevent_read(bev, msg, BUFLEN-1);
	msg[len] = '\0';
	printf("received: %s", msg);
	bufferevent_write(bev, msg, strlen(msg));
}

// 出错由该函数处理
void event_cb(struct bufferevent *bev, short events, void *arg)
{
	ConnectStat *stat = (ConnectStat *)arg;

	if(events & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}
	else if(events & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}

	// 自动关闭套接字和free读写缓冲区
	bufferevent_free(bev);
	free(stat);
}