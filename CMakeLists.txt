cmake_minimum_required(VERSION 3.20)

project(TaskForge
    VERSION 0.1.0
    DESCRIPTION "C++20 async file processing backend"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(taskforge
    src/main.cpp
)

enable_testing()

add_executable(taskforge_tests
    tests/basic_tests.cpp
)

add_test(NAME taskforge_tests COMMAND taskforge_tests)
