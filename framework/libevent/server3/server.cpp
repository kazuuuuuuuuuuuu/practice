// ip地址和端口号使用硬编码
// libevent 使用内部自带的buffer完成读写
// listen 和 accept由框架实现
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <event.h>
#include <event2/event.h>
#include<event2/listener.h>
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

void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *arg); 
void do_echo_request(struct bufferevent *bev, void *arg);
void event_cb(struct bufferevent *bev, short events, void *arg);

int main(int argc, char const *argv[])
{
	struct sockaddr_in sin = {0};
	sin.sin_family = AF_INET;
	sin.sin_port = htons(9999);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	base = event_base_new();

	// 建立链接（listen and accept）
    struct evconnlistener *listener = evconnlistener_new_bind(base, listener_cb, base, LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, 10, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));  
  
    event_base_dispatch(base);  
  
    evconnlistener_free(listener);
    event_base_free(base);

	return 0;
}

ConnectStat * stat_init(int fd, struct bufferevent *bev) 
{
	ConnectStat *temp = (ConnectStat *)malloc(sizeof(ConnectStat));
	assert(temp!=NULL);
	memset(temp, 0, sizeof(ConnectStat));
	temp->bev = bev;
}

/*
一个新客户端连接上服务器此函数就会被调用,当此函数被调用时，libevent已经帮我们accept了这个客户端。
该客户端的文件描述符为fd
*/
void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *arg)  
{  
    printf("accept a client %d\n", fd);  
  
    //为这个客户端分配一个bufferevent  
    struct bufferevent *bev =  bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    ConnectStat *stat = stat_init(fd, bev);											   
  
    bufferevent_setcb(bev, do_echo_request, NULL, event_cb, stat);  
    bufferevent_enable(bev, EV_READ | EV_PERSIST);  
}  

// 有消息来 读 并 发送
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

// 出错或断开 由该函数处理 free对应资源
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