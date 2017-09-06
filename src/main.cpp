#include <iostream>
#include <sqlite3.h>

#include "Scheduler.hpp"
int main() {
    sqlite3 *db;
    int rc = sqlite3_open("/Users/abhyudaya/thousandeyes/scheduler/database/stats.db", &db);
    if (rc) {
        std::cout << "Failed to open database" << std::endl;
    } else {
        std::cout << "Successfully opened database" << std::endl;
    }
    std::cout << "Hello Thousand Eyes are watching you!!!" << std::endl;
}
