#include "TaskQueue.h"

TaskQueue::TaskQueue(size_t capa)
:capacity_(capa), que_(), mux_(), notEmpty_(), notFull_(), flag_(true){

}

TaskQueue::~TaskQueue(){

}

//添加任务
void TaskQueue::pushTask(ElemType&& task){
    unique_lock<mutex> lock(mux_);
    //判断队列是否为满
    while(full()){
        //满了就不能在入队，释放锁，阻塞等待队列不满的的信号
        //唤醒后重新拿到锁
        notFull_.wait(lock);
    }
    //队列不满则入队
    que_.push(std::move(task));
    //发送队列不空的信号
    notEmpty_.notify_one();

}

//获取数据
ElemType TaskQueue::popTask(){
    unique_lock<mutex> lock(mux_);
    //判断是否队列是否为空和是否退出，避免虚假唤醒
    while(empty() && flag_){ 
        //队列为空则无法获取任务，释放锁，阻塞等待队列不空的信号
        //被唤醒后重新拿到锁
        notEmpty_.wait(lock);
    }
    //继续任务是否可以继续工作
    if(flag_){
        //获取任务
        ElemType tmp = que_.front();    //获取数据
        //将任务出队
        que_.pop();  //删除数据
        //返回任务
        return tmp;
    }else{
        //不能则返回空
        return nullptr;
    }
}

//判空
bool TaskQueue::empty() const{
    return (0 == que_.size());
}

//判满
bool TaskQueue::full() const{
    return (capacity_ == que_.size());
}

//当线程池要退出时，需要唤醒所有正在等待的消费者，并设置任务不能继续工作
void TaskQueue::wakeup(){
    flag_ = false;
    notEmpty_.notify_all();
}