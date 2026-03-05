#include <thread>
#include <chrono>
#include <cstdio>
#include "ThreadPool.h"
#include <string>
#include <iostream>

void taskFunc(void* arg)
{
    int num = *(int*)arg;
    std::cout<<"thread"<< std::this_thread::get_id()<< "is working, number ="<<std::to_string(num)<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main()
{
    // 创建线程池
    ThreadPool* pool = new ThreadPool(10,100);
    for (int i = 0; i < 100; ++i)
    {
        int* num = new int;
        *num = i + 100;
        pool->addTask(taskFunc,num);
    }
    std::this_thread::sleep_for(std::chrono::seconds(30));
    delete pool;
    pool=nullptr;
    return 0;
}


