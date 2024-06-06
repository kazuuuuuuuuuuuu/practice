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
		list_account_records_response larr;
		larr.set_code(200);
		larr.set_desc("ok");
		for(int i=0;i<5;i++)
		{
			list_account_records_response_account_record *ar = larr.add_records();
			ar->set_type(0);
			ar->set_limit(i*100);
			ar->set_timestamp(time(NULL));
		}
		printf("records size: %d\n", larr.records_size());
		larr.SerializeToString(&data);		
		
		// send(sockfd, data.c_str(), data.length());
	}

	// server
	{
		// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
		// recv(sockfd, buf, len, 0); -> 读，之后转换为string类型到data中
		
		list_account_records_response larr;
		larr.ParseFromString(data);
		printf("records size: %d\n", larr.records_size());
		cout << "recv code: " << larr.code() << endl;		
		cout << "recv desc: " << larr.desc() << endl;	
		for(int i=0;i<larr.records_size();i++)
		{
			const list_account_records_response_account_record ar = larr.records(i); // 该函数返回一个引用
			cout << "recv limit: " << ar.limit() << endl;
		}		
	}

	return 0;
}