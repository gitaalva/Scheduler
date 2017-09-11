#include "Task.hpp"
#include <iostream>


Task::Task (const int& taskId, const seconds& repeatSeconds,
            std::function<duration<double>()> taskMethod,
            std::function<void(Task_Id,duration<double>)> taskCompleteCallBack )
                :taskId(taskId), repeatSeconds(repeatSeconds),
                taskMethod(taskMethod),
                taskCompleteCallBack (taskCompleteCallBack) {
    time = time_point_cast<Time_Point::duration> (steady_clock::now())
            + repeatSeconds;
}

const Task_Id
Task:: getTaskId () const {
    return taskId;
}
// comparator for std::priority_queue;
bool
Task:: operator> (const Task &other) const {
    return time > other.time;
}

// get the next time point when this task will get executed
Time_Point
Task::getNextExecuteTime () const {
    return time;
}


void
Task::updateTime () {
    std::cout << "updateTime()::id and previous time was" <<
                taskId << " " << time.time_since_epoch().count() << std::endl;
    time += repeatSeconds;
    std::cout << "updateTime()::id and next time is" << taskId <<
                " " << time.time_since_epoch().count() << std::endl;
}

// modify the repeat interval
void
Task::modifySchedule (const Time_Point::duration &newRepeatSeconds) {
    repeatSeconds = newRepeatSeconds;
}

// execute the task passed by user;
void
Task::execute () {
    try {
        duration<double> result = taskMethod();
        taskCompleteCallBack(taskId,result);
    } catch (std::exception &e) {
        std::cerr << "Exception while executing tasks " << e.what() << std::endl;
    }
}
