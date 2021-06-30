cmake_minimum_required(VERSION 3.18)
project(microshell)

set(CMAKE_C_STANDARD 99)
include_directories(src)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/googletest)