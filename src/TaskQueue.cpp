#include <iostream>
#include <thread>
#include "TaskQueue.hpp"
#include "thread_guard.hpp"


TaskQueue::TaskQueue():stopScheduler(false) {
}

void
TaskQueue::run() {
    allocator = std::thread([&]() { jobAllocator(); });
    worker = std::thread([&](){ jobWorker(); });
}

void
TaskQueue::addTask(Task task) {
    std::unique_lock<std::mutex> lk((mu));
    if (!taskQueue.empty()) {
        auto top = taskQueue.top();
        taskQueue.push(task);
        // wake up allocator if the new task has to executed
        // before the timeout set in allocator;
        if (top.getNextExecuteTime() > task.getNextExecuteTime()) {
            allocator_cv.notify_one();
        }
    } else {
        taskQueue.push(task);
        lk.unlock();
        allocator_cv.notify_one();
        return;
    }
}

// worker that will do the actual tasks
void
TaskQueue::jobWorker() {
    while (true) {
        std::unique_lock<std::mutex> locker(mu);
        cv.wait(locker,[&]() {
            return (stopScheduler || (!taskQueue.empty() &&
             taskQueue.top().getNextExecuteTime() <= getCurrentTimeInSeconds()));
        });
        if (stopScheduler) {
            //std::cerr << "Exiting job worker" << std::endl;
            break;
        }
        Task toExecute = taskQueue.top();
        taskQueue.pop();
        // check if the task is cancelled, else perform the task
        const Task_Id &taskId = toExecute.getTaskId();
        if (cancelledTasks.find(taskId) == cancelledTasks.end()) {
            if (modifiedTasks.find(taskId) != modifiedTasks.end()) {
                std::cout << "TaskID " + std::to_string(taskId) + "schedule "
                  + "modified to "+std::to_string (modifiedTasks[taskId].count())
                  +"\n";
                toExecute.modifySchedule(modifiedTasks[taskId]);
            }
            toExecute.updateTime();
            locker.unlock();

            toExecute.execute();
            addTask (toExecute);
        } else {
            // tasks won't be read into queue and hence we can delete
            // it from cancelledTasks. Allows for adding the task again after
            // cancelling it.
            std::cout << "Task has been cancelled" + std::to_string(toExecute.getTaskId()) << std::endl;
            cancelledTasks.erase (toExecute.getTaskId());
            locker.unlock();
        }
    }
}

void
TaskQueue::jobAllocator() {
    while (true) {
        std::unique_lock<std::mutex> lk((mu));
        // if there is no task in the queue then wait until a day a stop signal
        // is received or a new item is added;
        if (taskQueue.empty()) {
            allocator_cv.wait_until(lk, getCurrentTimeInSeconds()+seconds(86400),
                                    [&]() {
                                        return !taskQueue.empty() ||
                                        stopScheduler;
                                    });
        }

        if (stopScheduler)  {
            //std::cerr << "Stopping Job Allocator" << std::endl;
            break;
        }

        if (!taskQueue.empty()) {
            auto waitTime = taskQueue.top().getNextExecuteTime();
            allocator_cv.wait_until(lk,waitTime,[&]() {
                return stopScheduler ||
                        taskQueue.top().getNextExecuteTime() < waitTime;
                });
            if (stopScheduler)  {
                //std::cerr << "Stopping Job Allocator" << std::endl;
                break;
            }
            if (taskQueue.top().getNextExecuteTime() > getCurrentTimeInSeconds())
                continue;
            // notify the worker to perform the tasks
            cv.notify_one();
        }
        lk.unlock();
    }
}

inline Time_Point
TaskQueue::getCurrentTimeInSeconds()
{
    return time_point_cast<Time_Point::duration> (steady_clock::now());
}

// remove a task from the TaskQueue
void
TaskQueue::cancelTask(Task_Id taskId) {
    std::lock_guard<std::mutex> lk(mu);
    std::cout << "cancelTask()::begin()" << std::endl;
    cancelledTasks.insert(taskId);
}

// modify the schedule of currently running task
void
TaskQueue::modifyTask(Task_Id taskId, Time_Point::duration newRepeatTime) {
    std::lock_guard<std::mutex> lk(mu);
    std::cout << "cancelTask()::begin()" << std::endl;
    modifiedTasks[taskId] = newRepeatTime;
}

// function provided for testing purposes
std::priority_queue<Task,std::vector<Task>, std::greater<Task> >
TaskQueue::getTaskQueue() {
    std::lock_guard<std::mutex> lk(mu);
    return taskQueue;
}

// stop the TaskQueue from running
void
TaskQueue::stop() {
    std::cout << "stopTaskQueue" << std::endl;
    std::lock_guard<std::mutex> lk(mu);
    stopScheduler = true;
    allocator_cv.notify_one();
    cv.notify_one();
}

TaskQueue::~TaskQueue() {
    worker.join();
    allocator.join();
}
