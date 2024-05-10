/*
本代码并不完整
1 创建listenfd 和 保存该fd的结构体
2 创建epollfd
3 将listenfd加入到监听事件
4 epoll_wait
5 处理不同事件
5.1 新连接 创建结构体 设置为非阻塞 加入到监听事件
5.2 读事件 读 解析并录入信息 修改回调函数 转换为写事件 or 关闭套接字 删除事件
5.3 写事件 执行回调函数 转换为读事件 or 关闭套接字 删除事件
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h.h>
#include <fcntl.h>
#include <unistd.h>

/* 一个事件包括事件的类型和数据
struct epoll_event()
{
	_uint32_t events; 事件类型
	epoll_data_t data; 数据
};

数据成员是个union 可选择类型
typedef union epoll_data
{
	void *ptr;
	int fd; 数据可以是一个指针
	uint32_t u32;
	uint64_t u64;
}epoll_data_t;
*/

typedef struct ConnectStat_ ConnectStat;
typedef void (*response_handler) (ConnectStat *stat); // 事件处理函数 回调函数
typedef ConnectStat_ // 用于保存事件的上下文
{
	int fd;
	char name[64];
	char age[64];
	struct epoll_event ev;
	int status; // 0 未登录 1 已登录
	response_handler handler; // 事件处理函数
};

// 启动套接字
int startup()
{
	// 创建套接字
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket<0)
	{
		perror("socket");
		exit(1);
	}

	// 套接字重用
	int opt = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 服务器地址
	struct sockaddr_in server_address;
	server_address.sin_port = htons(80);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int ret = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
	if(ret<0)
	{
		perror("bind");
		exit(1);		
	}

	// listen
	int ret = listen(server_socket, 5);
	if(ret<0)
	{
		perror("listen");
		exit(1);		
	}
	return server_socket;
}

ConnectStat *stat_init(int fd)
{
	ConnectStat *temp = (ConnectStat *)malloc(sizeof(ConnectStat));
	if(temp==NULL)
	{
		fprintf(stderr, "stat_init failed\n");
		return NULL;
	}
	memset(temp, 0, sizeof(ConnectStat));
	temp->fd = fd;
	temp->status = 0;
}

void connect_handle(int client_socket)
{
	// 创建stat
	ConnectStat *stat = stat_init(client_socket);
	set_nonblock(client_socket); // 设置为非阻塞

	stat->ev.events = EPOLLIN;
	stat->ev.data.ptr = stat;

	// 添加事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &stat->ev);
}

void do_http_response(ConnectStat *stat)
{
	stat->handler(stat); // 未实现 1调用对应回调函数发送html 2并把事件类型重新切换为读事件
}

void do_http_request(ConnectStat *stat)
{
	char buf[4096];
	ssize_t s = read(stat->fd, buf, sizeof(buf)-1);
	char *pos = NULL;
	
	if(s>0)
	{
		buf[s] = '\0';
		printf("receive: \n%s\n", buf);

		pos = buf;
		// 1 根据用户的请求 设置事件的回调函数 用于发送不同页面
		if(strncasecmp(pos, "GET", 3)==0)
		{
			stat->handler = welcome_response_handler; // 发送静态页面
		}
		else if(strncasecmp(pos, "POST", 4)==0)
		{
			printf("--post--\n");
			pos += strlen("POST");
			while(*pos==' '||*pos=='/') pos++;

			if(strncasecmp(pos, "commit", 6)==0)
			{
				int len = 0;

				printf("--post commit--\n");
				pos = strstr(buf, "\r\n\r\n");		
				char *end = NULL;	

				if(end=strstr(pos, "name="))
				{
					
					pos = end + strlen("name="); // 起始位置
					end = pos;
					while(('a'<=*end&&*end<='z')||('A'<=*end&&*end<='Z')||('0'<=*end&&*end<='9')) end++;
					len = end - pos;
					if(len>0)
					{
						memcpy(stat->name, pos, len);
						stat->name[len] = '\0';
					}
				}

				if(end=strstr(pos, "age="))
				{
					pos = end + strlen("age="); // 起始位置
					end = pos;
					while('0'<=*end&&*end<='9') end++;
					len = end - pos;
					if(len>0)
					{
						memcpy(stat->age, pos, len);
						stat->age[len] = '\0';
					}
				}
				stat->handler = commit_response_handler; // 根据用户输入输出静态页面			
			}
			else
				stat->handler = welcome_response_handler;
		}
		else
		{
			stat->handler = welcome_response_handler;
		}

		// 2 修改对应socket的事件类型 从读事件到为写事件
		stat->ev.events = EPOLLOUT;
		epoll_ctl(epfd, EPOLL_CTL_MOD, stat->fd, &stat->ev);
	}
	else if(s==0)
	{
		printf("client: %d close\n", stat->fd);
		// 客户断开连接 删除监听事件
		epoll_ctl(epfd, EPOLL_CTL_DEL, stat->fd, NULL);
		close(stat->fd);
		free(stat);
	}
	else
		perror("do_http_request - read");
}

int main(int argc, char const *argv[])
{
	// 1 创建套接字
	int listen_socket = startup();

	// 2 创建epoll 
	int epfd = epoll_create(256);
	if(epfd<0)
	{
		perror("epoll_create");
		eixt(1);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN; // 读事件
	ConnectStat *stat = stat_init(listen_socket);
	ev.data.ptr = (void *)stat; // data 是结构体中的union ptr是void*指针 保存指向事件上下文的指针

	// 3 listen事件加入监听事件集合
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_socket, &ev);

	struct epoll_event evs[64]; // 64 为同时就绪的事件的最大上限
	int timeout = -1; // 阻塞式等待
	int num = 0;
	int done = 0;

	while(!done)
	{
		// 4 监听事件集合
		switch((num = epoll_wait(epfd, evs, 64, timeout)))
		{
		case 0:
			fprintf(stderr, "time out\n");
			break;
		case -1:
			perror("epoll_wait failed");
			break;
		default:
			struct sockaddr_in client_address;
			socklen_t len = sizeof(client_address);
			// 5 遍历已经就绪的事件
			for(int i=0;i<num;i++)
			{
				ConnectStat *stat = (ConnectStat *)evs[i].data.ptr;

				int rsocket = stat->fd;
				// 新连接
				if(rsocket==listen_socket&&evs[i].events&EPOLLIN)
				{
					int client_socket = accept(listen_socket, (struct sockaddr *)&client_address, &len);
					if(client_socket>0)
					{
						printf("new customer is coming: %s, %d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
						// 添加事件
						connect_handle(client_socket);
					}
				}
				// http服务
				else
				{
					if(evs[i].events&EPOLLIN) // 读任务
					{
						do_http_request((ConnectStat *)evs[i].data.ptr);
					}
					else if(evs[i].events&EPOLLOUT) // 写任务
					{
						do_http_response((ConnectStat *)evs[i].data.ptr);
					}
				}

			}
		}
	}
	return 0;
}