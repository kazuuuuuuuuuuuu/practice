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
#include <arpa/inet.h>

int connect_server();
void socket_read_data(int fd, short events, void *arg);
void cmd_read_data(int fd, short events, void *arg);

int main(int argc, char const *argv[])
{
	int connfd = connect_server();
	if(connfd==-1)
		return -1;

	printf("connect to server successfully\n");
	struct event_base *base = event_base_new();;

	// 监听connfd上的读事件 并读取
	struct event *ev_conn = event_new(base, connfd, EV_READ|EV_PERSIST, socket_read_data, NULL);
	event_add(ev_conn, NULL);

	// 监听stdin上的读事件 并读取 之后发送给客户端
	struct event *ev_cmd = event_new(base, STDIN_FILENO, EV_READ|EV_PERSIST, cmd_read_data, (void *)&connfd);
	event_add(ev_cmd, NULL);	
	
	event_base_dispatch(base);

	printf("finished\n");
	return 0;
}

int connect_server()
{
	struct sockaddr_in server_addr;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9999);
	server_addr.sin_addr.s_addr = inet_addr("192.168.226.129");	

	int connfd = socket(AF_INET, SOCK_STREAM, 0);
	if(connfd==-1)
	{
		perror("socket failed");
		return -1;
	}

	if(connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
	{
		perror("connect failed");
		close(connfd);
		return -1;		
	}

	return connfd;
}

void socket_read_data(int fd, short events, void *arg)
{
	char msg[1024];

	int len = read(fd, msg, sizeof(msg)-1);
	if(len==0)
	{
		printf("connection closed\n");
		exit(1);
	}
	else if(len<0)
	{
		perror("read failed\n");
		return;
	}

	msg[len] = '\0';
	printf("recv from server >>> %s", msg);
}

void cmd_read_data(int fd, short events, void *arg)
{
	char msg[1024];

	int len = read(fd, msg, sizeof(msg)-1);
	if(len<=0)
	{
		printf("reading from cmd failed\n");
		exit(1);
	}

	msg[len] = '\0';
	printf("write to server >>> %s", msg);	
	write(*((int *)arg), msg, len);
}