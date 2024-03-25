#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

// 使用 非阻塞io using unblocked io

#define BUFF_SIZE 1024

static void str2up(char *str)
{
	while(*str!='\0')
	{
		if((*str)>='a'&&(*str)<='z')
		{
			*str = *str + ('A'-'a');
		}
		str ++;
	}
}

int main(int argc, char const *argv[])
{
	int server_sockfd;
	int ret;
	int recv_len;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(struct sockaddr_in);

	// 1 create a socket -> udp and unblocked io
	// server_sockfd = socket(AF_INET, SOCK_DGRAM|SOCK_NONBLOCK, 0);
	server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	// set unblocked io
	fcntl(server_sockfd, F_SETFL, fcntl(server_sockfd, F_GETFL, 0) | O_NONBLOCK);

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
		// 4 receive -> record the client address
		recv_len = recvfrom(server_sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr, &client_addr_len);
		if(recv_len<0)
		{
			// data is not ready -> retry
			if(errno==EAGAIN||errno==EWOULDBLOCK)
			{
				sleep(2);
				continue;
			}
			perror("recvfrom");
			exit(1);
		}
		printf("receive: %s, %d bytes\n", buff, recv_len);

		// 5 process and send to the client
		str2up(buff);
		ret = sendto(server_sockfd, buff, strlen(buff)+1, 0, (struct sockaddr *)&client_addr, client_addr_len);
	}

	close(server_sockfd);
	return 0;
}