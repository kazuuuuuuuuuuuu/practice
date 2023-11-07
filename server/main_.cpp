// server project 
// Virtual server -> setting from the router

#include<stdio.h>
#include<WinSock2.h>
#include<sys/types.h>
#include<sys/stat.h>
#pragma comment(lib, "WS2_32.lib")

// macro ->  more efficient than function calls
// #str c internal macro -> to print out the variable name
// __func__ -> current function
// __LINE__ -> on which line
#define PRINTF(str) printf("[%s - %d]"#str"=%s\n", __func__, __LINE__, str);

// print out error message
void error_die(const char* str)
{
	perror(str);
	exit(1);
}


// network initialization -> return a socket(server side)
int startup(unsigned short* port_num)
{
	// 1 initialization of network communication(windows)
	// MAKEWORD -> protocol-1.1
	WSADATA data;
	int ret = WSAStartup(MAKEWORD(1, 1), &data);
	if(ret!=0)
	{
		// fail
		error_die("WSAStartup failed\n");
	}

	// 2 create a socket
	// (socket type, datastream or dategram, protocol)
	int socket_ser = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(socket_ser==-1)
	{
		// fail
		error_die("socket creation failed\n");
	}

	// 3 Set Port Reuse
	int opt = 1;
	ret = setsockopt(socket_ser, SOL_SOCKET, SO_REUSEADDR, (const char*) &opt, sizeof(opt));
	if(ret==-1)
	{
		error_die("setting up for port reuse failed\n");
	}

	// configure ip address
	struct sockaddr_in addr_server;
	// set all members to 0
	memset(&addr_server, 0, sizeof(addr_server));
	addr_server.sin_family = PF_INET; // internet type
	addr_server.sin_port = htons(*port_num); // internet byte order conversion
	//Set IP address to INADDR_ Any, indicating binding to all available network interfaces 
	addr_server.sin_addr.s_addr = htonl(INADDR_ANY); // binding to my current ip


	// 4 bind socket with ip address
	ret = bind(socket_ser, (struct sockaddr*)&addr_server, sizeof(addr_server));
	if(ret<0)
	{
		error_die("bind failed\n");
	}

	// Dynamically allocate ports if input equals to 0
	int nameLen = sizeof(addr_server);
	if(*port_num==0)
	{
		ret = getsockname(socket_ser, (struct sockaddr*)&addr_server, &nameLen);
		if(ret<0)
		{
			error_die("dunamically allocating failed\n");
		}

		*port_num = addr_server.sin_port;
	}


	// 5 create a listening queue
	ret = listen(socket_ser, 5);
	if(ret<0)
	{
		error_die("listen failed");
	}

	return socket_ser;
}


// read a line from the specific connection socket and reserve in buff
// return the number of bytes readed
int get_line(int socket, char *buff, int size)
{
	char c = 0; //'\0'
	int i = 0;

	// the last position is for '\0'
	// \r\n indicates the end in http 
	while(i<size-1&&c!='\n')
	{
		// read byte by byte (char by char)
		int n = recv(socket, &c, 1, 0);
		if(n>0)
		{
			if(c=='\r')
			{
				// MSG_PEEK read a char but dont delete it from the input buffer
				n = recv(socket, &c, 1, MSG_PEEK);
				if(n>0 && c == '\n')
				{
					recv(socket, &c, 1, 0);
				}
				else
				{
					c = '\n';
				}
			}
			buff[i] = c;
			i ++;
		}
		else
		{
			c = '\n'; // break;
		}
	}

	buff[i] = 0; // the string end in '\0'
	return i;
}

void unimplement(int client)
{
	// return a message to the specified connection socket that the method has not been implemented yet
}

// 404 page
void not_found(int client)
{
	// return a message that the page requested was not found
	printf("some get there -> 404 not found\n");

	// send the header of the respond
	char buff[1024];
	// based on the http protocol
	// first line -> protocol, status code
	strcpy(buff, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buff, strlen(buff), 0);

	// second line -> information about server
	strcpy(buff, "Server: Kazu/0.1\r\n");
	send(client, buff, strlen(buff), 0);

	// third line -> type of the content
	strcpy(buff, "Content-type:text/html\n");
	send(client, buff, strlen(buff), 0);	

	strcpy(buff, "\r\n");
	send(client, buff, strlen(buff), 0);	

	// send the content of the 404 page 
	// \ -> continuation character
	strcpy(buff, 
	"<HTML>\
		<TITLE>NOT FOUND</TITLE>\
		<BODY>\
			<H2>The resource is unavailable.</H2>\
			<img src=\"404.png\" />\
		</BODY>\
	</HTML>");
	send(client, buff, strlen(buff), 0);	
	
}

// get the type of file user requested -> text or image
// respond header needed the information to fill out
const char* getHeadType(const char* fileName)
{
	// default
	const char *ret = "text/html";
	// check out the suffix
	const char *p = strrchr(fileName, '.');
	if(!p) return ret;

	p++;
	if(!strcmp(p, "css")) ret = "text/css";
	else if(!strcmp(p, "jpg")) ret = "image/jpeg";
	else if(!strcmp(p, "png")) ret = "image/png";
	else if(!strcmp(p, "js")) ret = "application/x-javascript";

	return ret;
}

