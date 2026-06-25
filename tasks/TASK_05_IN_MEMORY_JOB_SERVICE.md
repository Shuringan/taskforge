# TASK_05_IN_MEMORY_JOB_SERVICE.md

# Task 05 — In-memory job service

## Goal

Create an in-memory application service that can manage jobs without HTTP and without SQLite.

The service should create jobs, store them in memory, submit them to `JobQueue`, and expose basic job state operations.

Do not implement worker pool in this task. Job execution is not required yet.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-04.

Implement only this task.

## Scope

Create an `app` layer for in-memory job management.

Recommended files:

```text
include/taskforge/app/CreateJobRequest.hpp
include/taskforge/app/InMemoryJobService.hpp
src/app/InMemoryJobService.cpp
tests/in_memory_job_service_tests.cpp
```

You may add a small ID generator helper if needed:

```text
include/taskforge/utils/IdGenerator.hpp
src/utils/IdGenerator.cpp
```

## Required behavior

`InMemoryJobService` should support:

1. creating a job;
2. storing job metadata in memory;
3. pushing the created job ID to `JobQueue`;
4. retrieving job by ID;
5. listing jobs;
6. updating job status;
7. storing job result;
8. storing job error.

Suggested request type:

```cpp
struct CreateJobRequest {
    std::string type;
    std::string file_id;
    int priority = 0;
    int max_attempts = 1;
};
```

At this stage, `file_id` can be stored as a string even if full file registry is not implemented yet.

## Suggested service API

```cpp
class InMemoryJobService {
public:
    explicit InMemoryJobService(JobQueue& queue);

    Job create_job(CreateJobRequest request);

    std::optional<Job> get_job(const JobId& id) const;
    std::vector<Job> list_jobs() const;

    bool update_status(const JobId& id, JobStatus status);
    bool store_result(const JobId& id, std::string result);
    bool store_error(const JobId& id, std::string error);
};
```

The exact API may differ, but tests must cover equivalent behavior.

## Thread safety

The service may be accessed by future worker threads.

Protect internal state with `std::mutex`.

Avoid returning references to internal mutable state.

## Job model extension

If the current `Job` model does not have `file_id`, `result`, or `error`, extend it carefully.

Keep the model simple.

## Test requirements

Add tests covering:

1. creating a job returns a job with status `Pending`;
2. creating a job pushes job ID into `JobQueue`;
3. retrieving created job by ID works;
4. retrieving missing job returns empty result;
5. listing jobs returns created jobs;
6. updating status works;
7. storing result works;
8. storing error works;
9. service rejects invalid create request if type is empty.

## CMake requirements

Update CMake so the new sources and tests are compiled.

Keep the project buildable.

## Hard constraints

Do not:

- implement worker pool;
- execute jobs;
- implement HTTP;
- implement SQLite;
- implement file registry;
- implement retry logic;
- implement cancellation logic.

This task is only about in-memory job state and job creation.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. jobs are stored in memory;
5. job creation pushes to queue;
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
