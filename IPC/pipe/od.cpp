#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

// cin 和 scanf都是从标准输入读
// 失败不阻塞 可查看返回值确定状态
int main(int argc, char const *argv[])
{
	int ret;
    char buff[80] = {0,};
    string input;
    cin >> input;
    cout << "received:  " << input << endl;

    cin >> input;
    cout << "received2:  " << input << endl;    
    //ret = scanf("%s", buff);
    //printf("[ret: %d]buff=%s\n", ret, buff);

    //ret = scanf("%s", buff);
    //printf("[ret: %d]buff=%s\n", ret, buff);
    return 0;
	
}