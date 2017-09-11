

//Define our Module name (prints at testing)
 #define BOOST_TEST_MODULE "SchedulerModule"


#include <iostream>
#include <iostream>
#include <sqlite3.h>
#include <chrono>
#include "boost/asio.hpp"
#include <thread>
#include <boost/bind.hpp>
#include <istream>
#include <iostream>
#include <ostream>

#include "Scheduler.hpp"
#include "Task.hpp"
//VERY IMPORTANT - include this last
#include <boost/test/unit_test.hpp>

// helper to update database
void update_database(Task_Id id, duration<double> result) {
    sqlite3 *conn;
    sqlite3_stmt* stmt = nullptr;

    int rc;

    if (sqlite3_open("../database/stats.db", &conn) == SQLITE_OK) {
        std::string statement1 = "insert into task_metrics ("\
                                  "TASKID, METRIC ) values (?, ?)";

        rc = sqlite3_exec( conn, "BEGIN TRANSACTION", 0, 0, nullptr );
        if (rc != SQLITE_OK) {
          std::cout << "could not execute transaction" << std::endl;
          std::exit(EXIT_FAILURE);
        }

        rc = sqlite3_prepare_v2( conn, statement1.c_str(), -1, &stmt, nullptr );
        if (rc != SQLITE_OK) {
            std::cout << "The error message number is " << sqlite3_errcode(conn) << std::endl;
            std::cout << "the error message is " << *sqlite3_errmsg(conn) << std::endl;
            std::cout << "could not prepare statement1" << std::endl;
            std::exit(EXIT_FAILURE);
        }


        if (sqlite3_bind_int( stmt, 1, id ) != SQLITE_OK) {
            std::cout  << "could not bind int" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (sqlite3_bind_double( stmt, 2, result.count()) != SQLITE_OK) {
            std::cout  << "could not bind double" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cout << "could not execute insert " << std::endl;
            std::exit(EXIT_FAILURE);
        }

        sqlite3_finalize(stmt);

        std::string statement2 = "select COUNT, AVG_METRIC from task_avg_metrics"\
                                 " where TASKID= ?";

        if (sqlite3_prepare_v2( conn, statement2.c_str(), -1, &stmt, 0 ) != SQLITE_OK )
         {
            std::cout << "could not prepare statement3" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (sqlite3_bind_int( stmt, 1, id ) != SQLITE_OK) {
            std::cout  << "could not bind int" << std::endl;
            std::exit(EXIT_FAILURE);
        }


        double  count = 0;
        double average = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt,0);
            average = sqlite3_column_double(stmt,1);
            average = ((count*average) + result.count())/(count+1);
            ++count;
        } else {
            count = 1;
            average = result.count();
        }
        sqlite3_finalize(stmt);

        std::string insert_statement = "replace into task_avg_metrics (TASKID, COUNT"\
                            ", AVG_METRIC) values(?,?,?)";

        if (sqlite3_prepare_v2( conn, insert_statement.c_str(), -1, &stmt, 0 ) != SQLITE_OK) {
            std::cout  << "Prepare failure" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (sqlite3_bind_int( stmt, 1, id ) != SQLITE_OK) {
            std::cout  << "could not bind int" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (sqlite3_bind_int( stmt, 2, count ) != SQLITE_OK) {
            std::cout  << "could not bind int" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (sqlite3_bind_double( stmt, 3, average) != SQLITE_OK) {
            std::cout  << "could not bind double" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cout << "could not execute replace " << std::endl;
            std::exit(EXIT_FAILURE);
        }

        sqlite3_finalize(stmt);

        rc = sqlite3_exec(conn, "END TRANSACTION;", NULL, NULL, NULL);
    }
    sqlite3_close(conn);
}

// tcp connect to google and update database
void tcpConnectToGoogle (Task task) {
    using namespace boost::asio;
    duration<double> result;
    try {
        io_service io_service;
        ip::tcp::resolver resolver(io_service);
        ip::tcp::resolver::query query("google.com", "80");
        ip::tcp::resolver::iterator iter = resolver.resolve(query);
        ip::tcp::resolver::iterator end;

        ip::tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        steady_clock::time_point start;
        steady_clock::time_point finish;
        if (iter != end) {
            socket.close();
            start = std::chrono::steady_clock::now();
            socket.connect(*iter);
            finish= std::chrono::steady_clock::now();
        }

         result = finish-start;
        std::cout << "Time Elapsed while connecting to google every 60 seconds " <<
                      result.count() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception while trying to contact google server" << std::endl;
        std::cerr << e.what() << std::endl;
        throw;
    }
    update_database (task.getTaskId(),result);
}

void initialize_database() {
    sqlite3 *db = nullptr;
    int rc;

    if (sqlite3_open("../database/stats.db", &db) == SQLITE_OK) {
        const char *sql1 = "CREATE TABLE task_metrics(" \
                          "TASKID INTEGER NOT NULL,"\
                          "METRIC REAL NOT NULL,"\
                          "TIME DATETIME DEFAULT CURRENT_TIMESTAMP);";
        std::cout << "create table command" << sql1 << std::endl;
        rc = sqlite3_exec(db, sql1, nullptr, nullptr, nullptr);

        if (rc != SQLITE_OK) {
            std::cerr << "Error while creating table task_metrics" << std::endl;
        } else {
            std::cerr << "Table task_metrics created succesfully" << std::endl;
        }

        const char *sql2 = "CREATE TABLE task_avg_metrics(" \
                          "TASKID       PRIMARY KEY     NOT NULL,"\
                          "COUNT       INTEGER    NOT NULL,"\
                          "AVG_METRIC   REAL            NOT NULL,"\
                          "TIMESTAMP    DATETIME DEFAULT CURRENT_TIMESTAMP)";
        std::cout << "create table command" << sql2 << std::endl;

        rc = sqlite3_exec(db, sql2, nullptr, nullptr, nullptr);

        if (rc != SQLITE_OK) {
            std::cerr << "Error while creating task_avg_metrics" << std::endl;
        } else {
            std::cerr << "Table task_avg_metrics create succes" << std::endl;
        }
    }
    sqlite3_close(db);
}

using TQ = std::priority_queue<Task,std::vector<Task>, std::greater<Task> >;
// ------------- Tests Follow --------------
BOOST_AUTO_TEST_CASE( constructors )
{
    bool thread_safe = sqlite3_threadsafe();
    BOOST_CHECK_MESSAGE(thread_safe,"SQLite installation is not thread safe!!");
}

// function random function
auto f1 = [](Task task) {
            auto start = std::chrono::steady_clock::now();
            int count = 0;
            int value = 0;
            for (int i=0; i < 1000000; ++i) {
                ++value;
            }
            auto end = std::chrono::steady_clock::now();
            return end-start;
};



BOOST_AUTO_TEST_CASE( addTask ) {
    Scheduler s1;
    s1.start();
    Task t1(1,Time_Point::duration(35),f1);
    Task t2(2,Time_Point::duration(10),f1);
    Task t3(3,Time_Point::duration(60),&tcpConnectToGoogle);

    s1.addTask(t1);
    s1.addTask(t2);
    s1.addTask(t3);

    // give some time for the scheduler to add task;
    std::this_thread::sleep_for(seconds(30));
    TQ taskQueue = s1.getTaskQueue();

    std::unordered_set<Task_Id> existingTasks;
    while (!taskQueue.empty()) {
        auto top = taskQueue.top();
        taskQueue.pop();
        existingTasks.insert(top.getTaskId());
    }

    bool success = true;

    for (size_t i=1; i <= 3; ++i) {
        if (existingTasks.find(i) == existingTasks.end()) {
            success = false;
            break;
        }
    }

    s1.stop();
    BOOST_CHECK_MESSAGE(success,"Some tasks not added to the queue");

}

BOOST_AUTO_TEST_CASE( modifyTask ) {

    Scheduler s1;
    s1.start();
    Task t1(1,Time_Point::duration(35),f1);
    Task t2(2,Time_Point::duration(10),f1);
    Task t3(3,Time_Point::duration(60),&tcpConnectToGoogle);

    s1.addTask(t1);
    s1.addTask(t2);
    s1.addTask(t3);

    // give some time for the scheduler to add task;
    // wait for every task to be executed once
    std::this_thread::sleep_for(seconds(30));

    // modifying Task
    std::cout << "Modifying task" << std::endl;
    s1.modifyTask(t2.getTaskId(), Time_Point::duration(80));
    std::cout << "Modifying task done" << std::endl;
    // wait for the scheduler to modify the scheduler
    // the scheduler modifies the schedule the next time the task is run
    std::this_thread::sleep_for(seconds(30));

    TQ taskQueue = s1.getTaskQueue();

    // assume that modification has failed
    bool success = false;
    while (!taskQueue.empty()) {
        auto top = taskQueue.top();
        taskQueue.pop();
        if (top.getTaskId() == t2.getTaskId() &&
             top.getSchedule() == Time_Point::duration(80)) {
                 success = true;
                 break;
             }
    }

    s1.stop();
    BOOST_CHECK_MESSAGE(success,"Task not modified within the expected time");

}

BOOST_AUTO_TEST_CASE( cancelTask ) {

    Scheduler s1;
    s1.start();
    Task t1(1,Time_Point::duration(35),f1);
    Task t2(2,Time_Point::duration(10),f1);
    Task t3(3,Time_Point::duration(60),&tcpConnectToGoogle);

    s1.addTask(t1);
    s1.addTask(t2);
    s1.addTask(t3);

    // give some time for the scheduler to add task;
    // wait for every task to be executed once
    std::this_thread::sleep_for(seconds(90));

    // modifying Task
    s1.cancelTask (t2.getTaskId());

    // wait for the scheduler to modify the scheduler
    // the scheduler modifies the schedule the next time the task is run
    std::this_thread::sleep_for(seconds(30));

    TQ taskQueue = s1.getTaskQueue();

    // assume that modification has failed
    bool success = true;
    while (!taskQueue.empty()) {
        auto top = taskQueue.top();
        taskQueue.pop();
        if (top.getTaskId() == t2.getTaskId()) {
            success = false;
        }
    }

    s1.stop();
    BOOST_CHECK_MESSAGE(success,"Task not cancelled within expected time");

}
