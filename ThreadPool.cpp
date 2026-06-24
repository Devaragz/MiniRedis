#include "ThreadPool.h"
#include<windows.h> 

// Constructor->spawns worker threads
ThreadPool::ThreadPool(size_t threads) : stop(false){
    for(size_t i=0;i<threads;i++){
        workers.emplace_back([this,i] {
            DWORD_PTR mask=(1ULL<<i);
            SetThreadAffinityMask(GetCurrentThread(),mask);
            for(;;){
                TaskElement task_elm;
                {
                    std::unique_lock<std::mutex> lock(this->que_mutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });

                    if(this->stop && this->tasks.empty()){return;}

                    task_elm=std::move(this->tasks.top());
                    this->tasks.pop();
                }
                task_elm.task(); //  Execute task
            }
        });
    }
}

// Destructor
ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(que_mutex);
        stop=true;
    }
    condition.notify_all();
    for(std::thread &worker : workers){
        worker.join();
    }
}


