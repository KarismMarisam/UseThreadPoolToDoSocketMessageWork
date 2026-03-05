#include <queue>
#include <mutex>
using callback=void (*)(void* arg);//used to declare a ptr to a fuction returning void and needing void* parameter

class Task{
    public:
        Task();
        Task(callback f,void* arg);
        callback function;
        void* arg;
};//In C++,struct euqals a class which take public default

class TaskQueue{
    public:
        // TaskQueue();
        // ~TaskQueue();
        void addTask(Task task);
        void addTask(callback f,void *arg);
        Task getTask();
        int getTaskNum();
    private:
    std::mutex m_mutex;
    std::queue<Task> m_taskQ;
};