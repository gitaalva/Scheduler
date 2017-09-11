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
    std::condition_variable allocator_cv;
    std::vector<std::future<void>> pending_futures;

    std::thread worker;
    std::thread allocator;
    bool stopScheduler;
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

    void jobWorker();

    void jobAllocator();

    // stop the scheduler from running
    void stop();

    void run();

    inline Time_Point getCurrentTimeInSeconds();

    ~Scheduler();
};
#endif
