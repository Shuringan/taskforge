# TASK_10_RECOVERY_AFTER_RESTART.md

# Task 10 — Recovery after restart

## Goal

Add startup recovery logic for persisted jobs.

On service startup:

- `pending` jobs should be returned to the queue;
- interrupted `running` jobs should be marked as `failed`.

Do not implement HTTP, retry endpoint, cancellation endpoint, or Docker in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-09.

Implement only this task.

## Scope

Add recovery component.

Recommended files:

```text
include/taskforge/app/JobRecoveryService.hpp
src/app/JobRecoveryService.cpp
tests/job_recovery_tests.cpp
```

You may integrate it with existing application/service wiring only as much as needed.

## Required behavior

`JobRecoveryService` should:

1. query repository for jobs with status `pending`;
2. push each pending job ID to `JobQueue`;
3. query repository for jobs with status `running`;
4. mark each running job as `failed`;
5. store error message for interrupted running jobs.

Recommended error message:

```text
Service was restarted during job execution
```

## Suggested API

```cpp
struct RecoverySummary {
    std::size_t requeued_pending_jobs = 0;
    std::size_t failed_running_jobs = 0;
};

class JobRecoveryService {
public:
    JobRecoveryService(JobRepository& jobs, JobQueue& queue);

    RecoverySummary recover();
};
```

The exact API may differ, but tests must cover equivalent behavior.

## Idempotency

Running recovery more than once should not duplicate completed jobs or alter `done`, `failed`, or `cancelled` jobs.

For pending jobs, duplicate queueing after repeated recovery is acceptable only if the service is normally called once on startup. Prefer avoiding duplicates if this is easy, but do not over-engineer.

## Test requirements

Add tests covering:

1. pending jobs are pushed to queue;
2. running jobs are marked failed;
3. running jobs receive interruption error message;
4. done jobs are unchanged;
5. failed jobs are unchanged;
6. cancelled jobs are unchanged;
7. recovery summary counts are correct.

Tests may use SQLite repositories if stable. If storage tests are heavy, use a test fake repository.

## CMake requirements

Update CMake so new files and tests are compiled.

Keep the project buildable.

## Hard constraints

Do not:

- implement HTTP;
- implement retry endpoint;
- implement cancellation endpoint;
- implement Docker;
- change job lifecycle rules beyond recovery.

This task is only about startup recovery.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. pending jobs are requeued;
5. running jobs are failed with clear error;
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
