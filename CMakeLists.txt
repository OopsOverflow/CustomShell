cmake_minimum_required(VERSION 3.16)
project(microshell C)

set(CMAKE_C_STANDARD 99)

add_executable(microshell src/shell/shell.c src/process/process.c src/job/job.c shell_exec.c src/shell/shell_functions.c)

add_subdirectory(Google_tests)