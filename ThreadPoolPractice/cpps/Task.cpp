#include "TaskQueue.h"

Task::Task(){
    function = nullptr;
    arg = nullptr;
}

Task::Task(callback f,void* arg){
    this->arg=arg;
    function=f;
}
    