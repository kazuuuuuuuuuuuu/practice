#include <iostream>
using namespace std;

void myprintf(char * msg)
{
	cout << msg << endl;
}

int main(int argc, char const *argv[])
{
	myprintf("hello");
	return 0;
}