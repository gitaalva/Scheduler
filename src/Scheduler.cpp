#include <iostream>
#include <thread>
#include "Scheduler.hpp"
#include "thread_guard.hpp"


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

void
Scheduler::modifyTask(const Task_Id &taskId,
                      const Time_Point::duration &newRepeatTime) {
    taskQueue->modifyTask(taskId,newRepeatTime);
}

// stop the scheduler from running
void
Scheduler::stop() {
    taskQueue->stop();
}

Scheduler::~Scheduler() {

}
