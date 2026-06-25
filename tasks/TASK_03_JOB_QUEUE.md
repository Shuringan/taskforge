# TASK_03_JOB_QUEUE.md

# Task 03 — Thread-safe job queue

## Goal

Implement a thread-safe blocking queue for `JobId`.

This queue will later be used by worker threads to wait for jobs. In this task, only the queue itself and its tests are required.

Do not implement worker pool, executors, HTTP, SQLite, retry, or cancellation in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01 and 02.

Implement only this task.

## Scope

Add a queue under the core layer.

Recommended files:

```text
include/taskforge/core/JobQueue.hpp
src/core/JobQueue.cpp
tests/job_queue_tests.cpp
```

## Required behavior

`JobQueue` should store `JobId` values.

Minimum API:

```cpp
class JobQueue {
public:
    JobQueue() = default;

    void push(JobId id);

    // Blocks until a job is available or the queue is stopped.
    // Returns std::nullopt if the queue is stopped and no job is available.
    std::optional<JobId> wait_and_pop();

    // Stops the queue and wakes all waiting threads.
    void shutdown();

    bool empty() const;
    std::size_t size() const;
    bool is_shutdown() const;
};
```

The exact API may differ slightly, but it must provide equivalent behavior.

## Concurrency requirements

The implementation must:

- use `std::mutex`;
- use `std::condition_variable`;
- avoid busy waiting;
- wake waiting threads when a job is pushed;
- wake waiting threads when shutdown is requested;
- be safe for multiple producer threads;
- be safe for multiple consumer threads.

## Shutdown semantics

Expected behavior:

1. If the queue has jobs, `wait_and_pop()` may still return existing jobs.
2. If the queue is empty and shutdown was called, `wait_and_pop()` returns `std::nullopt`.
3. If a thread is blocked in `wait_and_pop()` and another thread calls `shutdown()`, the blocked thread wakes up and returns `std::nullopt` if no job is available.
4. After shutdown, `push()` should not accept new jobs.

For `push()` after shutdown, use this behavior:

- throw `std::runtime_error`.

Document this behavior in a test.

## Test requirements

Add tests covering:

1. New queue is empty.
2. `push()` increases size.
3. `wait_and_pop()` returns pushed job.
4. FIFO behavior for at least two jobs.
5. `shutdown()` marks queue as shut down.
6. `wait_and_pop()` returns `std::nullopt` when queue is shut down and empty.
7. A waiting consumer wakes up after `shutdown()`.
8. `push()` after shutdown throws `std::runtime_error`.
9. Basic multi-producer/multi-consumer behavior if practical.

The tests should not be flaky. Use timeouts conservatively if needed.

## CMake requirements

Update CMake so the new source and test files are compiled.

Keep the project buildable.

## Hard constraints

Do not:

- implement worker pool;
- implement job execution;
- implement executor interface;
- implement HTTP;
- implement SQLite;
- implement file registry;
- implement retry/cancel service logic.

This task is only about the queue.

## Acceptance criteria

The task is complete when:

1. project configures with CMake;
2. project builds;
3. all tests pass;
4. queue tests are added;
5. queue has no busy waiting;
6. queue supports clean shutdown;
7. no future-stage functionality is implemented.

## Suggested verification commands

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Completion response

After implementation, report:

1. summary of changes;
2. files changed;
3. commands run;
4. build/test result;
5. known limitations.
