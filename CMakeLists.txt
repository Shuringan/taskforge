cmake_minimum_required(VERSION 3.20)

project(TaskForge
    VERSION 0.1.0
    DESCRIPTION "C++20 async file processing backend"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(taskforge_core
    src/core/Job.cpp
    src/core/JobQueue.cpp
    src/core/JobStatus.cpp
)

target_include_directories(taskforge_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(taskforge_app
    src/app/InMemoryJobService.cpp
)

target_include_directories(taskforge_app
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(taskforge_app
    PUBLIC
        taskforge_core
)

add_library(taskforge_executors
    src/executors/LineCountExecutor.cpp
    src/executors/WordCountExecutor.cpp
)

target_include_directories(taskforge_executors
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(taskforge
    src/main.cpp
)

enable_testing()

add_executable(taskforge_tests
    tests/basic_tests.cpp
    tests/executor_tests.cpp
    tests/in_memory_job_service_tests.cpp
    tests/job_model_tests.cpp
    tests/job_queue_tests.cpp
)

target_link_libraries(taskforge_tests
    PRIVATE
        taskforge_app
        taskforge_core
        taskforge_executors
)

add_test(NAME taskforge_tests COMMAND taskforge_tests)
