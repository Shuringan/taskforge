# Architecture

```text
Client
  |
  v
HTTP API
  |
  v
Application services
  |
  +-------> Repository / SQLite
  |
  v
JobQueue
  |
  v
WorkerPool
  |
  v
Executors
```

## Layers

- `api`: HTTP routing and controllers.
- `app`: in-memory services, retry, cancellation, and recovery orchestration.
- `core`: job model, file model, queue, and worker pool.
- `executors`: file-processing executors.
- `storage`: SQLite database wrapper and repositories.

## Main components

- `JobQueue` is a blocking FIFO queue for `JobId`.
- `InMemoryJobService` stores job state and enqueues new jobs.
- `InMemoryFileService` registers existing local files.
- `WorkerPool` consumes job IDs, finds executors, and updates job state.
- `ExecutorRegistry` maps job type strings to executors.
- SQLite repositories persist files and jobs behind repository interfaces.

## Job lifecycle

```text
pending -> running -> done
pending -> running -> failed
pending -> cancelled
running -> cancelled
failed -> pending
```

## Worker flow

1. Worker waits on `JobQueue`.
2. Worker fetches the job from the job service.
3. Cancelled jobs are skipped.
4. Executor is resolved by job type.
5. Job is marked `running`.
6. Executor runs against the registered file path.
7. Success stores result and marks `done`.
8. Failure increments attempts and either requeues or marks `failed`.

## Queue behavior

The queue uses `std::mutex` and `std::condition_variable`. Shutdown wakes all
waiting workers. Pushing after shutdown throws `std::runtime_error`.

## SQLite persistence

The storage layer creates `files` and `jobs` tables and supports insert, find,
list, status filtering, and job updates. Timestamps are stored as Unix
milliseconds encoded as text.

## Retry

Automatic retry happens in the worker when execution fails and attempts remain.
Manual retry is available through `POST /jobs/{job_id}/retry` for failed jobs.

## Cancellation

Pending jobs are marked `cancelled` and skipped by workers. Running cancellation
is cooperative: the service records the cancellation and the worker avoids
overwriting a cancelled job after executor completion.

## Recovery after restart

`JobRecoveryService` requeues persisted `pending` jobs and marks interrupted
`running` jobs as `failed` with a clear restart error.
