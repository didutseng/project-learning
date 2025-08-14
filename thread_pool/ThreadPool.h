#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include "TaskQueue.h"

#include <vector>
#include <thread>
#include <functional>

using std::vector;
using std::thread;
using std::function;

using Task = function<void()>;

class ThreadPoll{
public:
    ThreadPoll(size_t threadNum, size_t capa);
    ~ThreadPoll();

    //线程的启动
    void start();
    //线程的停止
    void stop();
    //添加任务
    void addTask(Task&& task);

private:
    //获取任务
    Task getTask();
    //线程入口函数，线程池交给工作线程执行任务
    void doTask();

private:
    size_t threadNum_;  //子线程数目
    vector<thread> threads_;    //存放子线程的容器
    size_t capacity_;   //任务队列大小
    TaskQueue taskQue_; //任务队列
    bool isExit_;   //线程退出标志
};

#endif