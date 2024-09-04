#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

#define BUF_SIZE 512
#define MAX_EVENTS 10
#define MAX_CONN_COUNT 20

/**
 * 创建服务端socket，并返回socket文件描述符
 */
int createServerSocket(int listener_port) {
    // 创建服务端socket套接字
    int server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), client_sockfd = -1;
    // 将套接字和IP、端口绑定
    struct sockaddr_in server_addr;
    // 申请内存
    memset(&server_addr, 0, sizeof(server_addr)); 
    // IPv4地址
    server_addr.sin_family = AF_INET;  
    // IP地址
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    // 端口
    server_addr.sin_port = htons(listener_port); 
    // 绑定socket
    bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // 监听端口，最大连接数为20
    listen(server_sockfd, MAX_CONN_COUNT);

    printf("[debug] listening on %s:%d ...\n", "127.0.0.1", listener_port);
    return server_sockfd;
}

/**
 * 初始化epoll
 */
int initEpoll(int server_sockfd, struct epoll_event *ev) {
    // 创建epoll并初始化（加入server_sockfd）
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        printf("[error] epoll error\n");
        exit(EXIT_FAILURE);
    }
    ev->events = EPOLLIN;
    ev->data.fd = server_sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_sockfd, ev) == -1) {
        printf("[error] epoll server_sockfd error\n");
        exit(EXIT_FAILURE);
    }
    return epollfd;
}

/**
 * 处理connect事件，添加read事件
 */
void handleConnectSocketData(int server_sockfd, int epollfd, struct epoll_event *ev) {
    // 监听对应端口是否有事件触发
    printf("[debug] new client connecting...\n");
    // 有事件触发 accept一个与客户端对应的socket
    struct sockaddr_in client_addr;
    socklen_t clnt_addr_size = sizeof(client_addr);
    int client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &clnt_addr_size);
    if (client_sockfd == -1) {
        printf("[error] accept error\n");
        exit(EXIT_FAILURE);
    }
    ev->events = EPOLLIN | EPOLLET;
    ev->data.fd = client_sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sockfd, ev) == -1) {
        printf("[error] epoll client_sockfd error\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * 处理read事件，添加write事件
 */
void handleReadSocketData(int client_sockfd, int epollfd, struct epoll_event *ev) {
    printf("[debug] client %d is readable...\n", client_sockfd);
    // 定义数据缓冲
    char buf[BUF_SIZE];
    ssize_t nread;
    // 清空数组，用于接收新消息
    memset(buf, 0, sizeof(buf)); 
    nread = recv(client_sockfd, buf, BUF_SIZE,  MSG_DONTWAIT);
    // Ignore failed request
    if (nread == -1) {
        printf("[error] recv error\n");
        exit(EXIT_FAILURE);  
    } else if (nread == 0) {
        // 关闭socket
        printf("[debug] client exit...\n");
        close(client_sockfd); 
    } else {
        int received_size = (int)(strlen(buf));
        printf("<<<< Received request：%s, size: %d\n", buf, received_size);
        ev->events = EPOLLOUT | EPOLLET;
        ev->data.fd = client_sockfd;
        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, client_sockfd, ev) == -1) {
            printf("[error] epoll client_sockfd error\n");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * 处理write事件，添加read事件
 */
void handleWriteSocketData(int client_sockfd, int epollfd, struct epoll_event *ev) {
    printf("[debug] client %d is writeable...\n", client_sockfd);
    // 向客户端发送数据
    char feedback[] = "WriteResult({ \"nRemoved\" : 1 })";
    printf(">>>> Sending response: %s\n", feedback);
    write(client_sockfd, feedback, sizeof(feedback));
    ev->events = EPOLLIN | EPOLLET;
    ev->data.fd = client_sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, client_sockfd, ev) == -1) {
        printf("[error] epoll client_sockfd error\n");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // 定义服务端和客户端socket文件描述符
    int server_sockfd = -1, client_sockfd = -1;
    // 定义epoll相关
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd, fd;

    // 创建socket
    server_sockfd = createServerSocket(6000);
    // 初始化epoll
    epollfd = initEpoll(server_sockfd, &ev);

    // 使用while来监听连接到socket的事件
    while (1) {
        printf("[debug] start epoll...\n");
        // 使用epoll模型进行管理
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);;
        printf("[debug] epoll value is: %d\n", nfds);
        if (nfds < 0) {
            printf("[error] epoll error!!!\n");
            exit(EXIT_FAILURE);
        } 
        for (fd = 0; fd < nfds; fd++)  {
            // 有新的连接进来
            if (events[fd].data.fd == server_sockfd) {
                handleConnectSocketData(server_sockfd, epollfd, &ev);
            } 
            // 可读
            else if (events[fd].events & EPOLLIN){
               handleReadSocketData(events[fd].data.fd, epollfd, &ev);
            }
            // 可写
            else if (events[fd].events & EPOLLOUT){
                handleWriteSocketData(events[fd].data.fd, epollfd, &ev);
            }
        }
    }
    return 0;
}