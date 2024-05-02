#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

static const uint16_t SERVER_PORT = 6666;
#define SERVER_IP "192.168.226.129"

int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		fprintf(stderr, "usage: ./echo_client message\n");
	 	exit(1);
	}
	const char *message = argv[1];
	printf("  message: %s\n", message);

	// 1 set the socket
	int clientfd = socket(AF_INET, SOCK_STREAM, 0);

	// 2 set the server addr
	printf("set the server addr...\n");
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr); // 127.0.0.1 -> Local IP address
	server_addr.sin_port = htons(SERVER_PORT); // the port number
	
	// 3 connect
	printf("connecting...\n");
	if(connect(clientfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr))==-1)
	{
		fprintf(stderr, "error! no connection\n");
		exit(-1);
	}
	printf("has been connected\n");
	
	// 4 write
	printf("writing...\n");
	int ret = write(clientfd, message, strlen(message));
	fprintf(stderr, "writing is over\n");
	
	// 5 receive the echo
	char buf[64];
	int n = read(clientfd, buf, sizeof(buf)-1);
	if(n>0)
	{
		buf[n] = '\0';
		printf("echo back: %s, len: %d\n",  buf, n);
	}
	else
	{
		perror("Error");
	}
	
	printf("finished\n");
	close(clientfd);
	exit(0);
}