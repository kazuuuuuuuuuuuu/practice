#include <stdio.h>
#include <WinSock2.h> // provides tools for network programming on windows
#include <sys/types.h> // provides basic data types used in system calss (pit_t, size_t...)
#include <sys/stat.h> // provides declarations for functions used to get files information

#pragma comment(lib, "WS2_32.lib")

#define PORT_NUM 1024;

// 0 error
void error_die(const char* str)
{
	perror(str);
	exit(1);
}


// 1* take a port number -> return a listening socket
int open_listening_socket(unsigned short* port)
{
	int ret;

	// 1 initialize the network 
	WSADATA data;
	ret = WSAStartup(MAKEWORD(1, 1), &data); // MAKEWORD(1, 1) -> using Winsock 1.1 version
	if(ret!=0)
		error_die("WSAStartup failed\n");

	// 2 create a listening socket
	int listening_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // (socket type, datastream or datagram, protocol);
	if(listening_socket==-1)
		error_die("socket failed\n");

	// 3 set port reusability
	int opt = 1;
	ret = setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &opt, sizeof(opt));
	if(ret==-1)
		error_die("setsockopt failed\n");

	// 4 configure ip address
	struct sockaddr_in addr_server;
	memset(&addr_server, 0, sizeof(addr_server)); // zero out
	addr_server.sin_family = PF_INET; // internet type;
	addr_server.sin_port = htons(*port); // set port number using the internet byte order conversion
	addr_server.sin_addr.s_addr = htonl(INADDR_ANY); // binding to my current ip using the internet byte order conversion

	// 5 bind listening socket to that ip address
	ret = bind(listening_socket, (struct sockaddr*) &addr_server, sizeof(addr_server));
	if(ret<0)
		error_die("bind failed\n");

	
	// 6 dynamically allocate a port when the port number equals to 0
	int addr_length = sizeof(addr_server);
	if(*port==0)
	{
		ret = getsockname(listening_socket, (struct sockaddr*) &addr_server, &addr_length);
		if(ret<0)
			error_die("getsockname failed\n");
		*port = addr_server.sin_port;
	} 
	

	// 7 create a listening queue
	ret = listen(listening_socket, 5);
	if(ret<0)
		error_die("listen failed\n");

	return listening_socket;
}

// read a line from client
int get_line(int client, char buff[], int size)
{
	char c = 'a';
	int i = 0;
	while(i<size-1&&c!='\n')
	{
		// read from client byte by byte
		int n = recv(client, &c, 1, 0);
		if(n>0)
		{
			if(c=='\r')
			{
				continue;
			}
			buff[i] = c;
			i ++;
		}
		else
		{
			break;
		}
	}
	buff[i] = '\0';
	return i;
}

// parse the user request 
void parse_request(int client, char method[], char url[], char form[])
{	
	int i = 0;
	int j = 0;
	int n;
	char buff[1024];
	
	// 1 get the first line
	n = get_line(client, buff, sizeof(buff));
	printf(buff);

	// 2 get the methond from the first line
	while(isspace(buff[j]))
	{
		j ++; // skip spaces
	}
	while(!isspace(buff[j]) && j<sizeof(buff))
	{
		method[i] = buff[j];
		i ++;
		j ++;
	}
	method[i] = '\0';

	// 3 get the url from the first line
	while(isspace(buff[j]))
	{
		j ++; // skip spaces
	}
	i = 0;
	while(!isspace(buff[j]) && j<sizeof(buff))
	{
		url[i] = buff[j];
		i ++;
		j ++;
	}
	url[i] = '\0';

	// 4 exhaust remaining request lines
	int limit = 0;
	while(n>0&&strcmp(buff, "\n"))
	{
		n = get_line(client, buff, sizeof(buff));
		printf(buff);
		buff[14] = '\0';
		if(!strcmp(buff, "Content-Length"))
		{
			int i = 15;
			int j = 0;
			char char_limit[32];
			while(buff[i]!='\n')
			{
				char_limit[j] = buff[i];
				i ++;
				j ++;
			}
			char_limit[j] = '\0';
			limit = atoi(char_limit);
			printf("Content-Length: %d\n", limit);
		}
	}

	// 5 find the extra form line when the post method is coming
	i = 0;
	while(i<limit)
	{
		char c;
		recv(client, &c, 1, 0);
		form[i] = c;
		i ++; 
	}
	form[i] = '\0';

	printf("---------method:%s---------\n", method);	
	printf("---------url:%s---------\n", url);	
	printf("---------form:%s---------\n", form);		
}

