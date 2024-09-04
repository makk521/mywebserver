/*
生成包含文件内容的HTTP响应报文
模拟process_read、process_write函数
*/
#include <iostream>
#include <cstdarg> // 包含 va_list, va_start, va_arg, va_end
#include <cstdio>  // 包含 vsnprintf
#include <cstring> // 包含 strlen
#include <string.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

static const int WRITE_BUFFER_SIZE = 1024;
char m_write_buf[WRITE_BUFFER_SIZE];
int m_write_idx;
bool m_linger;
struct iovec m_iv[2]; //io向量机制iovec  m_iv[0]表示返回的报文头部，m_iv[1]表示返回的文件内容
int m_iv_count;

char *m_root;
char m_real_file[200];
char *doc_root;
char *m_url;
struct stat m_file_stat;
char *m_file_address; //读取服务器上的文件地址

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
    // std::cout << "request: " << m_write_buf << std::endl;

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

void getFile(char *m_url){
    /*
    模拟process_read()函数，根据url获取文件地址，并判断文件是否存在、是否有权限访问、是否为目录，
    并将文件内容用mmap映射到内存中，获得对应内存的地址
    */
    //WebServer中的init函数
    char server_path[200];
    getcwd(server_path, 200);
    std::cout << "server_path: " << server_path << std::endl;
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path); //拷贝
    strcat(m_root, root); //追加
    std::cout << "m_root: " << m_root << std::endl;

    //do_request()函数
    doc_root = m_root;
    strcpy(m_real_file, doc_root); //m_root=root=doc_root，传参
    int len = strlen(doc_root);

    const char *p = strrchr(m_url, '/');
    std::cout << "*p: " << *p << std::endl;

    strncpy(m_real_file + len, m_url, 200 - len - 1); 
    std::cout << "m_real_file: " << m_real_file << std::endl;

    if (stat(m_real_file, &m_file_stat) < 0) //判断文件是否存在
        std::cout << "file not exist" << std::endl;

    if (!(m_file_stat.st_mode & S_IROTH)) //判断是否有权限访问
        std::cout << "no permission" << std::endl;

    if (S_ISDIR(m_file_stat.st_mode))  //判断是否为目录
        std::cout << "is a directory" << std::endl;

    int fd = open(m_real_file, O_RDONLY);
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
}

bool gererateResponse(){
    /*
    模拟process_write函数，将报文头部放到m_iv[0]中，文件内容放到m_iv[1]中，并返回true
    */
    add_status_line(200, ok_200_title);
    if (m_file_stat.st_size != 0)
    {
        add_headers(m_file_stat.st_size);
        m_iv[0].iov_base = m_write_buf;
        m_iv[0].iov_len = m_write_idx;
        m_iv[1].iov_base = m_file_address;
        m_iv[1].iov_len = m_file_stat.st_size;
        m_iv_count = 2;
        return true;
    }
    else
    {
        const char *ok_string = "<html><body></body></html>";
        add_headers(strlen(ok_string));
        if (!add_content(ok_string))
            return false;
    }
}

int main() {
    // 重置缓冲区
    m_write_idx = 0;
    m_write_buf[0] = '\0';
    m_url = "/hello.html"; //parse_request_line函数赋值

    getFile(m_url);
    gererateResponse();

    std::cout << "m_iv[0] : " << std::endl;
    std::cout.write(static_cast<char*>(m_iv[0].iov_base), m_iv[0].iov_len);
    std::cout << std::endl;
    std::cout << "m_iv[1] : " << std::endl;
    std::cout.write(static_cast<char*>(m_iv[1].iov_base), m_iv[1].iov_len);

    return 0;
}