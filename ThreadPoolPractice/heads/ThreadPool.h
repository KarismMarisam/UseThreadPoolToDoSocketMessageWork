#include "TaskQueue.h"
#include <mutex>
#include <semaphore.h>
#include <thread>
#include <chrono>
class ThreadPool
{
public:
    ThreadPool(int min, int max);
    ~ThreadPool();

    // 添加任务y
    void addTask(Task task);
    void addTask(callback function,void* arg);
    // 获取忙线程的个数
    int getBusyNumber();
    // 获取活着的线程个数
    int getAliveNumber();

private:
    // 工作的线程的任务函数
    static void* worker(void* arg);
    // 管理者线程的任务函数
    static void* manager(void* arg);
    void threadExit(void* arg);

private:
    std::mutex m_lock;
    sem_t m_notEmpty;

    TaskQueue* m_taskQ;

    std::thread* workThreads;
    std::thread manageThread;
    
    int m_minNum;
    int m_maxNum;
    int m_busyNum;
    int m_aliveNum;
    int m_exitNum;
    bool m_shutdown = false;
};


