#include "rio_lib.h"

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = reinterpret_cast<char*>(usrbuf);

	while(nleft>0)
	{
		if((nread=read(fd, bufp, nleft))<0)
		{
			if(errno==EINTR) // retry
				nread = 0;
			else 
				return -1; 
		}
		else if(nread==0) // eof
			break; 

		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nwritten;
	char *bufp = reinterpret_cast<char*>(usrbuf);

	while(nleft>0)
	{
		if((nwritten=write(fd, bufp, nleft))<=0) // include nwritten == 0
		{
			if(errno==EINTR) // retry
				nwritten = 0;
			else 
				return -1;			
		}

		nleft -= nwritten;
		bufp += nwritten;
	}
	return n;
}

void rio_readinitb(rio_t *rp, int fd)
{
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}

// return value -> -1, 0, the number of char read
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
	// 1 ensure the buffer has data -> read data to the internal buffer
	while(rp->rio_cnt<=0)
	{
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if(rp->rio_cnt<0)
		{
			if(errno!=EINTR)
				return -1;
		}
		else if(rp->rio_cnt==0) // eof
			return 0;
		else
			rp->rio_bufptr = rp->rio_buf; // reset rio_bufptr to the beginning
	}

	// 2 copy min(n, rp->rio_cnt) data from internal buffer to user buffer
	int	cnt = n;
	if(rp->rio_cnt<n)
		cnt = rp->rio_cnt;
	memcpy(usrbuf, rp->rio_bufptr, cnt);
	rp->rio_bufptr += cnt;
	rp->rio_cnt -= cnt;
	return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
	int rc;
	char c, *bufp = reinterpret_cast<char*>(usrbuf);

	for(int n=1;n<maxlen;n++) // 1 - (maxlen-1) 
	{
		if((rc=rio_read(rp, &c, 1))==1) // read one char ever time
		{
			*bufp = c;
			bufp ++;
			if(c=='\n') // reach the end of the line
			{
				n ++;
				break; 
			}
		}
		else if(rc==0) // eof
		{
			if(n==1) // eof with no data read
				return 0;
			else
				break;
		}
		else
			return -1;
	}
	*bufp = 0; // end by 0
	return n-1;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = reinterpret_cast<char*>(usrbuf);

	while(nleft>0)
	{
		if((nread=rio_read(rp, bufp, nleft))<0)
			return -1; // read with buffer -> no need to retry
		else if(nread==0)
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}