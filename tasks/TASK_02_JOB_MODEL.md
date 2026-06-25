# TASK_02_JOB_MODEL.md

# Task 02 — Job model

## Goal

Add the basic job domain model for TaskForge.

This task introduces `JobId`, `JobStatus`, status conversion helpers, and `Job`.

Do not implement queues, workers, HTTP, SQLite, or executors in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Task 01.

Implement only this task.

## Scope

Add core model files under the public include directory and implementation directory.

Recommended files:

```text
include/taskforge/core/JobId.hpp
include/taskforge/core/JobStatus.hpp
include/taskforge/core/Job.hpp

src/core/JobStatus.cpp
src/core/Job.cpp

tests/job_model_tests.cpp
```

You may adjust file names if there is a strong reason, but keep the model under `taskforge/core`.

## Required behavior

### JobId

Create a simple strong type or lightweight wrapper around `std::string`.

Minimum requirements:

- constructible from `std::string`;
- rejects empty value, preferably by throwing `std::invalid_argument`;
- exposes value as `const std::string&`;
- supports equality comparison;
- usable in tests.

Example API:

```cpp
class JobId {
public:
    explicit JobId(std::string value);
    const std::string& value() const noexcept;

    friend bool operator==(const JobId&, const JobId&) = default;
};
```

### JobStatus

Define:

```cpp
enum class JobStatus {
    Pending,
    Running,
    Done,
    Failed,
    Cancelled
};
```

Add conversion helpers:

```cpp
std::string to_string(JobStatus status);
std::optional<JobStatus> job_status_from_string(std::string_view value);
```

Required string values:

```text
pending
running
done
failed
cancelled
```

### Job

Create a basic `Job` model.

Minimum fields:

- `JobId id`;
- `std::string type`;
- `JobStatus status`;
- `int priority`;
- `int attempts`;
- `int max_attempts`;
- `std::chrono::system_clock::time_point created_at`.

Suggested API:

```cpp
class Job {
public:
    Job(JobId id, std::string type, int priority = 0, int max_attempts = 1);

    const JobId& id() const noexcept;
    const std::string& type() const noexcept;
    JobStatus status() const noexcept;
    int priority() const noexcept;
    int attempts() const noexcept;
    int max_attempts() const noexcept;
    std::chrono::system_clock::time_point created_at() const noexcept;

    void set_status(JobStatus status) noexcept;
    void increment_attempts() noexcept;
};
```

The exact API may differ, but tests must cover the behavior.

## CMake requirements

Update CMake so model sources are compiled and tests are included.

Keep the project buildable.

## Test requirements

Add tests covering:

1. `JobId` stores value correctly.
2. Empty `JobId` is rejected.
3. `JobStatus` converts to string correctly.
4. Valid strings convert to `JobStatus`.
5. Invalid strings return empty `std::optional`.
6. New `Job` has status `Pending`.
7. New `Job` has attempts `0`.
8. `increment_attempts()` increments attempts.
9. `set_status()` changes status.

## Hard constraints

Do not:

- implement `JobQueue`;
- implement worker pool;
- implement executor interface;
- implement HTTP server;
- implement SQLite;
- implement file registry;
- implement retry service logic;
- implement cancellation logic.

This task is only about the domain model.

## Acceptance criteria

The task is complete when:

1. project configures with CMake;
2. project builds;
3. all tests pass;
4. model tests are added;
5. no future-stage functionality is implemented.

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
