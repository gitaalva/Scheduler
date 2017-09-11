#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include <queue>
#include "Task.hpp"
#include "TaskQueue.hpp"

class Scheduler {
public:

    Scheduler();

    // add a task to the scheduler
    void addTask(const Task &task);

    // remove a task from the scheduler
    void cancelTask(const Task_Id &taskId);

    // modify the schedule of a particular task that has been scheduled
    void modifyTask(const Task_Id &taskId, const Time_Point::duration &newRepeatTime);

    // start scheduling tasks
    void start();

    // stop the scheduler from running
    void stop();

    // destructor
    ~Scheduler();

private:
    TaskQueue *taskQueue;
};
#endif
