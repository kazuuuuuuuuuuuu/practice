#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "rio_lib.h"
#include <fcntl.h>
#include <pthread.h>

static int debug = 1;
static const uint16_t SERVER_PORT = 80;

void *handle_request(void *p_connected_socket);

void http_respond(int connected_socket, const char *path);
int header(int connected_socket, int fd);
void html_content(int connected_socket, int fd);

void http_respond2(int connected_socket); // serve html with hard coding

void inner_error(int connected_socket); // 500
void not_found(int connected_socket); // 404
void unimplemented(int connected_socket); //501
void bad_request(int connected_socket); // 400


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

		// print out client ip address
		char client_ip[64];
		printf("clinet ip address: %s\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)));
		printf("clinet port: %d\n", ntohs(client_addr.sin_port));

		// 6 create a thread to handle HTTP requests
		pthread_t pid;
		int *p_argu = new int;
		*p_argu = connected_socket;
		pthread_create(&pid, NULL, handle_request, reinterpret_cast<void *>(p_argu));
	}
	close(socket_);
	return 0;
}

void *handle_request(void *p_connected_socket)
{
	int connected_socket = * (reinterpret_cast<int *>(p_connected_socket));
	// 1 set the internal buffer for the function rio_readlineb()
	rio_t rp;
	rio_readinitb(&rp, connected_socket);

	// 2 read and parse the first line of request
	char buf[256];
	int len;
	len = rio_readlineb(&rp, buf, sizeof(buf));
	printf("the length of the first len:%d\n", len);
	if(len>2)
	{
		char method[64];
		char url[256];
		int i=0, j=0;
		// obtain method
		while(!isspace(buf[j])&&i<sizeof(method)-1)
		{
			method[i] = buf[j];
			i ++;
			j ++;
		}
		method[i] = '\0';
		if(debug) printf("request method: %s\n", method);
		
		// only deal with the "get" method and then obtain the url
		if(strcasecmp(method, "get")==0) // strcasecmp performs a case-insensitive string comparison.
		{
			// skip the blank space
			while(isspace(buf[j]))
			{
				j ++;
			}
			i = 0;
			// get url
			while(!isspace(buf[j])&&i<sizeof(url)-1)
			{
				url[i] = buf[j];
				i ++;
				j ++;
			}
			url[i] = '\0';
			if(debug) printf("request url: %s\n", url);
			// deal with '?'
			char *pos = strchr(url, '?'); // return a pointer pointing to the '?'
			if(pos)
			{
				*pos = '\0';
				printf("after modifications: %s\n", url);
			}
			// get the file path in the server
			char path[256];
			sprintf(path, "./html_docs%s", url);
			if(debug) printf("the file path: %s\n", path);
			// read the remaining lines
			do
			{
				len = rio_readlineb(&rp, buf, sizeof(buf));
				if(debug) printf("read line: %s", buf);
			}while(len>2); // the last line is "/r/n"
			// 3 serve the html file
			struct stat st;
			if(stat(path, &st)<0) // the file doesn't exist
			{
				fprintf(stderr, "stat %s failed: %s\n", path, strerror(errno));
				not_found(connected_socket);
			}
			else // the file exists
			{
				if(S_ISDIR(st.st_mode)) // if it is a directory
				{
					strcat(path, "/index.html");
					if(debug) printf("the final file path: %s\n", path);
				}
				http_respond(connected_socket, path);
			}
			
		}
		else // the method is not "get"
		{
			fprintf(stderr, "warning! other request method: %s\n", method);
			// read the remaining lines
			do
			{
				len = rio_readlineb(&rp, buf, sizeof(buf));
				if(debug) printf("read line: %s", buf);
			}while(len>2); // the last line is "/r/n"
			// 501 not implemented
			unimplemented(connected_socket);
		}
	}
	else // The format of the request is incorrect
	{
		// 400 bad request
		bad_request(connected_socket);
	}
	close(connected_socket);
	if(debug) printf("handle_request is over\n");
	delete(reinterpret_cast<int *>(p_connected_socket));
	return NULL;
}

void bad_request(int connected_socket)
{
	// 1 send the header 
	char buff[1024];

	strcpy(buff, "HTTP/1.0 400 Bad Request\r\n"); // first line -> protocol, status code
	int len = rio_writen(connected_socket, buff, strlen(buff));
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "Server: Kazu/0.1\r\n"); // second line -> information about server
	len = rio_writen(connected_socket, buff, strlen(buff));
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "Content-type: text/html\r\n"); // third line -> type of the content
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "\r\n"); // the end of the header
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);

	// 2 send the content of the 501 page ( '\' -> continuation character)
	strcpy(buff, 
	"<html lang=\"en\">\n\
	<HTML>\n\
		<TITLE>Bad Request</TITLE>\n\
		<BODY>\n\
			<H2> Bad Request. 400</H2>\n\
		</BODY>\n\
	</HTML>\n");
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);	
}


void unimplemented(int connected_socket)
{
	// 1 send the header 
	char buff[1024];

	strcpy(buff, "HTTP/1.0 501 Method Not Implemented\r\n"); // first line -> protocol, status code
	int len = rio_writen(connected_socket, buff, strlen(buff));
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "Server: Kazu/0.1\r\n"); // second line -> information about server
	len = rio_writen(connected_socket, buff, strlen(buff));
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "Content-type: text/html\r\n"); // third line -> type of the content
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "\r\n"); // the end of the header
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);

	// 2 send the content of the 501 page ( '\' -> continuation character)
	strcpy(buff, 
	"<html lang=\"en\">\n\
	<HTML>\n\
		<TITLE>Method Not Implemented</TITLE>\n\
		<BODY>\n\
			<H2> The Method Not Implemented. 501</H2>\n\
		</BODY>\n\
	</HTML>\n");
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);
}

