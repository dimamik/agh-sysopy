cmake_minimum_required(VERSION 3.16)
project(CLion_Project C)

set(CMAKE_C_STANDARD 99)

add_executable(CLion_Project main.c)
target_link_libraries(CLion_Project m)