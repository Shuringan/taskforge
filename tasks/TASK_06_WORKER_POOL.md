# TASK_06_WORKER_POOL.md

# Task 06 — Worker pool

## Goal

Implement a worker pool that consumes job IDs from `JobQueue`, executes them through registered executors, and updates in-memory job state.

This task should connect existing components:

- `JobQueue`;
- `InMemoryJobService`;
- `JobExecutor`;
- simple executors.

Do not implement HTTP, SQLite, retry, or cancellation in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-05.

Implement only this task.

## Scope

Add worker-related core/app files.

Recommended files:

```text
include/taskforge/core/WorkerPool.hpp
src/core/WorkerPool.cpp

include/taskforge/executors/ExecutorRegistry.hpp
src/executors/ExecutorRegistry.cpp

tests/worker_pool_tests.cpp
```

You may adjust file placement if it better matches existing code.

## Required behavior

### ExecutorRegistry

Create a registry that maps job type strings to executors.

Suggested API:

```cpp
class ExecutorRegistry {
public:
    void register_executor(std::shared_ptr<JobExecutor> executor);
    std::shared_ptr<JobExecutor> find(std::string_view type) const;
};
```

### WorkerPool

Worker pool should:

1. start N worker threads;
2. each worker waits on `JobQueue`;
3. when a job ID is received:
   - fetch job from service;
   - find executor by job type;
   - set job status to `Running`;
   - execute job;
   - set status to `Done` and store result on success;
   - set status to `Failed` and store error on failure;
4. stop cleanly.

Suggested API:

```cpp
class WorkerPool {
public:
    WorkerPool(
        std::size_t worker_count,
        JobQueue& queue,
        InMemoryJobService& job_service,
        ExecutorRegistry& executors
    );

    void start();
    void stop();

    ~WorkerPool();
};
```

## Missing executor behavior

If no executor exists for a job type:

- mark job as `Failed`;
- store a clear error message.

## Thread safety

Worker pool must:

- avoid busy waiting;
- join all threads on stop/destruction;
- be safe to stop more than once if practical;
- not leak threads.

## Test requirements

Add tests covering:

1. worker executes a `line_count` job and sets status `Done`;
2. worker stores result;
3. missing executor sets status `Failed`;
4. executor failure sets status `Failed`;
5. worker pool stops cleanly;
6. multiple jobs can be processed.

Tests should avoid flaky sleeps. If waiting is needed, use a bounded polling helper with a timeout.

## CMake requirements

Update CMake so the new sources and tests are compiled.

Keep the project buildable.

## Hard constraints

Do not:

- implement HTTP;
- implement SQLite;
- implement retry attempts;
- implement cancellation token;
- implement file registry beyond what is necessary for tests.

This task is only about background execution with the in-memory service.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. worker pool executes jobs in background;
5. worker pool stops cleanly;
6. no future-stage functionality is implemented.

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
