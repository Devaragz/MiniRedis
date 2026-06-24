#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<iostream>
#include<vector>
#include<queue>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<future>
#include<memory>
#include<atomic>
#include<chrono>
#include<type_traits>
#include<stdexcept>

// Global console lock
extern std::mutex consolemtx;

// Priorities
enum class TaskPriority{
    LOW=1,
    MEDIUM=5,
    HEAVY=10,
    URGENT=20
};

// PQ Task wrapper
struct TaskElement{
    TaskPriority prior;
    std::function<void()> task;

    // Max-Heap comparator
    bool operator<(const TaskElement& other) const{
        return static_cast<int>(prior)<static_cast<int>(other.prior);
    }
};

//  The Engine
class ThreadPool{
private:
    std::vector<std::thread> workers;
    std::priority_queue<TaskElement> tasks; //Thread safe PQ

    std::mutex que_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

public:
    ThreadPool(size_t threads);
    ~ThreadPool();

    //Template enqueue
    template<class F, class... Args>
    auto enqueue(TaskPriority prior,F&& f,Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type=typename std::result_of<F(Args...)>::type;
        auto task=std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
        );
        std::future<return_type>res=task->get_future();
        {
            std::unique_lock<std::mutex> lock(que_mutex);
            if(stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.push({prior,[task](){(*task)();}});
        }
        condition.notify_one();
        return res;
    }
};

#endif