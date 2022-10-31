cmake_minimum_required(VERSION 3.16)
project(3360isaac)

set(CMAKE_CXX_STANDARD 20)

add_executable(1
        PA/1/1.cpp)

add_executable(2Server
        PA/2/server.cpp)

add_executable(2Client
        PA/2/client.cpp)

add_executable(HW3
        PA/3/main.cpp)

target_link_libraries(HW3 -lpthread)
