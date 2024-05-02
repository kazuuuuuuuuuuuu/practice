#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 1024

int main(int argc, char const *argv[])
{
	FILE *file;
	char buff[BUFF_SIZE];
	int cnt;
	// execute pipe2_exe and write some data to it
	file = popen("./pipe2_exe", "w");
	if(!file)
	{
		fprintf(stderr, "popen failed\n");
		exit(1);
	}

	strcpy(buff, "hello world! kazu is the only king.");
	cnt = fwrite(buff, sizeof(char), strlen(buff)+1, file);
	pclose(file);
	return 0;
}
