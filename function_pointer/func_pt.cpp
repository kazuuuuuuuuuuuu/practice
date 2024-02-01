#include <iostream>

class MyClass {
public:
    int factor;
    static void staticMemberFunction(int x) {
        std::cout << "Static member function called with argument: " << x << std::endl;
    }

    void memberFunction(int x) {
        std::cout << "Member function called with argument: " << x << std::endl;
        std::cout << "Member function called with factor: " << factor << std::endl;
    }
};

void regularFunction(int x) {
    std::cout << "Regular function called with argument: " << x << std::endl;
}

void useregularFunctionPointer(void (*regularFunctionPointer)(int), int x)
{
    regularFunctionPointer(x);
}

void usestaticMemberFunctionPointer(void (*staticMemberFunctionPointer)(int), int x)
{
    staticMemberFunctionPointer(x);
}

void usememberFunctionPointer(MyClass obj, void (MyClass::*memberFunctionPointer)(int), int x)
{
    (obj.*memberFunctionPointer)(x);
}

int main() {
    // 函数指针指向普通函数
    void (*regularFunctionPointer)(int) = regularFunction;

    useregularFunctionPointer(regularFunctionPointer, 42);

    // 函数指针指向静态成员函数 -> 静态成员函数不属于任何一个实例 -> 无法访问非静态成员变量
    void (*staticMemberFunctionPointer)(int) = MyClass::staticMemberFunction;

    usestaticMemberFunctionPointer(staticMemberFunctionPointer, 42);

    // 函数指针指向非静态成员函数 -> 可成员变量
    MyClass obj;
    obj.factor = 10;
    void (MyClass::*memberFunctionPointer)(int) = &MyClass::memberFunction; 

    usememberFunctionPointer(obj, memberFunctionPointer, 42);

    return 0;
}