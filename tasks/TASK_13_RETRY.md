# TASK_13_RETRY.md

# Task 13 — Retry

## Goal

Add retry logic for failed jobs.

This task should allow failed jobs to be resubmitted for execution while tracking attempts and respecting `max_attempts`.

Do not implement cancellation in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-12.

Implement only this task.

## Scope

Add retry behavior to the application service and expose it through HTTP.

Recommended files to update or create:

```text
include/taskforge/app/RetryJobResult.hpp
src/app/InMemoryJobService.cpp
src/storage/SQLiteJobRepository.cpp
src/api/JobController.cpp
tests/retry_tests.cpp
```

Exact files depend on existing implementation.

## Required behavior

### Automatic retry during worker execution

When executor fails:

1. increment attempts;
2. if `attempts < max_attempts`, set status to `pending`;
3. clear or update error as appropriate;
4. push job ID back to queue;
5. if `attempts >= max_attempts`, set status to `failed` and store error.

This automatic retry can be implemented in worker pool if practical.

### Manual retry endpoint

Add endpoint:

```http
POST /jobs/{job_id}/retry
```

Behavior:

- only failed jobs can be manually retried;
- set status to `pending`;
- clear previous error or keep it as historical if current model supports history;
- push job ID back to queue;
- return updated job state.

Recommended behavior for attempts on manual retry:

- do not reset attempts;
- reject retry if `attempts >= max_attempts`.

If you choose to reset attempts, document and test it. Preferred: do not reset.

## Response examples

Success:

```json
{
  "job_id": "job_xxx",
  "status": "pending",
  "attempts": 1,
  "max_attempts": 3
}
```

Invalid state:

```json
{
  "error": "Only failed jobs can be retried"
}
```

Attempts exhausted:

```json
{
  "error": "Maximum retry attempts reached"
}
```

## Test requirements

Add tests covering:

1. failed job with attempts below max can be retried;
2. retry pushes job ID to queue;
3. retry changes status to `pending`;
4. retry rejects non-failed job;
5. retry rejects missing job;
6. retry rejects job with attempts >= max_attempts;
7. worker automatic retry if implemented.

If HTTP tests exist, add endpoint tests for success and invalid state.

## Persistence

If SQLite is integrated into the active service, attempts/status updates must persist.

If the app still uses in-memory storage for tests, ensure service-level behavior is correct.

## Hard constraints

Do not:

- implement cancellation endpoint;
- implement job history table;
- implement exponential backoff unless trivial;
- implement delayed retry scheduling.

This task is only about immediate retry.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. failed jobs can be retried;
5. attempts are tracked;
6. retry endpoint works if HTTP layer is present;
7. cancellation is not implemented in this task.

## Suggested verification commands

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual check example:

```bash
curl -X POST http://127.0.0.1:8080/jobs/job_xxx/retry
```

## Completion response

After implementation, report:

1. summary of changes;
2. files changed;
3. commands run;
4. build/test result;
5. manual HTTP checks if run;
6. known limitations.
