#include "Task.hpp"
#include <iostream>

namespace sch {
Task::Task (const int& taskId, const seconds& repeatSeconds,
            std::function<void(Task)> task):taskId (taskId),
                                        repeatSeconds(repeatSeconds),
                                        task(task) {
    time = time_point_cast<Time_Point::duration> (steady_clock::now());

}


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
    //std::cout << "updateTime()::id and previous time was " <<
    //            taskId << " " << time.time_since_epoch().count() << std::endl;
    time += repeatSeconds;
    //std::cout << "updateTime()::id and next time is " << taskId <<
    //            " " << time.time_since_epoch().count() << std::endl;
}

// modify the repeat interval
void
Task::modifySchedule (const Time_Point::duration &_repeatSeconds) {
    repeatSeconds = _repeatSeconds;
}

Time_Point::duration
Task::getSchedule () const {
    return repeatSeconds;
}


// execute the task passed by user;
void
Task::execute () {
    auto now = system_clock::to_time_t(system_clock::now());
    std::string log = "Time " + std::string (std::ctime(&now));
    log +=  "Running task " + std::to_string(taskId) + " every " +
            std::to_string (repeatSeconds.count()) + " seconds";

    std::cout << (log + "\n\n");
    try {
        task(*this);
    } catch (std::exception &e) {
        std::cerr << "Exception while executing tasks " << e.what() << std::endl;
    }
}
}
