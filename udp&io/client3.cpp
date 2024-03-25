#include <sys/types.h> 
#include <sys/socket.h> 
#include <stdio.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char const *argv[])
{
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(9000);

	socklen_t server_len = sizeof(server_address);	

	int result = connect(client_socket, (struct sockaddr *)&server_address, server_len);
	if(result==-1)
	{
		perror("connect");
		exit(1);		
	}
	char ch = 'a';
	// 1st
	write(client_socket, &ch, 1);
	read(client_socket, &ch, 1);
	printf("the first time read from server: %c\n", ch);
	sleep(5);

	// 2st
	write(client_socket, &ch, 1);
	read(client_socket, &ch, 1);
	printf("the second time read from server: %c\n", ch);
	
	close(client_socket);
	return 0;
}