cmake_minimum_required(VERSION 3.20)

project(TaskForge
    VERSION 0.1.0
    DESCRIPTION "C++20 async file processing backend"
    LANGUAGES C CXX
)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
    sqlite_amalgamation
    URL https://www.sqlite.org/2026/sqlite-amalgamation-3530200.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(sqlite_amalgamation)

set(CPP_HTTPLIB_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/cpp_httplib)
file(MAKE_DIRECTORY ${CPP_HTTPLIB_INCLUDE_DIR})
file(DOWNLOAD
    https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
    ${CPP_HTTPLIB_INCLUDE_DIR}/httplib.h
    STATUS CPP_HTTPLIB_DOWNLOAD_STATUS
)
list(GET CPP_HTTPLIB_DOWNLOAD_STATUS 0 CPP_HTTPLIB_DOWNLOAD_CODE)
if (NOT CPP_HTTPLIB_DOWNLOAD_CODE EQUAL 0)
    message(FATAL_ERROR "Failed to download cpp-httplib")
endif()

add_library(cpp_httplib INTERFACE)
target_include_directories(cpp_httplib
    INTERFACE
        ${CPP_HTTPLIB_INCLUDE_DIR}
)

add_library(sqlite3 STATIC
    ${sqlite_amalgamation_SOURCE_DIR}/sqlite3.c
)

target_include_directories(sqlite3
    PUBLIC
        ${sqlite_amalgamation_SOURCE_DIR}
)

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
    src/app/InMemoryFileService.cpp
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

add_library(taskforge_api
    src/api/HealthController.cpp
    src/api/HttpServer.cpp
    src/api/MetricsController.cpp
)

target_include_directories(taskforge_api
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(taskforge_api
    PUBLIC
        taskforge_app
        taskforge_core
    PRIVATE
        cpp_httplib
)

add_library(taskforge_storage
    src/storage/Database.cpp
    src/storage/SQLiteFileRepository.cpp
    src/storage/SQLiteJobRepository.cpp
)

target_include_directories(taskforge_storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(taskforge_storage
    PUBLIC
        taskforge_core
    PRIVATE
        sqlite3
)

add_library(taskforge_recovery
    src/app/JobRecoveryService.cpp
)

target_include_directories(taskforge_recovery
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(taskforge_recovery
    PUBLIC
        taskforge_core
        taskforge_storage
)

add_library(taskforge_executors
    src/executors/CsvValidateExecutor.cpp
    src/executors/LineCountExecutor.cpp
    src/executors/ExecutorRegistry.cpp
    src/executors/WordCountExecutor.cpp
)

target_include_directories(taskforge_executors
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(taskforge_worker
    src/core/WorkerPool.cpp
)

target_include_directories(taskforge_worker
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(taskforge_worker
    PUBLIC
        taskforge_app
        taskforge_core
        taskforge_executors
)

add_executable(taskforge
    src/main.cpp
)

target_link_libraries(taskforge
    PRIVATE
        taskforge_api
        taskforge_app
        taskforge_core
)

enable_testing()

add_executable(taskforge_tests
    tests/basic_tests.cpp
    tests/csv_validate_executor_tests.cpp
    tests/executor_tests.cpp
    tests/file_registry_tests.cpp
    tests/http_health_metrics_tests.cpp
    tests/in_memory_job_service_tests.cpp
    tests/job_model_tests.cpp
    tests/job_queue_tests.cpp
    tests/job_recovery_tests.cpp
    tests/sqlite_storage_tests.cpp
    tests/worker_pool_tests.cpp
)

target_link_libraries(taskforge_tests
    PRIVATE
        taskforge_app
        taskforge_api
        taskforge_core
        taskforge_executors
        taskforge_recovery
        taskforge_storage
        taskforge_worker
)

add_test(NAME taskforge_tests COMMAND taskforge_tests)
