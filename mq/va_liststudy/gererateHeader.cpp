/*
va_list实现参数的可变参数列表，可以用来代替参数数组，可以方便地处理函数的可变参数。
用于生成回传给客户端的HTTP响应。
*/
#include <iostream>
#include <cstdarg> // 包含 va_list, va_start, va_arg, va_end
#include <cstdio>  // 包含 vsnprintf
#include <cstring> // 包含 strlen

static const int WRITE_BUFFER_SIZE = 1024;
char m_write_buf[WRITE_BUFFER_SIZE];
int m_write_idx;
bool m_linger;

//定义http响应的一些状态信息
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

const char *ok_string = "<html><body></body></html>";

bool add_response(const char *format, ...)
{
    //将参数格式化到m_write_buf中，也就是将格式化后的字符串存入m_write_buf中，并返回是否成功
    //m_write_idx表示当前缓冲区的长度，如果超过了缓冲区的大小，则返回false
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false; //缓冲区已满，返回 false
    va_list arg_list;
    va_start(arg_list, format);
    // 将最多第二个参数这么大数据存入第一个参数的缓冲区中
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
    {
        va_end(arg_list);
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);
    std::cout << "request: " << m_write_buf << std::endl;

    return true;
}
bool add_status_line(int status, const char *title)
{
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}
bool add_content_length(int content_len)
{
    return add_response("Content-Length:%d\r\n", content_len);
}
bool add_blank_line()
{
    return add_response("%s", "\r\n");
}
bool add_linger()
{
    return add_response("Connection:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}
bool add_headers(int content_len)
{
    return add_content_length(content_len) && add_linger() &&
           add_blank_line();
}

bool add_content_type()
{
    return add_response("Content-Type:%s\r\n", "text/html");
}
bool add_content(const char *content)
{
    return add_response("%s", content);
}

int main() {
    // 重置缓冲区
    m_write_idx = 0;
    m_write_buf[0] = '\0';

    //未清空缓存，会在前面的结果上追加
    add_status_line(200, "OK");
    add_status_line(500, error_500_title);
    add_headers(strlen(error_500_form));
    add_content(ok_string);
    return 0;
}