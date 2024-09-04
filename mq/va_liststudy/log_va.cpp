#include <iostream>
#include <cstdarg> // 包含 va_list, va_start, va_arg, va_end
#include <cstdio>  // 包含 vsnprintf

// 定义一个日志函数，接受格式化字符串和可变数量的参数
bool log_message(const char *format, ...) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    va_list args;
    va_start(args, format); // 初始化 va_list，指向 format 参数之后的第一个可变参数

    // 将格式化字符串和参数列表写入 buffer,返回值为写入到缓冲区（不包括终止符 \0）的字符总数
    // 如果该总数等于或大于 size 参数，则表示输出被截断。如果发生错误，则返回一个负值。
    int len = vsnprintf(buffer, BUFFER_SIZE, format, args);

    if (len >= (BUFFER_SIZE - 1))
    {
        va_end(args);
        return false;
    }

    va_end(args); // 清理 va_list

    // 输出格式化后的字符串到控制台
    std::cout << buffer << std::endl;
}

int main() {
    log_message("This is a simple message.");
    log_message("Formatted number: %d", 42);
    log_message("Multiple values: %s, %d, %.2f", "example", 123, 45.67);

    return 0;
}
