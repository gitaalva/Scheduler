#ifndef TASK_HPP
#define TASK_HPP
#include <chrono>
#include <functional>

using namespace std::chrono;
typedef time_point<steady_clock, seconds> Time_Point;
typedef unsigned int Task_Id;


class Task {

    // id for the task
    Task_Id                 taskId;
    Time_Point              time;
    Time_Point::duration    repeatSeconds;
    std::function<void()>   taskMethod;
    std::function<void()>   taskCompleteCallBack;

public:

    Task (const int& taskId, const seconds& repeatSeconds,
    std::function<void()>& taskMethod,
    std::function<void()> taskCompleteCallBack );

    // comparator for std::priority_queue;
    bool operator>(const Task &other) const;

    // set the next executable time;
    Time_Point getNextExecuteTime() const;

    // updates the time to next execute based on repeat
    void updateTime();

    // modify the repeat interval
    void modifySchedule (const Time_Point::duration &newRepeatSeconds);

};
#endif
