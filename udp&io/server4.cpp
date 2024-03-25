#include <sys/types.h> 
#include <sys/socket.h> 
#include <stdio.h> 
#include <netinet/in.h> 
#include <sys/time.h> 
#include <sys/ioctl.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <poll.h>
#include <string.h>


#define MAX_FD 8192
// POLL 监听的fd集合 每个fd一个结构体 
struct pollfd fds[MAX_FD]; // 用fd当作数组的下标 索引对应的结构体
int cur_max_fd = 1;

void set_cur_max_fd(int fd)
{
	if(fd>=cur_max_fd)
		cur_max_fd = fd + 1;	
}



// poll
int main(int argc, char const *argv[])
{
	// 1 create a socket
	int client_socket;
	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// 2 set the address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(9000);

	socklen_t server_len = sizeof(server_address);

	// 3 bind
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

	// 4 listen
	listen(server_sockfd, 5);
	
	// 加入监听集合
	fds[server_sockfd].fd = server_sockfd;
	fds[server_sockfd].events = POLLIN;
	fds[server_sockfd].revents = 0;
	set_cur_max_fd(server_sockfd);

	while(1)
	{
		char ch;
		printf("server is waiting..\n");

		// poll 只是用来监视 -> 有信号就唤醒 或者超时
		// 没有有文件描述符数量的限制 

		int result = poll(fds, cur_max_fd, 2000);
		if(result<0)
		{
			perror("select");
			exit(1);
		}
		if(result==0)
			continue;

		// poll只会通知集合中的元素有可用的 但不会说明哪个是可用的
		// 全部遍历一边
		for(int fd=0;fd<cur_max_fd;fd++)
		{
			if(fds[fd].revents)
			{
				if(fd==server_sockfd) // 客户端连接也是读事件
				{
					struct sockaddr_in client_address;
					socklen_t client_len = sizeof(client_address);
					client_socket = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
					// 将client socket加入监听集合
					fds[client_socket].fd = client_socket;
					fds[client_socket].events = POLLIN;
					fds[client_socket].revents = 0;
					set_cur_max_fd(client_socket);					
					printf("adding client on fd : %d\n", client_socket);
				}
				else if(fds[fd].revents&POLLIN)
				{
					int nread = read(fd, &ch, 1);

					if(nread==0) // 客户端关闭也是读事件
					{
						close(fd);
						memset(&fds[fd], 0, sizeof(struct pollfd)); // fds[client_socket].events = 0 取消监听的类型 就从监听集合中取消了
						printf("removing client on fd : %d\n", fd);
					}
					else
					{
						fds[client_socket].events = POLLOUT;
					}
				}
				else if(fds[fd].revents&POLLOUT)
				{
					sleep(5);
					printf("serving client on fd : %d\n", fd);
					ch ++;
					write(fd, &ch, 1);	
					fds[client_socket].events = POLLIN;
				}
			}
		}
	}

	return 0;
}