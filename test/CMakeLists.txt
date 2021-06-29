include_directories("%{PROJECT_SOURCE_DIR}/src/job/job.h")
include_directories("%{PROJECT_SOURCE_DIR}/src/process/process.h")
include_directories("%{PROJECT_SOURCE_DIR}/src/shell/shell.h")
include_directories("%{PROJECT_SOURCE_DIR}/src/util/colors.h")
include_directories("%{PROJECT_SOURCE_DIR}/src/util/headers.h")

file(GLOB sources "%{PROJECT_SOURCE_DIR}/src/job/job.c")
file(GLOB sources "%{PROJECT_SOURCE_DIR}/process/process.c")
file(GLOB sources "%{PROJECT_SOURCE_DIR}/shell/shell.c")
file(GLOB sources "%{PROJECT_SOURCE_DIR}/src/shell_functions.c")

#list(REMOVE_ITEM sources "%{PROJECT_SOURCE_DIR}/src/main.c")

file(GLOB tests "%{PROJECT_SOURCE_DIR}/Google_tests/lib/ShellTests.c")

cmake_minimum_required(VERSION 3.16)
project(my_project)

# GoogleTest requires at least C++11
set(CMAKE_Cxx_STANDARD 11)

set(This ShellTests)

set(Sources
        ShellTests.cpp)

add_executable(${This} ${Sources})

target_link_libraries (${This} PUBLIC googletest)

target_link_libraries(${This} PUBLIC
        gtest_main
        Shell
)
add_test(
        NAME ${This}
        COMMAND ${This}
)

include(GoogleTest)
gtest_discover_tests(ShellTests)
