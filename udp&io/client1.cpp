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

int main(int argc, char const *argv[])
{
	int sockfd;
	struct sockaddr_in server_addr;
	char buff[BUFF_SIZE];

	// 1 create a socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); // SOCK_DGRAM -> UDP

	// 2 set the server address 
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(9000);

	// 3 send data to the server
	strcpy(buff, "hello world");
	(void) sendto(sockfd, buff, strlen(buff)+1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

	// 4 close socket
	close(sockfd);
	return 0;
}