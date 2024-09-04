/*
根据HTTP协议，解析HTTP请求报文，并提取请求方法、目标url及http版本号、请求头部信息、请求内容等。
*/
#include <iostream>
#include <cstring>

// 定义请求方法的枚举类型
enum METHOD { GET, POST, BAD_REQUEST, NO_REQUEST, GET_REQUEST };
// 定义检查状态的枚举类型
enum CHECK_STATE { CHECK_STATE_REQUESTLINE, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
//从状态机状态
enum LINE_STATUS{LINE_OK = 0, LINE_BAD, LINE_OPEN};

// 声明全局变量
METHOD m_method;
char *m_version;
CHECK_STATE m_check_state;
char *m_url;
int cgi = 0; // 添加 cgi 变量


long m_content_length;
bool m_linger;
char *m_host;

//存储读取的请求报文数据
char m_read_buf[2048];
//缓冲区中m_read_buf中数据的最后一个字节的下一个位置
long m_read_idx;
//m_read_buf读取的位置m_checked_idx
long m_checked_idx;

// 解析http请求行，获得请求方法，目标url及http版本号
METHOD parse_request_line(char *text)
{
    m_url = strpbrk(text, " \t");
    if (!m_url)
    {
        return BAD_REQUEST;
    }
    *m_url++ = '\0';
    char *method = text;
    if (strcasecmp(method, "GET") == 0)
        m_method = GET;
    else if (strcasecmp(method, "POST") == 0)
    {
        m_method = POST;
        cgi = 1; // 设置cgi标志
    }
    else
        return BAD_REQUEST;
    m_url += strspn(m_url, " \t");
    m_version = strpbrk(m_url, " \t");
    if (!m_version)
        return BAD_REQUEST;
    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");
    if (strcasecmp(m_version, "HTTP/1.1") != 0)
        return BAD_REQUEST;
    if (strncasecmp(m_url, "http://", 7) == 0)
    {
        m_url += 7;
        m_url = strchr(m_url, '/');
    }

    if (strncasecmp(m_url, "https://", 8) == 0)
    {
        m_url += 8;
        m_url = strchr(m_url, '/');
    }

    if (!m_url || m_url[0] != '/')
        return BAD_REQUEST;
    //当url为/时，显示判断界面
    if (strlen(m_url) == 1)
    {
        // 确保 m_url 有足够的空间
        strcat(m_url, "judge.html");
    }
    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//解析http请求的一个头部信息
METHOD parse_headers(char *text)
{
    if (text[0] == '\0')
    {
        if (m_content_length != 0)
        {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0)
        {
            m_linger = true;
        }
    }
    else if (strncasecmp(text, "Content-length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        m_content_length = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    }
    else
    {
        std::cout << "unknow header: " << text << std::endl;
    }
    return NO_REQUEST;
}

LINE_STATUS parse_line(char *m_read_buf)
{
    /**
    * @brief 读取缓冲区中查找HTTP请求行的结束标志（即回车符和换行符的组合\r\n），并根据查找结果返回相应的状态。
    *        缓冲区为socket连接中recv的所有数据
    *        只判断操作第一个完整的行！！！！
    * @param 
    * @return 
    */
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx];
        if (temp == '\r') //回车符
        {
            if ((m_checked_idx + 1) == m_read_idx)
                return LINE_OPEN;
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == '\n') // 换行符
        {
            if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r')
            {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

int main()
{
    // parse_request_line测试
    char text[1024] = "GET /index.html HTTP/1.1";
    parse_request_line(text);
    std::cout << "method: " << m_method << std::endl;
    std::cout << "url: " << m_url << std::endl;
    std::cout << "version: " << m_version << std::endl;
    std::cout << "cgi: " << cgi << std::endl;

    // parse_headers测试
    char header1[1024] = "Content-Length: 0";
    parse_headers(header1);
    std::cout << "content_length: " << m_content_length << std::endl;

    char header2[1024] = "Connection: keep-alive";
    parse_headers(header2);
    std::cout << "linger: " << m_linger << std::endl;

    char header3[1024] = "Host: 124.223.76.58:9006";
    parse_headers(header3);
    std::cout << "host: " << m_host << std::endl;

    // parse_line测试
    char read_buf[2048] = "GET /index.html HTTP/1.1\rContent-Length: 0\r\nConnection: keep-alive\r\nHost: 124.223.76.58:9006\r\n\r\n";
    m_read_idx = strlen(read_buf);
    m_checked_idx = 0;
    LINE_STATUS status = LINE_OK;
    std::cout << "Parse result : " << parse_line(read_buf) << std::endl;
    //std::cout << "Parse buffer result : " << read_buf << std::endl;
    return 0;
}