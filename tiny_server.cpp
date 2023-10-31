// 迭代服务器 -> 这种类型的服务器一次一个地在客户端间迭代
// uses the GET method to serve static and dynamic content

// 必要的函数原型
int main(int argc, char const *argv[])
{
	int listenfd, connfd;
	char hostname[MAXLINE], port[MAXLINE];
	socklen_t clientlen;
	//通用套接字地址结构 -> 不受协议限制
	struct sockaddr_storage clientaddr;

	// check command-line args
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}

	// 打开监听描述符, 参数为端口号 (以字符串的格式)
	listenfd = Open_listenfd(argv[1]);
	while(1)
	{
		clientlen = sizeof(clientaddr);
		// 已连接描述符 -> 已建立和一个客户端的链接
		cnnfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		// 打印客户端信息
		Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		printf("Accepted connection from (%s, %s)\n", hostname, port);
		// 执行事务 -> 为客户端提供服务
		doit(connfd);
		Close(connfd);
	}
	return 0;
}

// doit函数处理HTTP事务
void doit(int fd)
{
	int is_static;
	// 储存文件信息的结构体
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	// cgi -> 一个标准 -> 服务器创建子进程运行cgi程序为客户端提供动态内容
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	// 一个http请求包括
	// 一个请求行 request line -> method URI version -> uri是相应的url的后缀，包括文件名和可选的参数
	// 跟随零个或更多个请求报头 request header
	// 再跟随一个空的文本行来终止报头列表
	// read request line and headers
	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	printf("Request headers \n");
	printf("%s", buf);
	sscanf(buf, "%s %s %s", method, uri, version); //按格式读取 中间用空格分隔
	// 不是get方法就返回
	if(strcasecmp(method, "GET"))
	{
		clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
		return;
	}
	read_requesthdrs(&rio);

	// parse URI from GET request
	is_static = parse_uri(uri, filename, cgiargs);
	if(stat(filename, &sbuf)<0)
	{
		clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
		return;
	}

	// serve static content
	if(is_static)
	{
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
		{
			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	// serve dynamic content
	else
	{
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
		{
			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
			return;
		}
		serve_dynamic(fd, filename, cgiargs);
	}
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXBUF];

	// build the http response body
	sprintf(body, "<html><title>TinyError</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

	// print the HTTP reponse
	sprintf(buf, "HTTP\1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	Rio_writen(fd, buf, strlen(buf));
	Rio_writen(fd, body, strlen(body));
}

// just read and ignore request header
void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];
	
	Rio_readlineb(rp, buf, MAXLINE);
	// 空文本行 仅由\r\n组成
	while(strcmp(buf, "\r\n"))
	{
		// rp为缓冲区 -> 由此读到buf中
		Rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);
	}
	return;
}


int parse_uri(char *uri, char *filename, char *cgiargs)
{
	char *ptr;

	// static content
	if(!strstr(uri, "cgi-bin"))
	{
		strcpy(cgiargs, "");
		// linux relative path name 
		strcpy(filename, ".");
		strcat(filename, uri);
		if(uri[strlen(uri)-1] == '/')
			strcat(filename, "home.html");
		return 1;
	}
	// dynamic content
	else
	{
		ptr = index(uri, '?');
		if(ptr)
		{
			strcpy(cgiargs, ptr+1);
			*ptr = '\0';
		}
		else
			strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}
}

void serve_static(int fd, char *filename, int filesize)
{
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXLINE];

	// send response headers to client
	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 ok\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sConnection: close\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	Rio_writen(fd, buf, strlen(buf));
	printf("Response header: \n");
	printf("%s", buf);

	// send response body to client
	srcfd = Open(filename, O_RDONLY, 0);
	// 映射到虚拟内存空间 ——> 由descriptor srcp指示的文件  大小为filesize字节
	// Mmap 返回指向虚拟内存空间开始处的指针
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	// 关闭file descriptor
	Clsoe(srcfd);
	// 复制到已连接操作符
	Rio_writen(fd, srcp, filesize);
	// 释放虚拟内存
	Munmap(srcp, filesize);
}

// get_filetype - derive file type from filename
void get_filetype(char *filename, char *filetype)
{
	// strstr查找第一个参数 字符串中 是否有第二个参数 字符串的存在
	if(strstr(filename, ".html"))
	{
		strcpy(filetype, "text/html");
	}
	else if(strstr(filename, ".gif"))
	{
		strcpy(filetype, "image/gif");
	}
	else if(strstr(filename, ".png"))
	{
		strcpy(filetype, "image/png");
	}
	else if(strstr(filename, ".jpg"))
	{
		strcpy(filetype, "image/jpg");
	}
	else
		strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
	char buf[MAXLINE], *emptylist[] = {NULL};

	//return first part of HTTP response
	sprintf(buf, "HTTP/1.0 200 ok\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	Rio_writen(fd, buf, strlen(buf));

	// child
	if(Fork() == 0)
	{
		// ----------------------------------------
		// ----------------------------------------
		// real server would set all CGI vars here

		// set the environ parameters
		setenv("QUERY_STRING", cgiargs, 1);
		// redirect stdout to client
		Dup2(fd, STDOUT_FILENO);
		// run cgi program
		Execve(filename, emptylist, environ);
	}

	// parent waits for and reaps child
	Wait(NULL);
}



