cmake_minimum_required(VERSION 3.28.3)
project(lab2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O0 -g")

add_executable(lab2 cache/lab2.cpp cache/lab2.h Testi.cpp)
