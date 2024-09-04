/*
namespace的定义与使用
*/
#include <iostream>
#include <string>

namespace student {
    int age = 18;
    std::string name = "John";
}

namespace teacher {
    int age = 25;
    std::string name = "Mary";
}

int main() {
    std::cout << "Student age: " << student::age << std::endl;
    std::cout << "Student name: " << student::name << std::endl;
    std::cout << "Teacher age: " << teacher::age << std::endl;
    std::cout << "Teacher name: " << teacher::name << std::endl;
    return 0;
}