#include <iostream>
#include "Scheduler.hpp"

Scheduler::Scheduler() {
    std::cout << "Constructor" << std::endl;
}

void
Scheduler::addTask(Task task) {
    std::unique_lock<std::mutex> lk(mu);
    if (!pQ.empty()) {
        auto top = pQ.top();
        pQ.push(task);
        // asynchronously wake up worker if the timeOut is less than
        // the timeOut in the next thread
        if (top.getTimePoint() > task.getTimePoint()) {
            auto f = std::async (std::launch::async,[&]() {
                std::cout << "Sleeping for a while" << std::endl;
                std::this_thread::sleep_until(task.getTimePoint());
                std::cout << "Done sleeping notify worker" << std::endl;
                cv.notify_one();
            });
        }
    } else {
        pQ.push(task);
        lk.unlock();
        non_empty_wake_up.notify_one();
        return;
    }
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
