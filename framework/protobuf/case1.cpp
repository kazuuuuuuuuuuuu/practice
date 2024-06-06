// g++ --std=c++11  case1.cc bike.pb.cc -lprotobuf -o test -lpthread
#include "bike.pb.h"
#include <string>
#include <iostream>

using namespace std;
using namespace tutorial;

int main(int argc, char const *argv[])
{
	string data;

	// client
	{
		mobile_request mr;
		mr.set_mobile("15011451774");
		mr.SerializeToString(&data);		
		cout << "after serialization: " << endl;
		cout << "tag:" << hex << (int)((data.c_str())[0]) << endl;	
		cout << "length: " << hex << (int)((data.c_str())[1]) << endl;	
		// send(sockfd, data.c_str(), data.length());
	}

	// server
	{
		// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
		// recv(sockfd, buf, len, 0); -> 读，之后转换为string类型到data中
		mobile_request mr_server;
		mr_server.ParseFromString(data);
		cout << "recv: " << mr_server.mobile() << endl;		
	}

	return 0;
}