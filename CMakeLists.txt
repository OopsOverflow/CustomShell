cmake_minimum_required(VERSION 3.18)
project(microshell)

set(CMAKE_CXX_STANDARD 11)
include_directories(src)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/googletest)