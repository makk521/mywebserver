/*
这是一个简单的定时信号走向模拟，流向如下：
定时器信号->处理函数->管道->epoll->读取管道->处理信号
*/
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>

// 全局变量用于存储socketpair的文件描述符
int sockets[2];

// 信号处理函数
void alarm_handler(int signum) {
    int msg = signum;
    std::cout << "Received signal: " << msg << std::endl;
    if (signum == SIGALRM) {
        // 将信号值写入管道
        send(sockets[1], (char *)&msg, 1, 0);
        alarm(2);
    }
}

int main() {
    // 创建socketpair
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
        perror("socketpair");
        return 1;
    }

    // 设置SIGALRM信号处理函数
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alarm_handler;
    sigaction(SIGALRM, &sa, NULL);

    // 创建epoll实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return 1;
    }

    // 添加socketpair的一端到epoll监听
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sockets[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[0], &event) == -1) {
        perror("epoll_ctl");
        return 1;
    }

    // 设置定时器，2秒后触发SIGALRM信号
    alarm(2);

    // 等待epoll事件
    struct epoll_event events[10];
    while (true) {
        int nfds = epoll_wait(epoll_fd, events, 10, -1);
        if (nfds == -1) {
            if (errno == EINTR) {
                // 被信号中断，重新调用epoll_wait
                continue;
            } else {
                perror("epoll_wait");
                return 1;
            }
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == sockets[0]) {
                char received_signal;
                int count = recv(sockets[0], &received_signal, sizeof(received_signal), 0);
                if (count > 0) {
                    std::cout << "count: " << count << std::endl;   
                    for (int i = 0; i < count; ++i)
                        {
                            switch (received_signal)
                            {
                            case SIGALRM:
                            {
                                std::cout << "Received SIGALRM " << std::endl;
                                break;
                            }
                            case SIGTERM:
                            {
                                std::cout << "Received SIGTERM" << std::endl;
                                break;
                            }
                            }
                        }
                    std::cout << "Received signal: " << (int)received_signal << std::endl;
                } else {
                    perror("recv");
                }
            }
        }
    }

    // 关闭文件描述符
    close(sockets[0]);
    close(sockets[1]);
    close(epoll_fd);

    return 0;
}