// response header
void headers(int client, const char* type)
{
	// send the header of the respond
	char buff[1024];
	// based on the http protocol
	// first line -> protocol, status code
	strcpy(buff, "HTTP/1.0 200 OK\r\n");
	send(client, buff, strlen(buff), 0);

	// second line -> information about server
	strcpy(buff, "Server: Kazu/0.1\r\n");
	send(client, buff, strlen(buff), 0);

	// third line -> type of the content
	char buf[1024];
	sprintf(buf, "Content-type: %s\r\n", type);
	send(client, buf, strlen(buf), 0);	

	strcpy(buff, "\r\n");
	send(client, buff, strlen(buff), 0);	

}

void cat(int client, FILE *resource)
{
	// the maximum amount of bytes sent at once
	char buff[4096];
	int count = 0;
	
	while(1)
	{
		// Read 1 byte(unit size) at a time, read 4096 times
		int ret = fread(buff, sizeof(char), sizeof(buff), resource);
		if(ret<=0)
		{
			break;
		}
		send(client, buff, ret, 0);
		count += ret;		
	}

	printf("the total amount of bytes sent = %d\n", count);

}

// send the static resource
// favicon.ico is the default resource the browser requested ->  can be escaped -> modify the html file
void server_file(int client, const char *fileName)
{
	int num_chars = 1;
	char buff[1024];
	// before send file, clean up the data from the user request header 
	while(num_chars>0 && strcmp(buff, "\n"))
	{
		num_chars = get_line(client, buff, sizeof(buff));
		if(strcmp(buff, "\n"))
			PRINTF(buff);
	}


	// the type FILE -> a structure including some information(entail file descriptor) about that file 
	FILE *resource = NULL;
	if(strcmp(fileName, "htdocs/index.html")==0)
	{
		// open as a text file
		resource = fopen(fileName, "r");
	}
	else
	{
		// as a binary file(music, image, etc)
		resource = fopen(fileName, "rb");
	}

	if(resource==NULL)
	{
		not_found(client);
	}
	else
	{
		// send resource to the user
		// http header
		headers(client, getHeadType(fileName));

		// send the resource requested
		cat(client, resource);

		printf("the transaction ends\n");
	}
	fclose(resource);
}



// thread funcion -> to serve  
// (connection socket)
DWORD WINAPI accept_request(LPVOID arg)
{
	char buff[1024]; // 1K

	int client = (SOCKET) arg; // the client connection socket

	// read a line from a user
	int num_chars = get_line(client, buff, sizeof(buff));
	PRINTF(buff);

	// parse the line readed
	// "GET / HTTP/1.1\n"
	char method[255];
	int j = 0, i = 0;
	while(!isspace(buff[j]) && i<sizeof(method)-1 && j<sizeof(buff))
	{
		method[i] = buff[j];
		i ++;
		j ++;
	}

	method[i] = '\0';
	PRINTF(method);

	// check if the method of request is supported by the server 
	if(_stricmp(method, "GET") && _stricmp(method, "POST"))
	{
		// return an error prompt page to the browser
		unimplement(client);
		return 0;
	}

	// parse the path of the resource
	// www.rock.com/abc/test.html
	// "GET /abc/test.html HTTP/1.1\n"
	char url[255]; 
	i = 0;

	// skip spaces
	while(isspace(buff[j]) && j<sizeof(buff))
	{
		j ++;
	}

	// read
	while(!isspace(buff[j]) && i<sizeof(url)-1 && j<sizeof(buff))
	{
		url[i] = buff[j];
		i ++;
		j ++;
	}

	url[i] = '\0';
	PRINTF(url);

	
	// in general, the intact path = htdocs + url
	// htdocs -> Path to local resources folder -> customized name 
	char path[512] = "";
	sprintf(path, "htdocs%s", url);
	// url -> / -> by default, index.html
	if(path[strlen(path)-1]=='/')
	{
		strcat(path, "index.html");
	}
	PRINTF(path);
	// url -> /networks -> end with a directory name -> append index.html -> networks/index.html 
	
	// this structure is used to store the information about a file (store its attributes)
	struct stat status;
	
	// the resource the user is asking for does not exist
	if(stat(path, &status)==-1)
	{
		// clean up the user request data
		while(num_chars>0&&strcmp(buff, "\n"))
		{
			num_chars = get_line(client, buff, sizeof(buff));
		}
		
		not_found(client);
	}
	else
	{
		// the rusult of the bit operation is a file type -> if it is a directory
		if((status.st_mode & S_IFMT) == S_IFDIR)
		{
			strcat(path, "/index.html");
		}

		server_file(client, path);

	}

	closesocket(client);

	
	return 0;
}


int main()
{
	unsigned short port = 1024;
	int socket_ser = startup(&port);
	printf("httpd service has started, listening on %d port...\n", port);

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);

	while(1)
	{
		// block until get someone access
		int client_sock = accept(socket_ser, (struct sockaddr*) &client_addr, &client_addr_len);
	
		if(client_sock==-1)
		{
			error_die("accept failed\n");
		}

		// Opening up new threads -> offer service
		// thread - a execuation units that runs independently within a process
		// thread number
		DWORD threadId = 0;
		// ( , , thread funcion, connection socket)
		CreateThread(0, 0, accept_request, (void *) client_sock, 0, &threadId);


		
	}		

	closesocket(socket_ser);
	return 0;
}
