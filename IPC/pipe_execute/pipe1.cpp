#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int main(int argc, char const *argv[])
{
	FILE *file;
	char buff[BUFF_SIZE];
	int cnt;

	// execute ls -l -> redirect the output to this program
	file = popen("ls -ls", "r");
	if(!file)
	{
		printf("popen failed\n");
		exit(1);
	}

	cnt = fread(buff, sizeof(char), BUFF_SIZE, file);
	if(cnt > 0)
	{
		buff[cnt] = '\0';
		printf("%s", buff);
	}

	pclose(file);
	return 0;
}