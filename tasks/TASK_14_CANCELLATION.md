# TASK_14_CANCELLATION.md

# Task 14 — Cancellation

## Goal

Add cancellation for pending and running jobs.

Cancellation should work at least for pending jobs. Running jobs should support cooperative cancellation where practical.

Do not implement new job types or new storage systems in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-13.

Implement only this task.

## Scope

Add cancellation support to core/app/API layers.

Recommended files to create or update:

```text
include/taskforge/core/CancellationToken.hpp
include/taskforge/app/CancelJobResult.hpp
src/app/InMemoryJobService.cpp
src/core/WorkerPool.cpp
src/api/JobController.cpp
tests/cancellation_tests.cpp
```

Exact files depend on current structure.

## Required behavior

### Pending job cancellation

If job status is `pending`:

- change status to `cancelled`;
- prevent execution if worker later pops this job from queue;
- return success.

It is acceptable not to physically remove the job ID from `JobQueue`. Worker should skip cancelled jobs.

### Running job cancellation

If job status is `running`:

- mark cancellation requested;
- executor/worker should check cancellation where practical;
- if cancellation is observed, set status to `cancelled`.

Minimum acceptable implementation:

- store cancellation request;
- worker checks it between major execution steps;
- simple executors may not interrupt instantly.

### Completed job cancellation

If job status is `done`, `failed`, or `cancelled`:

- return invalid state;
- do not change status.

## HTTP endpoint

Add endpoint:

```http
POST /jobs/{job_id}/cancel
```

Success response:

```json
{
  "job_id": "job_xxx",
  "status": "cancelled"
}
```

Invalid state response:

```json
{
  "error": "Cannot cancel completed job"
}
```

Use 409 for invalid state.

## Test requirements

Add tests covering:

1. pending job can be cancelled;
2. cancelled pending job is not executed by worker;
3. cancelling missing job fails;
4. cancelling done job fails;
5. cancelling failed job fails;
6. cancelling already cancelled job is handled consistently;
7. HTTP endpoint works if HTTP tests are available.

If running cancellation is implemented, test it with a controllable slow executor.

## Thread safety

Cancellation state may be accessed by API thread and worker thread.

Protect shared state with synchronization.

Avoid data races.

## Hard constraints

Do not:

- implement job history;
- implement distributed cancellation;
- implement signal-based thread killing;
- forcefully terminate worker threads;
- rewrite executors unnecessarily.

Cancellation must be cooperative.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. pending jobs can be cancelled;
5. workers skip cancelled jobs;
6. HTTP cancel endpoint works if HTTP layer is present;
7. no unsafe thread termination is used.

## Suggested verification commands

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual check example:

```bash
curl -X POST http://127.0.0.1:8080/jobs/job_xxx/cancel
```

## Completion response

After implementation, report:

1. summary of changes;
2. files changed;
3. commands run;
4. build/test result;
5. manual HTTP checks if run;
6. known limitations.
