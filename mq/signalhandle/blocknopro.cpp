/*
收到定时信号SIGALRM后进入处理函数，处理过程中屏蔽所有信号(ctrl+c无效)，保证xEFU的正常运行。
sigaddset用于将信号加入到信号集中，sigaction.sa_mask选定该信号集，表示
运行sigaction.sa_handler函数时屏蔽信号集中的信号。sigprocmask用于设置信号屏蔽集。
*/
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <assert.h>

// 定义信号处理函数
void signalHandler(int signum) {
    std::cout << "定时器触发进入函数,五秒内屏蔽所有信号! 信号: " << signum << std::endl;
    sleep(5);
    std::cout << "五秒结束，可以接收其他信号了!" << std::endl;
    // 重新设置定时器，2秒后再次触发SIGALRM信号
    alarm(5);
}

//设置信号函数
void addsig(int sig, void(handler)(int))
{
    /*
    这段代码通过配置 sigaction 结构体并使用 sigaction 系统调用，
    实现了信号处理函数的注册和配置。它确保在接收到特定信号时，
    能够执行指定的信号处理逻辑，并且在处理期间阻塞所有其他信号，
    以保证信号处理的完整性。
    */
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;

    // 使用 sigfillset 将 sa.sa_mask 设置为包含所有信号即阻塞所有信号
    sigfillset(&sa.sa_mask);
    // 使用 sigaction 系统调用注册信号处理函数
    // assert 确保 sigaction 调用成功，如果失败则终止程序
    assert(sigaction(sig, &sa, NULL) != -1);
}


int main() {
    addsig(SIGALRM, signalHandler);

    // 设置初始定时器，2秒后触发SIGALRM信号
    alarm(2);

    // 主循环，让程序一直运行
    while (true) {
        pause();  // 等待信号
    }

    return 0;
}
