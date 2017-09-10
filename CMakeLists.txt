cmake_minimum_required (VERSION 2.6)
project(Scheduler)

set (CMAKE_CXX_STANDARD 11)
#Bring the headers, such as Student.h into the project
include_directories(include)

#set the cmake_module_path to find sqlite3 library

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#Can manually add the sources using the set command as follows:
#set(SOURCES src/main.cpp)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

add_executable(Scheduler ${SOURCES})

#find and link sqlite3
find_package (sqlite3)
if (SQLITE3_FOUND)
  include_directories(${SQLITE3_INCLUDE_DIRS})
  target_link_libraries (Scheduler ${SQLITE3_LIBRARIES})
endif (SQLITE3_FOUND)

#find and link boost
find_package(Boost 1.60 COMPONENTS filesystem unit_test_framework system REQUIRED system)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
target_link_libraries(Scheduler ${Boost_SYSTEM_LIBRARY})
