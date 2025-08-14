#include "ThreadPool.h"
#include <iostream>

using std::cout;
using std::endl;

ThreadPoll::ThreadPoll(size_t threadNum, size_t capa)
:threadNum_(threadNum), threads_(), capacity_(capa), taskQue_(capacity_), isExit_(false){

}
    
ThreadPoll::~ThreadPoll(){

}

//线程的启动
void ThreadPoll::start(){
    for(size_t i = 0; i < threadNum_; ++i){
        //创建线程并启动，将其放入vector中
        threads_.push_back(thread(&ThreadPoll::doTask, this));
    }
}

//线程的停止
void ThreadPoll::stop(){
    //判断任务队列是否为空
    //不为空就继续等待执行结束
    while(!taskQue_.empty()){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    //更改线程池退出标志位
    isExit_ = true;
    //将所有等待在notEmpty_条件变量上的线程唤醒
    taskQue_.wakeup();
    //等待子线程执行结束
    for(auto& th : threads_){
        th.join();
    }
}

//添加任务
void ThreadPoll::addTask(Task&& task){
    if(task){
        taskQue_.pushTask(std::move(task));
    }
}

//获取任务
Task ThreadPoll::getTask(){
    return taskQue_.popTask();
}

//线程入口函数，线程池交给工作线程执行任务
void ThreadPoll::doTask(){
    while(!isExit_){
        Task taskcd = getTask();
        //检查任务是否为空
        if(taskcd){
            //执行任务
            taskcd();   //回调函数
        }else{
            //空任务警告
            cout << "task is NULL!" << endl;
        }
    }
}