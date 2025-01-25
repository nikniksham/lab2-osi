cmake_minimum_required(VERSION 3.28.3)
project(lab2)

set(CMAKE_CXX_STANDARD 17)

# Добавляем библиотеку и путь к исходникам
add_library(niki2 SHARED cache/lab2.cpp)

# Указываем путь к библиотекам
link_directories(${CMAKE_SOURCE_DIR}/cache)

# Выключаем оптимизацию
set(CMAKE_CXX_FLAGS "-O0 -g")

add_executable(lab2 Testi.cpp)

# Указываем, с какими библиотеками связываемся
target_link_libraries(lab2 niki2)
