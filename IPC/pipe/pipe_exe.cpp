#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char const *argv[])
{
	int fd[2];
	int ret;
	ret = pipe(fd);
	if(ret<0)
	{
		fprintf(stderr, "pipe failed\n");
		exit(1);
	}

	pid_t pid;
	pid = fork();
	if(pid<0)
	{
		fprintf(stderr, "fork failed\n");
		exit(1);
	}
	else if(pid==0)
	{
		close(fd[1]);
		int len;
		char buff[1024];
		while((len=read(fd[0], buff, sizeof(buff)))>0)
		{
			cout << "child process echoes: " << buff << endl;
		}
		close(fd[0]);
		exit(0); // 直接结束 之后就不用考虑了
	}
	else
	{
		close(fd[0]);
		string input;
		while(1)
		{
			cout << "please enter a string: ";
			getline(cin, input);
			char buff2[1024];
			strcpy(buff2, input.c_str());	
			
			if(strcmp(buff2, "exit")==0)
			{
				close(fd[1]);
				break;
			}

			write(fd[1], buff2, strlen(buff2)+1);
		}
	}

	if(pid>0)
	{
		wait(NULL);
	}
	return 0;
}

