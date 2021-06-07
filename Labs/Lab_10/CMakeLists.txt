cmake_minimum_required(VERSION 3.19)
project(Lab_10 C)

set(CMAKE_C_STANDARD 99)

add_executable(Lab_10 common.h server.c common.h client)
target_link_libraries(Lab_10 pthread)
