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

foreach(file ${tests})
    set(name)
    get_filename_component(name ${file} NAME_WE)
    add_executable("${name}_tests"
            ${sources}
            ${file}
            "${PROJECT_SOURCE_DIR}/test/ShellTests.c")
    target_link_libraries("${name}_tests" gtest_main)
    add_test(NAME ${name} COMMAND "${name}_tests")
endforeach()