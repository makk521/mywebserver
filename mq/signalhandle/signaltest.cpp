/*
实现定时器信号处理函数，并在定时器到期时发送信号给主进程，通知主进程进行处理。
*/

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <iostream>

// 定义信号处理函数
void signalHandler(int signum) {
    std::cout << "定时器触发! 信号号: " << signum << std::endl;

    // 重新设置定时器，2秒后再次触发SIGALRM信号
    alarm(2);
}

int main() {
    // 创建sigaction结构体
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    // 设置信号处理函数
    sa.sa_handler = signalHandler;

    // 安装SIGALRM信号的处理程序
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        std::cerr << "无法捕捉SIGALRM信号" << std::endl;
        return 1;
    }

    // 设置初始定时器，2秒后触发SIGALRM信号
    alarm(2);

    // 主循环，让程序一直运行
    while (true) {
        pause();  // 等待信号
    }

    return 0;
}