#include <iostream>
#include <map>
#include <vector>
#include <utility>


// pair vector 和自定义类 作为map的键
// 只要重载了< 运算符即可
class MyClass {
public:
    int x;
    int y;

    // Constructor
    MyClass(int x, int y) : x(x), y(y) {}

    // Define less than operator for MyClass objects
    bool operator<(const MyClass& other) const {
        return x < other.x;
    }
};

int main() {
    std::map<std::vector<int>, std::string> map1;
    std::map<std::pair<int, int>, int> map2;
    std::map<MyClass, std::string> map3;

    std::vector<int> vec = {1, 2, 3};
    std::pair<int, int> p = std::make_pair(1, 2);
    MyClass obj1(1, 2);
    MyClass obj2(3, 4);

    // Insertion
    map1[vec] = "Vector";
    map2[p] = 100;
    map3[obj1] = "Object1";
    map3[obj2] = "Object2";

    // Access
    std::cout << map1[vec] << std::endl;
    std::cout << map2[p] << std::endl;
    std::cout << map3[obj1] << std::endl;
    std::cout << map3[obj2] << std::endl;

    return 0;
}