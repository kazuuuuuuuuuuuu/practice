#include "global.h"

// 每个fd 一个结构体 send_buf保存需要echo的内容
#define BUFLEN 1024
typedef struct ConnectStat_
{
	int fd;
	char send_buf[BUFLEN];
	PF *handler; // pf * = 函数指针 -> 回调函数 ->  分别发送不同的页面
}ConnectStat;

void accept_connection(int fd, void *data);
void do_echo_handler(int fd, void  *data);
void do_echo_response(int fd,void *data);
void do_echo_timeout(int fd, void *data);

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

// 写 welcome信息
void do_welcome_handler(int fd, void *data)
{
	ConnectStat *stat = (ConnectStat *)data;

	const char *welcome = "welcome.\n";
	int wlen = strlen(welcome);

	// 1 将welcome信息发给用户
	int s = write(fd, welcome, wlen);
	if(s>0)
	{
		// 2 转换为读事件
		commUpdateReadHandler(fd, do_echo_handler, (void *)stat);
		// 3 设置fd下次的超时时间
		commSetTimeout(fd, 10, do_echo_timeout, (void *)stat);		
	}
	else if(s==0)
	{
		fprintf(stderr, "connection fd: %d has been closed\n", fd);
		comm_close(fd);
		free(stat);
	}
	else
	{
		fprintf(stderr, "write failed fd: %d, reason: %s\n", fd, strerror(errno));
	}
}

// 写 echo信息
void do_echo_response(int fd, void *data)
{
	ConnectStat *stat = (ConnectStat *)data;

	int len = strlen(stat->send_buf);
	
	// 1 将buffer内容发给用户
	int s = write(fd, stat->send_buf, len);
	if(s>0)
	{
		// 2 转换为读事件
		commUpdateReadHandler(fd, do_echo_handler, (void *)stat);
		// 3 设置fd下次的超时时间
		commSetTimeout(fd, 10, do_echo_timeout, (void *)stat);		
	}
	else if(s==0)
	{
		fprintf(stderr, "connection fd: %d has been closed\n", fd);
		comm_close(fd);
		free(stat);
	}
	else
	{
		fprintf(stderr, "write failed fd: %d, reason: %s\n", fd, strerror(errno));
	}
}

// 读echo信息
void do_echo_handler(int fd, void *data)
{
	ConnectStat *stat = (ConnectStat *)data;
	
	char *p = stat->send_buf;
	*p ++ = '-';
	*p ++ = '>';
	// 1 从用户读数据 到准备发送的buffer中
	// BUFLEN - (p - stat->send_buf) - 1 == buffer的剩余大小 - 1
	int s = read(fd, p, BUFLEN - (p - stat->send_buf) - 1);
	if(s>0)
	{
		p[s] = '\0';
		printf("received from client: %s\n", p);
		if(strncasecmp(p, "quit", 4)==0)
		{
			comm_close(fd);
			free(stat);
			return;
		}
		// 2 转换为写事件
		commUpdateWriteHandler(fd, do_echo_response, (void *)stat);
		// 3 设置fd下次的超时时间
		commSetTimeout(fd, 10, do_echo_timeout, (void *)stat);
	}
	else if(s==0)
	{
		fprintf(stderr, "connection fd: %d has been closed\n", fd);
		comm_close(fd);
		free(stat);
	}
	else
	{
		fprintf(stderr, "read failed fd: %d, reason: %s\n", fd, strerror(errno));
	}
}

// 这里data没有用到 只是为了统一回调函数的接口
void accept_connection(int fd, void *data)
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
		set_nonblock(connect_fd);
		// 1 转换为写事件
		commUpdateWriteHandler(connect_fd, do_welcome_handler, (void *)stat);
		// 2 设置fd下次的超时时间
		commSetTimeout(connect_fd, 30, do_echo_timeout, (void *)stat);
	}
}

void do_echo_timeout(int fd, void *data)
{
	fprintf(stdout, "----------time out, fd: %d-----------\n", fd);
	comm_close(fd);
	free(data);
}

int main(int argc, char const *argv[])
{
	int listen_fd = startup();

	comm_init(102400);
	commUpdateReadHandler(listen_fd, accept_connection, NULL);
	do
	{
		// 不断等待事件到来并处理
		comm_select(1000);
	}while(1);

	comm_select_shutdown();
	return 0;
}