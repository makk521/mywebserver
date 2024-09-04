/*
sigaddset，sigprocmask，sigaction 用于屏蔽信号
sigaddset用于将信号加入到信号集中，sigaction.sa_mask选定该信号集，表示
运行sigaction.sa_handler函数时屏蔽信号集中的信号。sigprocmask用于设置信号屏蔽集。
*/
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

// 定义信号处理函数
void signalHandler(int signum) {
    std::cout << "定时器触发! 信号号: " << signum << std::endl;

    // 重新设置定时器，2秒后再次触发SIGALRM信号
    alarm(2);
}

int main() {
    sigset_t set;

    sigaddset(&set, SIGALRM); // 仅阻塞 SIGALRM
    // sigfillset(&set); // 将 set 中所有信号标志设置为包含状态
    sigprocmask(SIG_BLOCK, &set, NULL); // 应用阻塞 
    printf("Alarm信号已被阻塞\n");

    // 主程序继续执行...
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    // 设置信号处理函数
    sa.sa_handler = signalHandler;
    //sa.sa_mask = set; // 屏蔽 SIGALRM 信号

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
