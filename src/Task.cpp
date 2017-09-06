#include "Task.hpp"
#include <iostream>

Task::Task (const int& taskId, const seconds& repeatSeconds,
            std::function<void()>& taskMethod,
            std::function<void()> taskCompleteCallBack )
                :taskId(taskId), repeatSeconds(repeatSeconds),
                taskMethod(taskMethod),
                taskCompleteCallBack (taskCompleteCallBack) {

}

// comparator for std::priority_queue;
bool
Task:: operator>(const Task &other) const {
    return time > other.time;
}

// set the next executable time;
Time_Point
Task::getNextExecuteTime() const {
        return time;
}

void
Task::updateTime() {
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
