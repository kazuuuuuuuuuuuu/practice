#include <iostream>
#include <unordered_map>
#include <functional>



// unordered_map 
// 自定义类用作key 需要重载==和operator()
class MyClass {
public:
    int x;
    int y;

    MyClass(int x, int y) : x(x), y(y) {}

    // 重载 == 运算符
    bool operator==(const MyClass& other) const {
        return x == other.x && y == other.y;
    }
};

struct MyClassHash {
    size_t operator()(const MyClass& obj) const {
        return std::hash<int>()(obj.x) ^ std::hash<int>()(obj.y);
    }
};

int main() {
    std::unordered_map<MyClass, std::string, MyClassHash> map;

    MyClass obj1(1, 2);
    MyClass obj2(3, 4);
    MyClass obj3(1, 5);

    map[obj1] = "Object1";
    map[obj2] = "Object2";
    map[obj3] = "Object3";

    for (const auto& pair : map) {
        std::cout << pair.second << " with key (" << pair.first.x << ", " << pair.first.y << ")" << std::endl;
    }

    return 0;
}
