#include <iostream>
#include <sqlite3.h>
#include <chrono>
#include "boost/asio.hpp"
#include <thread>
#include <boost/bind.hpp>
#include <istream>
#include <iostream>
#include <ostream>

#include "Ping.hpp"
#include "Scheduler.hpp"
#include "Task.hpp"

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;
namespace posix_time = boost::posix_time;

void update_database(Task_Id id, duration<double> value) {
    std::chrono::milliseconds result = std::chrono::duration_cast<std::chrono::milliseconds>(value);
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
            std::cout << "old average " << average << " sample count " << count << std::endl;
            average = ((count*average) + result.count())/(count+1);
            std::cout << "new val " << result.count() << std::endl;
            std::cout << "new avg " << average << std::endl;
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
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(result);
        std::cout << "Time Elapsed while sending tcp packet to google " <<
                      ms.count() << "ms" << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Exception while trying to contact google server" << std::endl;
        std::cerr << e.what() << std::endl;
        throw;
    }
    update_database (task.getTaskId(),result);
}

void icmpPing (Task task) {
    duration<double> result;
    try {
        boost::asio::io_service io_service;
        pinger p(io_service, "google.com");
        io_service.run();
        result = p.getDuration();
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(result);
        std::cout << "Time Elapsed while sending icmp packet to google " <<
                      ms.count() << "ms" << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception while sending icmp packets to google" << std::endl;
        std::cerr << e.what() << std::endl;
        throw;
    }

    update_database(task.getTaskId(),result);
}


void initialize_database() {
    sqlite3 *db;
    int rc;

    if (sqlite3_open("../database/stats.db", &db) == SQLITE_OK) {
        const char *sql1 = "CREATE TABLE task_metrics(" \
                          "TASKID INTEGER NOT NULL,"\
                          "METRIC REAL NOT NULL,"\
                          "TIME DATETIME DEFAULT CURRENT_TIMESTAMP);";
        std::cout << "create table command" << sql1 << std::endl;
        rc = sqlite3_exec(db, sql1, nullptr, nullptr, nullptr);

        if (rc != SQLITE_OK) {
            std::cout << "Error while creating table task_metrics" << std::endl;
        } else {
            std::cout << "Table task_metrics created succesfully" << std::endl;
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


int main() {

    int thread_safe = sqlite3_threadsafe();
    if (thread_safe == 0) {
        std::clog << "sqlite is not threadsafe" << std::endl;
        abort();
    } else {
        std::clog << "sqlite is threadsafe" << std::endl;
    }

    // initialize the database in the beginning
    initialize_database();

    Scheduler s1;
    s1.start();

    Task t1(3,Time_Point::duration(20),&tcpConnectToGoogle);
    Task t2(4,Time_Point::duration(30),&icmpPing);

    s1.addTask(t1);
    s1.addTask(t2);

    std::cout << "Sleeping for 100 seconds" << std::endl;
    std::this_thread::sleep_for(seconds(100));

    std::cout << "Modifying tcp to google schedule from every 20 sec to 60 sec\n\n\n" << std::endl;
    s1.modifyTask(t1.getTaskId(),Time_Point::duration(60));

    std::this_thread::sleep_for(seconds(200));
    std::clog << "Calling s1.stop()" << std::endl;
    s1.stop();
}
