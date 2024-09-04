/*
enum做函数的返回类型,返回的是enum中对应的编号
枚举常量代表该枚举类型的变量可能取的值，编译系统为每个枚举常量指定一个整数值，默认状态下，
这个整数就是所列举元素的序号，序号从0开始。 可以在定义枚举类型时为部分或全部枚举常量指定整
数值，在指定值之前的枚举常量仍按默认方式取值，而指定值之后的枚举常量按依次加1的原则取值。 
各枚举常量的值可以重复。

用于http_conn类中的一些
*/
#include <iostream>

enum Color {
    red,
    green,
    blue
};

class Hello
{
private:
    Color color;
public:
    Hello(Color hellocolor);
    ~Hello();

    Color getColor();
};

Hello::Hello(Color hellocolor)
{
    color = hellocolor;
}

Hello::~Hello()
{
}

Color Hello::getColor()
{
    
    return color;
}

int main() {
    Hello hello(red);
    std::cout << "Color: " << hello.getColor() << std::endl;
    return 0;
}