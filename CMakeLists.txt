cmake_minimum_required (VERSION 2.6)
project(Scheduler)

set (CMAKE_CXX_STANDARD 11)
#Bring the headers, such as Student.h into the project
include_directories(include)

#set the cmake_module_path to find sqlite3 library

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#Can manually add the sources using the set command as follows:
#set(SOURCES src/main.cpp)




# END OF LIBRARY

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

add_executable(Scheduler ${SOURCES})

#find and link sqlite3
find_package (sqlite3)
if (SQLITE3_FOUND)
  include_directories(${SQLITE3_INCLUDE_DIRS})
  target_link_libraries (Scheduler ${SQLITE3_LIBRARIES})
endif (SQLITE3_FOUND)

#find and link boost unit testing framework
find_package(Boost 1.60 COMPONENTS filesystem unit_test_framework system REQUIRED system)
add_definitions(-DBOOST_TEST_DYN_LINK)


INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
target_link_libraries(Scheduler ${Boost_SYSTEM_LIBRARY})




#FOR BOOST TESTING FRAMEWORK
# WE ARE GOING TO BUILD A LIBRARY HERE
set (LIBSOURCES src/Scheduler.cpp src/Task.cpp src/TaskQueue.cpp)
#add_library(TaskScheduler ${LIBSOURCES})

#Setup CMake to run tests
enable_testing()

#Prep ourselves for compiling boost
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories (${Boost_INCLUDE_DIRS})

#I like to keep test files in a separate source directory called test
file(GLOB TEST_SRCS RELATIVE ${CMAKE_SOURCE_DIR} test/*.cpp)

#Run through each source
foreach(testSrc ${TEST_SRCS})
        #Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        #Add compile target
        add_executable(${testName} ${testSrc} ${LIBSOURCES})

        #link to Boost libraries AND your targets and dependencies
        target_link_libraries(${testName}
                             ${Boost_LIBRARIES} ${SQLITE3_LIBRARIES}
                             ${Boost_SYSTEM_LIBRARY})

        #I like to move testing binaries into a testBin directory
        set_target_properties(${testName} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/testBin)

        #Finally add it to test execution -
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName}
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/testBin
                 COMMAND ${CMAKE_SOURCE_DIR}/testBin/${testName} )
endforeach(testSrc)
