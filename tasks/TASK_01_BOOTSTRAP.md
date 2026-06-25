# TASK_01_BOOTSTRAP.md

# Task 01 — Bootstrap C++20 project

## Goal

Create only the initial buildable C++20 project skeleton for TaskForge.

This task must not implement any business logic.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`

Use those files for context, but implement only this task.

## Scope

Create a minimal project that:

1. configures with CMake;
2. builds a simple executable;
3. builds and runs a minimal test target;
4. contains a minimal README with build and test instructions.

## Required files

Create or update:

```text
CMakeLists.txt
README.md
src/main.cpp
tests/basic_tests.cpp
```

You may also create:

```text
cmake/
```

only if needed.

## CMake requirements

Use C++20.

The project should expose:

- executable target: `taskforge`;
- test target: `taskforge_tests`.

Use GoogleTest if practical. If adding GoogleTest through `FetchContent`, keep the setup simple and standard.

Suggested commands:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Executable behavior

`src/main.cpp` should compile and run.

For now it may only print a short message, for example:

```text
TaskForge bootstrap
```

Do not start an HTTP server.

Do not create queue, worker, storage, or executor classes.

## Test behavior

Create one minimal test proving that the test infrastructure works.

Example:

```cpp
TEST(BasicTests, SanityCheck) {
    EXPECT_EQ(2 + 2, 4);
}
```

## README requirements

README must be in English.

Include:

- project name;
- short description;
- current status: bootstrap only;
- build commands;
- test commands;
- note that full functionality is planned and implemented task by task.

Do not claim that HTTP, queue, SQLite, workers, or file processing are implemented.

## Hard constraints

Do not:

- implement `Job`;
- implement `JobQueue`;
- implement worker pool;
- implement HTTP;
- implement SQLite;
- implement file processing;
- add placeholder TODO-only classes for future stages.

## Acceptance criteria

The task is complete when:

1. `cmake -S . -B build` succeeds.
2. `cmake --build build` succeeds.
3. `ctest --test-dir build --output-on-failure` succeeds.
4. `taskforge` executable builds.
5. `taskforge_tests` target builds.
6. README contains correct build/test instructions.
7. No future-stage functionality is implemented.

## Completion response

After implementation, report:

1. summary of changes;
2. files changed;
3. commands run;
4. build/test result;
5. known limitations.
