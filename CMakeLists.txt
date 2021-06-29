cmake_minimum_required(VERSION 3.19)
project(microshell C)

set(CMAKE_C_STANDARD 99)

add_executable(microshell src/shell.c src/process.c src/job.c)