void not_found(int client)
{
	// return a message that the page requested was not found
	printf("someone gets there -> 404 not found\n");

	// 1 send the header of the respond based on the http protocol
	char buff[1024];

	strcpy(buff, "HTTP/1.0 404 NOT FOUND\r\n"); // first line -> protocol, status code
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "Server: Kazu/0.1\r\n"); // second line -> information about server
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "Content-type:text/html\n"); // third line -> type of the content
	send(client, buff, strlen(buff), 0);	

	strcpy(buff, "\r\n"); // the end of the header
	send(client, buff, strlen(buff), 0);	

	// 2 send the content of the 404 page ( '\' -> continuation character)
	strcpy(buff, 
	"<HTML>\
		<TITLE>NOT FOUND</TITLE>\
		<BODY>\
			<H2>404 The resource is unavailable. 404</H2>\
			<img src=\"404.png\" />\
		</BODY>\
	</HTML>");
	send(client, buff, strlen(buff), 0);	
}

// determind the file type used in header
const char *getheadtype(const char* path)
{
	const char *ret = "text/html"; // default
	const char* suffix = strrchr(path, '.'); // find the pointer of the character '.'
	if(suffix==NULL)
		return ret;

	suffix ++; // point to the next position
	if(!strcmp(suffix, "css")) ret = "text/css";
	else if(!strcmp(suffix, "jpg")) ret = "image/jepg";
	else if(!strcmp(suffix, "png")) ret = "image/png";
	else if(!strcmp(suffix, "js")) ret = "application/x-javascript";

	return ret;
}

void header(int client, const char* header_type)
{
	char buff[1024];

	strcpy(buff, "HTTP/1.0 200 OK\r\n"); // first line -> protocol, status code
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "Server: Kazu/0.1\r\n"); // second line -> information about server
	send(client, buff, strlen(buff), 0);

	sprintf(buff, "Content-type:%s\n", header_type); // third line -> type of the content
	send(client, buff, strlen(buff), 0);	

	strcpy(buff, "\r\n"); // the end of the header
	send(client, buff, strlen(buff), 0);	
}

void transfer(int client, FILE *resource)
{
	char buff[4096]; // according to specification, the maximum size can be sent at once
	int count = 0;
	while(1)
	{
		int ret = fread(buff, sizeof(char), sizeof(buff), resource);
		if(ret<=0)
		{
			break;
		}
		send(client, buff, ret, 0);
		count += ret;
	}
	printf("have sent %d bytes...\n", count);
}

// send static files
void send_file(int client, const char* path)
{
	// 1 open file
	FILE *resource = NULL; // the file handle
	if(strcmp(path, "htdocs/index.html")==0)
	{
		resource = fopen(path, "r"); // open as a text file
	}
	else
	{
		resource = fopen(path, "rb"); // open as a binary file
	}
	if(resource==NULL)
	{
		fclose(resource);
		error_die("send_file failed\n");
	}

	// 2 determind the file type used in header
	const char *header_type = getheadtype(path);

	if(strcmp)
	// 3 send header
	header(client, header_type);

	// 4 send content of the file 
	transfer(client, resource);

	fclose(resource);
}

