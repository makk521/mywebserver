#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include <thread>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class threadpool
{
public:
    /* thread_number 是线程池中线程的数量，max_requests 是请求队列中最多允许的、等待处理的请求的数量 */
    threadpool(int thread_number = 8, int max_request = 10000);
    ~threadpool();
    bool append(T *request);
    bool append_p(T *request);

private:
    /* 工作线程运行的函数，它不断从工作队列中取出任务并执行之 */
    static void *worker(void *arg);
    void run();

private:
    int m_thread_number;        // 线程池中的线程数
    int m_max_requests;         // 请求队列中允许的最大请求数
    pthread_t *m_threads;       // 描述线程池的数组，其大小为 m_thread_number
    std::list<T *> m_workqueue; // 请求队列
    std::mutex m_queuelocker;   // 保护请求队列的互斥锁
    std::condition_variable m_queuestat; // 条件变量，用于线程同步
};

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests)
    : m_thread_number(thread_number), m_max_requests(max_requests)
{
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
        throw std::exception();
    for (int i = 0; i < thread_number; ++i)
    {
        // 创建线程，如果失败则释放已分配的内存并抛出异常
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        // 分离线程，确保后续可以自动释放资源
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
}

template <typename T>
bool threadpool<T>::append(T *request)
{
    std::unique_lock<std::mutex> lock(m_queuelocker);
    if (m_workqueue.size() >= m_max_requests)
    {
        return false;
    }
    m_workqueue.push_back(request);
    m_queuestat.notify_one(); // 唤醒一个等待的线程
    return true;
}

template <typename T>
bool threadpool<T>::append_p(T *request)
{
    return append(request);
}

template <typename T>
void *threadpool<T>::worker(void *arg)
{
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run()
{
    while (true)
    {
        T *request;
        {
            std::unique_lock<std::mutex> lock(m_queuelocker);
            m_queuestat.wait(lock, [this] { return !m_workqueue.empty(); }); // 等待任务
            request = m_workqueue.front();
            m_workqueue.pop_front();
        }
        if (request)
        {
            request->process();
        }
    }
}

class SumTask {
public:
    SumTask(int value) : value(value), result(0) {}
    void process() {
        result = 0;
        for (int i = 1; i <= value; ++i) {
            result += i;
        }
        printf("Sum of 1 to %d is %d\n", value, result);
    }

private:
    int value;
    int result;
};

int main() {
    threadpool<SumTask> pool(4);  // 创建一个有 4 个线程的线程池

    // 创建一些任务并将它们添加到池中
    SumTask *task1 = new SumTask(10);  // 计算 1+2+...+10 的任务
    SumTask *task2 = new SumTask(20);  // 计算 1+2+...+20 的任务

    pool.append(task1);  // 将 task1 添加到线程池
    pool.append(task2);  // 将 task2 添加到线程池

    std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待任务完成

    delete task1;
    delete task2;

    return 0;
}
