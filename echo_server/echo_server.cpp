#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

static const uint16_t SERVER_PORT = 6666;
int main(int argc, char const *argv[])
{
	// 1 set the socket
	int socket_ = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM -> tcp

	// 2 set the ip address and the port number
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(sockaddr_in));
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // listen on that ip address -> INADDR_ANY indicates any local ip available
	server_addr.sin_port = htons(SERVER_PORT); // listen on that port number

	// 3 bind the socket with that address 
	bind(socket_, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(sockaddr_in));
	
	// 4 listen on the socket specified
	listen(socket_, 64); // 64 -> maximum number of clients arriving
	
	while(1)
	{
		// store the address info of the clients
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(sockaddr_in);
		// 5 waiting for the clients
		printf("waiting for the clients\n");
		int connected_socket = accept(socket_, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);

		char client_ip[64];
		printf("a client is comming\n");
		printf("clinet ip address: %s\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)));
		printf("clinet port: %d\n", ntohs(client_addr.sin_port));

		// 6 read from the client
		char buf[256];
		int len = read(connected_socket, buf, sizeof(buf)-1);
		buf[len] = '\0';
		printf("receive: %s, len: %d.\n", buf, len);
		for(int i = 0; buf[i] != '\0'; i++) 
		{
        	buf[i] = toupper(buf[i]);
    	}
    	// 7 echo back to the client
		len = write(connected_socket, buf, len);
		printf("write: %s, len_write: %d.\n", buf, len);
		close(connected_socket);
	}
	close(socket_);
	return 0;
}