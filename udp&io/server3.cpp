#include <sys/types.h> 
#include <sys/socket.h> 
#include <stdio.h> 
#include <netinet/in.h> 
#include <sys/time.h> 
#include <sys/ioctl.h> 
#include <unistd.h> 
#include <stdlib.h>

// select
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
	
	fd_set readfds, testfds; // 监听的descriptor的集合
	FD_ZERO(&readfds);
	FD_SET(server_sockfd, &readfds);

	while(1)
	{
		testfds = readfds; // select会更改这个集合 所以要用另一个变量保存
		printf("server is waiting..\n");

		// select 只是用来监视 -> 有信号就唤醒
		// FD_SETSIZE 系统默认的最大文件描述符 -> 有文件描述符数量的限制 对比poll 

		int result = select(FD_SETSIZE, &testfds, (fd_set*)0, (fd_set*)0, (struct timeval*)0);
		if(result<1)
		{
			perror("select");
			exit(1);
		}

		// select只会通知集合中的元素有可用的 但不会说明哪个是可用的
		// 全部遍历一边
		for(int fd=0;fd<FD_SETSIZE;fd++)
		{
			if(FD_ISSET(fd, &testfds))
			{
				if(fd==server_sockfd) // 客户端连接也是读事件
				{
					struct sockaddr_in client_address;
					socklen_t client_len = sizeof(client_address);
					client_socket = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
					FD_SET(client_socket, &readfds);
					printf("adding client on fd : %d\n", client_socket);
				}
				else
				{
					int nread;
					ioctl(fd, FIONREAD, &nread); // 取得数据量

					if(nread==0) // 客户端关闭也是读事件
					{
						close(fd);
						FD_CLR(fd, &readfds);
						printf("removing client on fd : %d\n", fd);
					}
					else
					{
						char ch;
						read(fd, &ch, 1);
						sleep(5);
						printf("serving client on fd : %d\n", fd);
						ch ++;
						write(fd, &ch, 1);
					}
				}
			}
		}
	}

	return 0;
}