// ip地址和端口号使用硬编码
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
	struct event *ev;
	char buf[BUFLEN];
}ConnectStat;

static struct event_base *base;

int tcp_server_init();
void accept_connection(int fd, short events, void *arg);
void do_echo_request(int fd, short events, void *arg);
void do_echo_response(int fd, short events, void *arg);

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

	ConnectStat *stat = (ConnectStat *)calloc(1, sizeof(ConnectStat));
	assert(stat!=NULL);
	struct event *ev = event_new(base, connfd, EV_READ, do_echo_request, (void *)stat);
	stat->ev = ev;

	event_add(ev, NULL);
}

void do_echo_request(int fd, short events, void *arg)
{
	ConnectStat *stat = (ConnectStat *)arg;
	struct event *ev = stat->ev;
	char *msg = stat->buf;

	int len = read(fd, msg, BUFLEN-1);
	if(len<=0)
	{
		close(fd);
		free(stat);
		event_free(ev);
		printf("close\n");
		if(len<0) perror("read failed");
		return;
	}

	msg[len] = '\0';
	printf("received: %s", msg);
	event_set(ev, fd, EV_WRITE, do_echo_response, (void *)stat);
	stat->ev->ev_base = base;
	event_add(ev, NULL);
}

void do_echo_response(int fd, short events, void *arg)
{
	ConnectStat *stat = (ConnectStat *)arg;
	struct event *ev = stat->ev;
	char *msg = stat->buf;

	int len = strlen(msg);
	int s = write(fd, msg, len);
	if(s<=0)
	{
		close(fd);
		free(stat);
		event_free(ev);
		printf("close\n");
		if(len<0) perror("write failed");
		return;
	}

	printf("sent back: %s", msg);
	event_set(ev, fd, EV_READ, do_echo_request, (void *)stat);
	stat->ev->ev_base = base;
	event_add(ev, NULL);
}