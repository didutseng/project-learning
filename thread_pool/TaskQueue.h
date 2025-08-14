#ifndef _TASKQUEUE_H_
#define _TASKQUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

using std::queue;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::function;

using ElemType = function<void()>;

class TaskQueue{
public:
    TaskQueue(size_t capa);
    ~TaskQueue();

    //添加任务
    void pushTask(ElemType&& task);
    //获取数据
    ElemType popTask();
    //判空
    bool empty() const;
    //判满
    bool full() const;
    //唤醒所有在条件变量notEmpty_上睡觉的线程
    void wakeup();

private:
    size_t capacity_;   //任务队列大小（容量）
    queue<ElemType> que_;   //存放数据的数据结构
    mutex mux_; //访问共享数据的互斥锁
    condition_variable notEmpty_;   //非空条件变量
    condition_variable notFull_;    //非满条件变量
    bool flag_; //标识任务是否能继续工作
};

#endif