#include <iostream>
#include <sqlite3.h>
#include <chrono>
#include "boost/asio.hpp"
#include "Scheduler.hpp"
#include "Task.hpp"
#include <thread>
#include <boost/bind.hpp>
#include <istream>
#include <iostream>
#include <ostream>

using boost::asio::ip::icmp;
using boost::asio::deadline_timer;
namespace posix_time = boost::posix_time;

duration<double> tcpConnectToGoogle () {
    using namespace boost::asio;
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

        duration<double> diff = finish-start;
        std::cout << "Time Elapsed while connecting to google every 60 seconds " <<
                      diff.count() << std::endl;
        return diff;
    } catch (std::exception& e) {
        std::cerr << "Exception while trying to contact google server" << std::endl;
        std::cerr << e.what() << std::endl;
        throw;
    }
}

void icmpPing() {
    using namespace boost::asio;
    io_service io_service;
    ip::icmp::resolver resolver(io_service);
    ip::icmp::resolver::query query(icmp::v4(),"google.com", "");
    ip::icmp::resolver::iterator iter = resolver.resolve(query);

    //std::cout << "Time Elapsed " << diff.count() << std::endl;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

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
            std::cout << "Error while creating task_avg_metrics" << std::endl;
        } else {
            std::cout << "Table task_avg_metrics create succes" << std::endl;
        }
    }
    sqlite3_close(db);
}

int main() {

    int thread_safe = sqlite3_threadsafe();
    if (thread_safe == 0) {
        std::cout << "sqlite is not threadsafe" << std::endl;
        abort();
    } else {
        std::cout << "sqlite is threadsafe" << std::endl;
    }

    // initialize the database in the beginning
    initialize_database();
    /*
    sqlite3 *db;
    int rc = sqlite3_open("/Users/abhyudaya/thousandeyes/scheduler/database/stats.db", &db);
    if (rc) {
        std::cout << "Failed to open database" << std::endl;
    } else {
        std::cout << "Successfully opened database" << std::endl;
    }
    */
    Scheduler s1;
    s1.run();

    // google
    auto f1 = [](){
                auto start = std::chrono::steady_clock::now();
                std::cout << "Saying what after 35 seconds" << std::endl;
                auto end = std::chrono::steady_clock::now();
                return end-start;
              };
    auto f2 = [](){
               auto start = std::chrono::steady_clock::now();
               std::cout << "Saying what after 10 seconds" << std::endl;
               auto end = std::chrono::steady_clock::now();
               return end-start;
              };


    Task t1(1,Time_Point::duration(35),f1,update_database);
    Task t2(2,Time_Point::duration(10),f2,update_database);
    Task t3(3,Time_Point::duration(60),&tcpConnectToGoogle,update_database);

    s1.addTask(t1);
    s1.addTask(t2);
    s1.addTask(t3);

    std::cout << "Sleeping for 30 seconds" << std::endl;
    std::this_thread::sleep_for(seconds(30));
    std::cout << "Waking up after 30 seconds and let us cancel t1" << std::endl;
    /*
    s1.cancelTask(t1.getTaskId());
    std::this_thread::sleep_for(seconds(30));
    s1.cancelTask(t2.getTaskId());
    std::this_thread::sleep_for(seconds(30));
    s1.cancelTask(t3.getTaskId());
    */
    std::cout << "Calling s1.stop()" << std::endl;
    s1.stop();
}
