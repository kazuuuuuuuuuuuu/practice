#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_SIZE 80

struct my_msg_st
{
	long int msg_type; // must be defined
	char msg[MSG_SIZE]; // the length sent does not consider the first member
};

int main(int argc, char const *argv[])
{
	int msgid;
	int ret;
	struct my_msg_st msg;

	msgid = msgget((key_t) 1235, 0666|IPC_CREAT);
	if(msgid<0)
	{
		fprintf(stderr, "msgget failed\n");
		exit(1);
	}

	// receive messge 
	ret = msgrcv(msgid, &msg, MSG_SIZE, 0, 0);
	if(ret==-1)
	{
		fprintf(stderr, "msgsnd failed\n");
		exit(1);		
	}

	printf("received: %s\n", msg.msg);

	ret = msgctl(msgid, IPC_RMID, 0);
	if(ret<0)
	{
		fprintf(stderr, "msgctl failed\n");
		exit(1);			
	}
	return 0;
}