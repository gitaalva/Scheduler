#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <future>
#include "Task.hpp"

using namespace std::chrono;
class Scheduler {
    // all threading primitives
    std::mutex mu;
    std::condition_variable cv;
    std::condition_variable non_empty_wake_up;
    // not sure if we need this now
    std::vector<std::future<void>> pending_futures;

    using time_point_t = time_point<steady_clock, seconds>;
    std::priority_queue<Task,std::vector<Task>, std::greater<Task> > taskQueue;
    std::unordered_set<Task_Id> cancelledTasks;
    std::unordered_map<Task_Id,Time_Point::duration> modifiedTasks;
public:
    // default constructor
    Scheduler();

    // add a task to the scheduler
    void addTask(Task task);

    // remove a task from the scheduler
    void cancelTask(Task_Id taskId);

    // modify the schedule of a particular task that has been scheduled
    void modifyTask(Task_Id taskId, Time_Point::duration newRepeatTime);

    // stop the scheduler from running
    void stopScheduler();
};
#endif
