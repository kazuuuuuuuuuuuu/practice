#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

using namespace std;

#define MSG_SIZE 80
struct my_msg_st
{
	long int msg_type; // must be defined
	char msg[MSG_SIZE]; // the length sent does not consider the first member
};

int main(int argc, char const *argv[])
{
	int i = 1;
	int msgid;
	int ret;
	struct my_msg_st msg;
	stringstream ss;

	// 1 establish the message queue
	msgid = msgget((key_t) 1235, 0666|IPC_CREAT);
	if(msgid<0)
	{
		fprintf(stderr, "msgget failed\n");
		exit(1);
	}

	msg.msg_type = 1;
	string user_input;
	string content;

	while(1)
	{
		// 2 receive user input
		ss.str("");
		cin >> user_input;
		ss << user_input << ", time: " << i << endl; // concatenating
		i ++;
		content = ss.str();
		strcpy(msg.msg, content.c_str());

		// 3 send messge 
		ret = msgsnd(msgid, &msg, MSG_SIZE, 0);
		if(ret==-1)
		{
			fprintf(stderr, "msgsnd failed\n");
			exit(1);		
		}	

		if(user_input=="exit")
			break;
	}

	return 0;
}