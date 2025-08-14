#include "ThreadPool.h"

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <memory>
#include <functional>

using std::cout;
using std::endl;
using std::unique_ptr;
using std::bind;

class MyTask{
public:
    void process(){
        srand(clock());
        int number = rand()%100;
        cout << "ThreadPooll.MyTask number = " << number << endl;  
    }
};

void test(){
    unique_ptr<MyTask> ptask(new MyTask());
    ThreadPoll pool(4, 10);
    pool.start();
    int cnt = 20;
    while(cnt--){
        pool.addTask(bind(&MyTask::process, ptask.get()));
        //为了输出美观，实际项目中不建议用
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        cout << "cnt = " << cnt << endl;
    }
    pool.stop();
}

int main(int argv, char* argc[]){
    test();
    return 0;
}