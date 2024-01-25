#ifndef _RIO_LIB_H_
#define _RIO_LIB_H_

#include <unistd.h>
#include <errno.h>
#include <cstring>

#define RIO_BUFSIZE 8192

// read and write without application buffer
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, const void *usrbuf, size_t n);

// read with application buffer
struct rio_t
{
	int rio_fd;
	int rio_cnt;
	char *rio_bufptr; // point to the next byte in the buffer
	char rio_buf[RIO_BUFSIZE];
};

void rio_readinitb(rio_t *rp, int fd);

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen); // read a line
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
#endif