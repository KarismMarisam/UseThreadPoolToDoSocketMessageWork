#include <iostream>
#include "ThreadPool.h"

ThreadPool::ThreadPool(int minNum, int maxNum)
{
    // 实例化任务队列
    m_taskQ = new TaskQueue;
    do {
        // 初始化线程池
        m_minNum = minNum;
        m_maxNum = maxNum;
        m_busyNum = 0;
        m_aliveNum = minNum;

        /////////////////// 创建线程 //////////////////
        // 根据最小线程个数, 创建线程
        workThreads=new std::thread[maxNum];//heap 
        if(workThreads==nullptr){
            std::cout<<"not enough space for workers"<<std::endl;
        }
         // 初始化semaphore for tasks
        if (sem_init(&m_notEmpty,0,0))
        {
            std::cout << "init sem fail..." << std::endl;
            break;
        }

           for (int i = 0; i < minNum; ++i)
        {
            std::thread t(worker,this);
            std::cout << "创建子线程, ID: " << t.get_id() << std::endl;
            workThreads[i]=std::move(t);
        }
        // 创建管理者线程, 1个
        std::thread t(manager,this);
        manageThread=std::move(t);
    } while (0);//when it arrive here it ends
}

ThreadPool::~ThreadPool()
{
    m_shutdown = 1;
    // 销毁管理者线程
    if(manageThread.joinable())
        manageThread.join();
    else 
       std::cout<<"managerThread is not joinable!"<<std::endl;

    // 唤醒所有消费者线程
    for (int i = 0; i < m_aliveNum; ++i)
    {
        sem_post(&m_notEmpty);
    }
    //wait for work thread to end work
    while(1){
        if(m_busyNum<=0)
            break;
    }

    // join all joinable thread(important before delete thread obj!)
    for (int i = 0; i < m_maxNum; ++i) {
        if (workThreads[i].joinable()) {
            workThreads[i].join();
        }
    }

    if (m_taskQ) delete m_taskQ;
    if (workThreads) delete[]workThreads;//if you dont wait for joinable to join,delete obj will result terminate error!
    sem_destroy(&m_notEmpty);
}

void ThreadPool::addTask(Task task)
{
    if (m_shutdown)
    {
        return;
    }
    // 添加任务，不需要加锁，任务队列中有锁
    m_taskQ->addTask(task);//m_task is a ptr pointing at the taskqueue object
    // 唤醒工作的线程
    sem_post(&m_notEmpty);
}

void ThreadPool::addTask(callback function,void* arg){
     if (m_shutdown)
    {
        return;
    }
    // 添加任务，不需要加锁，任务队列中有锁
    m_taskQ->addTask(function,arg);//m_task is a ptr pointing at the taskqueue object
    // 唤醒工作的线程
    sem_post(&m_notEmpty);//this->m_notEmpty
}


int ThreadPool::getAliveNumber()
{
    int threadNum = 0;
    m_lock.lock();
    threadNum = m_aliveNum;
    m_lock.unlock();
    return threadNum;
}

int ThreadPool::getBusyNumber()
{
    int busyNum = 0;
    m_lock.lock();
    busyNum = m_busyNum;
    m_lock.unlock();
    return busyNum;
}


// 工作线程任务函数
void* ThreadPool::worker(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);//transform arg to ThreadPool category
    // try to work
    while (!pool->m_shutdown)
    {
        
        std::cout << "thread " << std::this_thread::get_id() << " waiting..." << std::endl;
        sem_wait(&pool->m_notEmpty); // 判断任务队列是否为空, 如果为空工作线程阻塞
        pool->m_lock.lock();// 访问任务队列(共享资源)加锁      

        if (pool->m_exitNum > 0)// 判断是否要销毁线程
            {
                std::cout << "current thread need to be deleted"<<std::endl;
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum)
                {
                    pool->m_aliveNum--;
                    pool->m_lock.unlock();
                    sem_post(&pool->m_notEmpty);
                    pool->threadExit(pool);//become detach
                    return nullptr;//real exit wait for recyle
                }
            }
        // 判断线程池是否被关闭了
        if (pool->m_shutdown)
        {
            std::cout << "Thread Pool Shut Down!"<<std::endl;
            if(pool==nullptr){
                std::cout<<"pool is nullpter!"<<std::endl;
            }
            pool->m_lock.unlock();
            sem_post(&pool->m_notEmpty);
            pool->threadExit(pool);
            return nullptr;
        }

        // 从任务队列中取出一个任务
        Task task = pool->m_taskQ->getTask();
        if(task.arg==nullptr){
            std::cout << "Task Got: arg is nullptr!"<<std::endl;
        }
        if(task.function==nullptr){
             std::cout << "Task Got:function is nullptr!"<<std::endl;
        }
        // 工作的线程+1
        pool->m_busyNum++;
        // 线程池解锁
        pool->m_lock.unlock();

        // 执行任务
        std::cout << "thread " << std::this_thread::get_id() << " start working..." << std::endl;  
        task.function(task.arg);//something wrong here it seems arg is a bad ptr
        std::cout << "thread " << std::this_thread::get_id() << " work end..." << std::endl;
    
        delete static_cast<int*>(task.arg);//point to a specific space of heap
        task.arg = nullptr;//make it be a nullptr

        pool->m_lock.lock();
        pool->m_busyNum--;
        pool->m_lock.unlock();
    }
    return nullptr;
}


// 管理者线程任务函数
void* ThreadPool::manager(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    // 如果线程池没有关闭, 就一直检测
    while (!pool->m_shutdown)
    {
        // 每隔5s检测一次
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // 取出线程池中的任务数和线程数量
        //  取出工作的线程池数量
        pool->m_lock.lock();
        int queueSize = pool->m_taskQ->getTaskNum();
        int liveNum = pool->m_aliveNum;
        int busyNum = pool->m_busyNum;
        pool->m_lock.unlock();
        // 创建线程
        const int NUMBER = 2;
        // 当前任务个数>存活的线程数 && 存活的线程数<最大线程个数
        if (queueSize > liveNum && liveNum < pool->m_maxNum)
        {
            // 线程池加锁
            pool->m_lock.lock();
            int num = 0;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER
                && pool->m_aliveNum < pool->m_maxNum; ++i)
            {
                if (!pool->workThreads[i].joinable())//if there is space to create new thread
                {
                    std::thread t(worker,pool);//this t obj inlcludes thread's message,in stack
                    std::cout << "创建子线程, ID: " << t.get_id() << std::endl;
                    pool->workThreads[i]=std::move(t);
                    num++;
                    pool->m_aliveNum++;
                }
            }
            pool->m_lock.unlock();
        }

        // 销毁多余的线程
        // 忙线程*2 < 存活的线程数目 && 存活的线程数 > 最小线程数量
        if (busyNum * 2 < liveNum && liveNum > pool->m_minNum)
        {
            pool->m_lock.lock();
            pool->m_exitNum = NUMBER;
            pool->m_lock.unlock();
            // for (int i = 0; i < NUMBER; ++i)
            // {
            //     sem_post(&pool->m_notEmpty);
            // }
        }
    }
    return nullptr;
}

// 线程退出
void ThreadPool::threadExit(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    std::thread::id tid = std::this_thread::get_id();
    for (int i = 0; i < m_maxNum; ++i)
    {
        if (pool->workThreads[i].get_id() == tid)
        {
            std::cout << "threadExit() function: thread " <<tid << " exiting..." << std::endl;
            pool->workThreads[i].detach();
        }
    }
}