// execute cgi
void execute_cgi(int client, const char* path, const char *form)
{
	printf("execute_cgi is running...\n");
	
	// 1 create two pipes
	HANDLE cgi_input[2]; // the read end and the write end
	HANDLE cgi_output[2];

	SECURITY_ATTRIBUTES pipe_para; // set parameters
	pipe_para.nLength = sizeof(pipe_para);
	pipe_para.bInheritHandle = TRUE;
	pipe_para.lpSecurityDescriptor = NULL;

	bool pipe_create;
	pipe_create = CreatePipe(&cgi_input[0], &cgi_input[1], &pipe_para, 0);
	if(!pipe_create)
	{
		error_die("CreatePipe 1 failed\n");
	}
	pipe_create = CreatePipe(&cgi_output[0], &cgi_output[1], &pipe_para, 0);
	if(!pipe_create)
	{
		error_die("CreatePipe 2 failed\n");
	}
	printf("pipes have been created successfully...\n");

	// 2 create another process to run the cgi
	PROCESS_INFORMATION process_info {0}; // store the information of the new process
	STARTUPINFO startup_info {0}; // the information of how to display the new process

	startup_info.cb = sizeof(startup_info);
	startup_info.hStdInput = (HANDLE) cgi_input[0]; // redirect stdin to cgi_input[0] - the read end
	startup_info.hStdOutput = (HANDLE) cgi_output[1]; // redirect stdout to cgi_output[1] - the write end
	startup_info.hStdError = (HANDLE) cgi_output[1]; // redirect stderr to cgi_output[1] - write end
	startup_info.wShowWindow = SW_HIDE;
	startup_info.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	bool process_create;
	char *cgi_path = (char *) path;
	process_create = CreateProcess(NULL, cgi_path, NULL, NULL, TRUE, NULL, NULL, NULL, &startup_info, &process_info);
	if(!process_create)
	{
		error_die("CreateProcess failed\n");
	}

	CloseHandle(cgi_input[0]); // those end points are used in the child process, close them in the parent process
	CloseHandle(cgi_output[1]); // those end points are used in the child process, close them in the parent process

	// 3 send data to the cgi
	char w;
	DWORD write_info;
	int limit = strlen(form);
	for(int i=0;i<limit;i++)
	{	
		w = form[i];
		WriteFile(cgi_input[1], &w, 1, &write_info, NULL);
	}
	w = '\n';
	WriteFile(cgi_input[1], &w, 1, &write_info, NULL);
	printf("writing is over...\n");
	
	// 4 receive data from the cgi
	WaitForSingleObject(process_info.hProcess, INFINITE); // wait for the child process to end
	char r;
	DWORD read_info;
	printf("begin to read...\n");
	FILE *file = fopen("test.html", "w");
	while(ReadFile(cgi_output[0], &r, 1, &read_info, NULL))
	{
        fputc(r, file);
    }
    fclose(file);

    // 5 send it to the user
    header(client, "text/html");
    FILE *resource = fopen("test.html", "r");
    transfer(client, resource);
    fclose(resource);

	CloseHandle(cgi_input[1]);
	CloseHandle(cgi_output[0]);
	CloseHandle(process_info.hProcess); // close the handle of the process new created -> avoid resource leaking
	CloseHandle(process_info.hThread); // close the handle of the main thread of the process new created -> avoid resource leaking

	printf("\nexecute_cgi is finished...\n");
}

// 3* create a worker thread to serve the client
DWORD WINAPI handleclient(LPVOID arg)
{
	int client = (SOCKET) arg;
	// 1 parse the user request
	char method[255];
	char url[255]; 
	char form[255];
	parse_request(client, method, url, form);
	

	// 2 derive the file path from the url
	char path[512] = "";
	sprintf(path, "htdocs%s", url);
	if(path[strlen(path)-1]=='/')
	{
		strcat(path, "index.html");
	}
	printf("--------path:%s---------\n", path);

	// 3 check if the file exists 
	struct stat status;
	if(stat(path, &status)==-1)
	{
		// 1 not found
		printf("!!!can not find the file at the stat stage!!!\n");
		not_found(client); 
	}
	else
	{
		// 2 send files or deal with cgi
		if(!strcmp(method, "GET"))
			send_file(client, path); 
		else if(!strcmp(method, "POST"))
			execute_cgi(client, path, form);
		else
		{
			printf("!!!failed at the stage of choosing the method!!!\n");
			not_found(client);
		}
	}

	closesocket(client); 
}

int main(int argc, char const *argv[])
{
	unsigned short port = PORT_NUM;
	// 1* take a port number -> return a listening socket
	int listening_socket = open_listening_socket(&port);
	printf("the server is listening on %d...\n", port);

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);

	while(1)
	{
		// 2* server is blocked waiting for connection
		int connect_socket = accept(listening_socket, (struct sockaddr*) &client_addr, &client_addr_len);
		if(connect_socket==-1)
			error_die("accept failed\n");

		DWORD threadid;
		// 3* create a thread to serve the client with its request
		CreateThread(0, 0, handleclient, (void *)connect_socket, 0, &threadid);
	}

	closesocket(listening_socket);
	return 0;
}
