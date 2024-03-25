#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#define BUFF_SIZE 1024
// udp 加一般使用的阻塞型io
int main(int argc, char const *argv[])
{
	int server_sockfd;
	int ret;
	int recv_len;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(struct sockaddr_in);

	// 1 create a socket
	server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	// 2 set the server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(9000);	

	// 3 bind the socket to that address
	ret = bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret==-1)
	{
		perror("bind");
		exit(1);
	}

	while(1)
	{
		printf("waiting..\n");
		// 4 receive
		recv_len = recvfrom(server_sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr, &client_addr_len);
		if(recv_len<0)
		{
			perror("recvfrom");
			exit(1);
		}
		printf("received: %s\n", buff);
	}

	close(server_sockfd);
	return 0;
}