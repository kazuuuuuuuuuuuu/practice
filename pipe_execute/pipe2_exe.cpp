#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

int main(int argc, char const *argv[])
{

	char buff[1024];

	// read from standard input
	read(STDIN_FILENO, buff, sizeof(buff));
	printf("received: %s\n", buff);
	return 0;
}