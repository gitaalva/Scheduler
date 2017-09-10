#include <iostream>
#include <thread>
#include "Scheduler.hpp"
#include "thread_guard.hpp"


Scheduler::Scheduler() {
    std::cout << "Constructor" << std::endl;
}

void
Scheduler::run() {
    allocator = std::thread([&]() { jobAllocator(); });
    worker = std::thread([&](){ jobWorker(); });
}

void
Scheduler::addTask(Task task) {
    std::unique_lock<std::mutex> lk((mu));
    if (!taskQueue.empty()) {
        auto top = taskQueue.top();
        taskQueue.push(task);
        // asynchronously wake up worker if the timeOut is less than
        // the timeOut in the next thread
        if (top.getNextExecuteTime() > task.getNextExecuteTime()) {
            auto f = std::async (std::launch::async,[&]() {
                std::cout << "Sleeping for a while" << std::endl;
                std::this_thread::sleep_until(task.getNextExecuteTime());
                std::cout << "Done sleeping notify worker" << std::endl;
                cv.notify_one();
            });
        }
    } else {
        taskQueue.push(task);
        lk.unlock();
        non_empty_wake_up.notify_one();
        return;
    }
}

// worker that will do the actual tasks
void
Scheduler::jobWorker() {
    while (true) {
        std::unique_lock<std::mutex> locker(mu);
        cv.wait(locker,[&]() {
            return (!taskQueue.empty() &&
             taskQueue.top().getNextExecuteTime() <= getCurrentTimeInSeconds());
        });

        Task toExecute = taskQueue.top();
        taskQueue.pop();
        toExecute.updateTime();
        locker.unlock();

        toExecute.execute();
        addTask (toExecute);
    }
}

// allocator that allocates objects
void
Scheduler::jobAllocator() {
    while (true) {
        std::unique_lock<std::mutex> lk((mu));
        if (taskQueue.empty()) {
            non_empty_wake_up.wait(lk, [&]() {return !taskQueue.empty();});
        }
        auto top = taskQueue.top();
        lk.unlock();
        std::this_thread::sleep_until(top.getNextExecuteTime());
        cv.notify_one();
    }
}

inline Time_Point
Scheduler::getCurrentTimeInSeconds()
{
    return time_point_cast<time_point_t::duration> (steady_clock::now());
}
// remove a task from the scheduler
void
Scheduler::cancelTask(Task_Id taskId) {
    std::cout << "removeTask()::begin()" << std::endl;
}

void
Scheduler::modifyTask(Task_Id taskId, Time_Point::duration newRepeatTime) {
    std::cout << "cancelTask()::begin()" << std::endl;
}

// stop the scheduler from running
void
Scheduler::stopScheduler() {
    std::cout << "stopScheduler" << std::endl;
}

Scheduler::~Scheduler() {
    worker.join();
    allocator.join();
}
