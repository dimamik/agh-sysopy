cmake_minimum_required(VERSION 3.19)
project(Lab_8 C)

set(CMAKE_C_STANDARD 99)

add_executable(Lab_8 main.c pgma.h)
target_link_libraries(Lab_8 m)
target_link_libraries(Lab_8 pthread)
