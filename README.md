# C++ Periodic Task Scheduler

# Dependencies

1.  Boost (1.60.1)
2. sqlite3 (3.8.10.2) (NOTE: should run in either multithreaded or Serialized mode)
3. cmake (3.6.1)

#Tested on
    OSX El Capitan (10.11.16)

# Build Instructions
In the project folder execute the following commands

    mkdir build
    cd build
    cmake ../
    make

# RUN instructions

# Special instructions:
  1. To send icmp packets run with sudo
  2. Ignore database create table error when running the executable second time

# Execute to send tcp and icmp packets to google.com

    sudo ./Scheduler (Note: sudo required to send icmp packets)


To run unit tests that checks that addTask, modify task and cancelTask is working properly

    Note: test executables are in testBin folder

    sudo ../testBin/test_scheduler
