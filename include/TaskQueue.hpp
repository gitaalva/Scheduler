#ifndef TaskQueue_HPP
#define TaskQueue_HPP
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <future>
#include "Task.hpp"

using namespace std::chrono;
class TaskQueue {
    // mutex to synchronize access to priorityQueue
    std::mutex mu;

    // conditional variable for the job allocator to notify worker
    std::condition_variable cv;

    // to notify the job allocator when it is sleeping
    // if  a new task is added or if it has to shut down
    std::condition_variable allocator_cv;

    std::thread worker;
    std::thread allocator;
    bool stopScheduler;

    // priorityQueue to get the next task;
    std::priority_queue<Task,std::vector<Task>, std::greater<Task> > taskQueue;

    // set that holds the list of tasks that should be cancelled;
    std::unordered_set<Task_Id> cancelledTasks;

    // tasks in priorityQueue whose schedule has to be modified
    std::unordered_map<Task_Id,Time_Point::duration> modifiedTasks;
public:
    // default constructor
    TaskQueue();

    // add a task to the TaskQueue
    void addTask(Task task);

    // remove a task from the TaskQueue
    void cancelTask(Task_Id taskId);

    // modify the schedule of a particular task that has been scheduled
    void modifyTask(Task_Id taskId, Time_Point::duration newRepeatTime);

    void jobWorker();

    void jobAllocator();

    // stop the TaskQueue from running
    void stop();

    void run();

    inline Time_Point getCurrentTimeInSeconds();

    ~TaskQueue();
};
#endif
