#include <iostream>
#include <string>
#include <list>
#include <ctime>
using namespace std;

class TodoItem
{
private:
	int id;
	string description;
	bool completed;

public:
	//  initialization
	TodoItem() : id(0), description(""), completed(false) {}
	~TodoItem() = default;

	bool create(string new_description)
	{
		// generate a random integer between 1 - 100
		id = rand() % 100 + 1;
		description = new_description;
		return true;
	}

	int getid() {return id;}
	string getdescription() {return description;}
	bool iscompleted() {return completed;}

	void setcompleted(bool val)
	{
		completed = val;
		return; 
	}

};

int main(int argc, char const *argv[])
{
	char input_option;
	int input_id;
	string input_description;
	string version = "v0.2.0";
	list<TodoItem> todoItems;
	// 创建一个iterator
	list<TodoItem>::iterator it;

	srand(time(NULL));

	todoItems.clear();

	//TodoItem test;
	//test.create("this is a test");
	//todoItems.push_back(test);

	while(1)
	{
		// a system call each iteration it will clear the screen
		system("cls");
		cout << "todo list maker - " << version << endl;
		cout << endl << endl;

		for(it=todoItems.begin();it!=todoItems.end();it++)
		{	

			string completed = it->iscompleted() ? "done" : "not done";

			// iterator can be seen as a pointer
			cout << it->getid() << " | " << it->getdescription() << " | " 
			<< completed << endl;
		}

		if(todoItems.empty())
		{
			cout << "add your first todo!" << endl;
		}

		cout << "[a]dd a new Todo" << endl;
		cout << "[c]complete a todo" << endl;
		cout << "[q]uit" << endl;

		cout << "choice: ";

		cin >> input_option;

		if(input_option=='q')
		{
			cout << "have a great day now！" << endl;
			break;
		}
		else if(input_option=='c')
		{
			cout << "enter id to mark completed: " << endl;
			cin >> input_id;

			for(it=todoItems.begin();it!=todoItems.end();it++)
			{	
				if(it->getid()==input_id)
				{
					it->setcompleted(true);
					break;
				}
			}			
		}
		else if(input_option=='a')
		{
			cout << "add a new description: ";
			// 从用户那里接受input之前要做的事
			cin.clear();
			// 无视选项后面的换行
			cin.ignore();
			//cin >> input_description;
			getline(cin, input_description);

			// 从我的自定义类中创建一个对象
			TodoItem newItem;
			newItem.create(input_description);
			todoItems.push_back(newItem);
		}
	}


	return 0;
}