void not_found(int connected_socket)
{
	// 1 send the header 
	char buff[1024];

	strcpy(buff, "HTTP/1.0 404 NOT FOUND\r\n"); // first line -> protocol, status code
	int len = rio_writen(connected_socket, buff, strlen(buff));
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "Server: Kazu/0.1\r\n"); // second line -> information about server
	len = rio_writen(connected_socket, buff, strlen(buff));
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "Content-type: text/html\r\n"); // third line -> type of the content
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "\r\n"); // the end of the header
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);

	// 2 send the content of the 404 page ( '\' -> continuation character)
	strcpy(buff, 
	"<html lang=\"en\">\n\
	<HTML>\n\
		<TITLE>NOT FOUND</TITLE>\n\
		<BODY>\n\
			<H2>404 The resource is unavailable. 404</H2>\n\
		</BODY>\n\
	</HTML>\n");
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);
}

void inner_error(int connected_socket)
{
	// 1 send the header 
	char buff[1024];

	strcpy(buff, "HTTP/1.0 500 Internal Server Error\r\n"); // first line -> protocol, status code
	int len = rio_writen(connected_socket, buff, strlen(buff));
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "Server: Kazu/0.1\r\n"); // second line -> information about server
	len = rio_writen(connected_socket, buff, strlen(buff));
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "Content-type: text/html\r\n"); // third line -> type of the content
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);

	strcpy(buff, "\r\n"); // the end of the header
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);

	// 2 send the content of the 500 page ( '\' -> continuation character)
	strcpy(buff, 
	"<html lang=\"en\">\n\
	<head>\n\
    <meta charset=\"UTF-8\">\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
    <title>500 Internal Server Error</title>\n\
	</head>\n\
	<body>\n\
    <h1>500 Internal Server Error</h1>\n\
    <p>Sorry, something went wrong on our server. We are working to fix the issue. Please try again later.</p>\n\
	</body>\n\
	</html>\n");
	len = rio_writen(connected_socket, buff, strlen(buff));	
	if(debug) printf("write[%d]: %s", len, buff);
}


void http_respond(int connected_socket, const char *path)
{
	// 1 open the target file
	int fd = open(path, O_RDWR);
	
	if(fd<0)
	{
		fprintf(stderr, "http_respond %s failed: %s\n", path, strerror(errno));
		not_found(connected_socket);
		return;
	}
	

	// 2 send the header
	int ret = header(connected_socket, fd);
	if(ret==-1)
	{
		close(fd);
		return;
	}

	// 3 send the content
	html_content(connected_socket, fd);

	close(fd);
}

int header(int connected_socket, int fd)
{
	char buff[1024];
	char temp[128];

	// prepare the header
	strcpy(buff, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: Close\r\n"); 
	struct stat st;
	if(fstat(fd, &st)<0)
	{
		inner_error(connected_socket);
		return -1;
	}
	sprintf(temp, "Content-Length : %ld\r\n\r\n", st.st_size);
	strcat(buff, temp);

	// send it to the client
	int len = rio_writen(connected_socket, buff, strlen(buff));
	if(len<0)
	{
		fprintf(stderr, "header failed: %s\n", strerror(errno));
		return -1;
	}
	if(debug) printf("write[%d]: %s", len, buff);
	return 0;
}

void html_content(int connected_socket, int fd)
{
	// 0 set the internel buffer
	rio_t rp;
	rio_readinitb(&rp, fd);

	// 1 read by line and send it to the client
	int total = 0;
	char buff[128];
	while(rio_readlineb(&rp, buff, sizeof(buff))>0) // read a line
	{
		int len = rio_writen(connected_socket, buff, strlen(buff)); // write a line
		if(len<0)
		{
			fprintf(stderr, "html_content->rio_writen failed: %s\n", strerror(errno));
			break;
		}
		if(debug) printf("write[%d]: %s", len, buff);
		total += len;
	}
	printf("html file size: %d\n", total);
}

void http_respond2(int connected_socket)
{
	// 1 prepare the content to be sent
	const char *header = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: Close\r\n";
	const char *content = "<html lang=\"en\">\n\
<head>\n\
    <meta charset=\"UTF-8\">\n\
    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
    <title>Welcome Kazu</title>\n\
</head>\n\
<body>\n\
    <p>Welcome Kazu</p>\n\
</body>\n\
</html>\n";

	if(debug) printf("http_respond is running\n");
	// 2 send the header
	int len = rio_writen(connected_socket, header, strlen(header));
	if(debug) printf("write[%d]: %s", len, header);
	// 3 obtain and send the parameter of content length (part if the header)
	int content_len = strlen(content);
	char buf[64];
	len = sprintf(buf,"Content-Length : %d\r\n\r\n", content_len);
	len = rio_writen(connected_socket, buf, len);
	if(debug) printf("write[%d]: %s", len, buf);
	// 4 send the html
	len = rio_writen(connected_socket, content, content_len);
	if(debug) printf("write[%d]: %s", len, content);
}