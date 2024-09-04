/*
获取当前路径，拼接root文件夹路径，并获得文件绝对路径
判断文件是否存在，是否有权限访问，是否为目录
打开文件，读取文件内容，打印文件内容
*/
#include <string.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

//root文件夹路径

char *m_root;
char m_real_file[200];
char *doc_root;
char *m_url;
struct stat m_file_stat;
char *m_file_address; //读取服务器上的文件地址

int main() {
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

    m_url = "/hello.html"; //parse_request_line函数赋值
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
    
    // 直接打印文件内容
    std::cout << "File content:\n";
    std::cout.write(m_file_address, m_file_stat.st_size);
    std::cout << std::endl;
    munmap(m_file_address, m_file_stat.st_size);
    close(fd);

}