cmake_minimum_required(VERSION 3.28.3)
project(lab2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O0 -g")

add_executable(lab2 lab2.cpp lab2.h test1.cpp)
