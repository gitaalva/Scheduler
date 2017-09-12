#include <iostream>
#include <thread>
#include "Scheduler.hpp"
#include "thread_guard.hpp"

namespace sch {
Scheduler::Scheduler():taskQueue(new TaskQueue()) {

}

void
Scheduler:: start() {
    taskQueue->run();
}

void
Scheduler::addTask(const Task &task) {
    taskQueue->addTask(task);
}

void
Scheduler::cancelTask(const Task_Id &taskId) {
    taskQueue->cancelTask(taskId);
}

// modify the schedule of a task
void
Scheduler::modifyTask(const Task_Id &taskId,
                      const Time_Point::duration &newRepeatTime) {
    taskQueue->modifyTask(taskId,newRepeatTime);
}

std::priority_queue<Task,std::vector<Task>, std::greater<Task> >
Scheduler::getTaskQueue() {
    return taskQueue->getTaskQueue();
}
// stop the scheduler from running
void
Scheduler::stop() {
    taskQueue->stop();
}

Scheduler::~Scheduler() {
    taskQueue->stop();
}

} // namespace sch
