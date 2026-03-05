#include "TaskQueue.h"
#include <iostream>

void TaskQueue::addTask(Task task){
    m_mutex.lock();
    m_taskQ.push(task);
    m_mutex.unlock();
}

void TaskQueue::addTask(callback f,void *arg){
    m_mutex.lock();
    m_taskQ.push(Task(f,arg));
    m_mutex.unlock();
}

Task TaskQueue::getTask(){
    Task t;
    m_mutex.lock();
    if(!m_taskQ.empty()){
        t=m_taskQ.front();
        m_taskQ.pop();
    }
    else{
         std::cout << "taskQueue is empty!"<<std::endl;
    }
    m_mutex.unlock();
    return t;
}

int TaskQueue::getTaskNum(){
    return static_cast<int>(m_taskQ.size());//actually return long unsigned int 
}