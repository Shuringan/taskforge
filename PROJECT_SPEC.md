# PROJECT_SPEC.md

# TaskForge — C++ Async File Processing Backend

## 1. Project overview

TaskForge is a production-like C++20 backend service for asynchronous processing of CSV and text files.

The project is intended as a portfolio-grade C++ backend project. It should demonstrate practical backend engineering skills:

- REST API design;
- background job processing;
- worker pool;
- thread-safe queue;
- persistent job state;
- file processing;
- retry and cancellation logic;
- structured error handling;
- logging;
- configuration;
- tests;
- CMake-based build;
- reproducible local launch.

The project must be implemented incrementally through small, verifiable development tasks. Do not ask Codex to implement the full project in one run.

---

## 2. Product idea

TaskForge solves the problem of long-running file operations that should not block an HTTP request.

Instead of this flow:

```text
Client sends request -> server processes file synchronously -> client waits
```

TaskForge uses this flow:

```text
Client registers file -> client creates job -> API returns job_id immediately
-> worker processes job in background -> client checks status/result later
```

This approach is useful for:

- CSV validation;
- data import preprocessing;
- text statistics;
- report generation;
- batch file processing;
- any operation where processing may take longer than a normal HTTP request should take.

---

## 3. Target user scenario

Example scenario:

1. A user has a CSV file with product data.
2. The user registers the file in TaskForge.
3. The user creates a `csv_validate` job for that file.
4. The API immediately returns a `job_id`.
5. A worker thread picks up the job.
6. The service validates the CSV file:
   - counts rows;
   - detects empty fields;
   - detects duplicate rows;
   - detects inconsistent column counts.
7. The user checks job status via API.
8. When the job is done, the user retrieves the result as JSON.

---

## 4. Final target functionality

The final service should support:

### File operations

- register local file path;
- store file metadata;
- validate that a file exists;
- use registered files as input for jobs.

Multipart upload is intentionally not required in the first complete implementation. It may be added later as an improvement.

### Job operations

- create job;
- get job by ID;
- list jobs;
- get job result;
- cancel pending/running job;
- retry failed job.

### Service operations

- health check;
- basic metrics;
- graceful shutdown;
- queue recovery after restart.

---

## 5. Target HTTP API

```http
POST /files
GET /files/{file_id}

POST /jobs
GET /jobs
GET /jobs/{job_id}
GET /jobs/{job_id}/result
POST /jobs/{job_id}/cancel
POST /jobs/{job_id}/retry

GET /health
GET /metrics
```

The exact API can be refined during implementation, but the product direction should remain the same.

---

## 6. Job lifecycle

```text
pending -> running -> done
pending -> running -> failed
pending -> cancelled
running -> cancelled
failed -> pending    # after retry
```

Required statuses:

```cpp
enum class JobStatus {
    Pending,
    Running,
    Done,
    Failed,
    Cancelled
};
```

---

## 7. Initial job types

The final project should support at least these job types.

### line_count

Counts lines in a text file.

Example result:

```json
{
  "lines": 1000
}
```

### word_count

Counts words in a text file.

Example result:

```json
{
  "words": 5820
}
```

### csv_validate

Validates a CSV file.

Minimum checks:

- total rows;
- empty fields;
- duplicate rows;
- inconsistent column count.

Example result:

```json
{
  "total_rows": 12000,
  "valid_rows": 11840,
  "invalid_rows": 160,
  "empty_fields": 51,
  "duplicate_rows": 34,
  "inconsistent_rows": 75
}
```

---

## 8. Target architecture

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
Thread-safe queue
  |
  v
Worker pool
  |
  v
Executors
  |
  v
Result storage
```

Recommended layers:

```text
api/        HTTP server and controllers
app/        application wiring and service orchestration
core/       job model, queue, worker pool, lifecycle logic
storage/    repositories and SQLite implementation
executors/  actual job execution logic
config/     configuration loading
utils/      logging, time, id generation, common helpers
```

---

## 9. Target technology stack

Preferred stack:

- C++20;
- CMake;
- GoogleTest;
- SQLite;
- nlohmann/json;
- Boost.Asio / Boost.Beast for HTTP;
- spdlog or a simple custom logger.

Dependency choices can be adjusted if build complexity becomes excessive. Any major substitution must be documented in README.

---

## 10. Persistence

The final project should use SQLite.

Target tables:

```sql
CREATE TABLE IF NOT EXISTS files (
    id TEXT PRIMARY KEY,
    original_name TEXT NOT NULL,
    path TEXT NOT NULL,
    size INTEGER NOT NULL,
    uploaded_at TEXT NOT NULL
);
```

```sql
CREATE TABLE IF NOT EXISTS jobs (
    id TEXT PRIMARY KEY,
    type TEXT NOT NULL,
    file_id TEXT NOT NULL,
    status TEXT NOT NULL,
    priority INTEGER NOT NULL DEFAULT 0,
    attempts INTEGER NOT NULL DEFAULT 0,
    max_attempts INTEGER NOT NULL DEFAULT 1,
    result TEXT,
    error TEXT,
    created_at TEXT NOT NULL,
    started_at TEXT,
    finished_at TEXT,
    FOREIGN KEY(file_id) REFERENCES files(id)
);
```

---

## 11. Non-goals for the first complete implementation

Do not implement:

- PostgreSQL;
- authentication;
- web UI;
- Kubernetes;
- distributed queue;
- multipart upload;
- WebSocket status updates;
- Prometheus exporter;
- complex OpenAPI generator.

These can be listed as future improvements.

---

## 12. Quality expectations

The final project should be:

- buildable from a clean checkout;
- reasonably cross-platform: Linux and Windows when practical;
- modular;
- testable;
- documented;
- free of obvious data races;
- free of busy waiting;
- explicit about errors;
- suitable for review in a C++ backend interview.

---

## 13. Portfolio positioning

Suggested resume description after completion:

```text
TaskForge — C++20 Async File Processing Backend

Developed a production-like backend service for asynchronous file processing.
Implemented REST API, worker pool, thread-safe job queue, SQLite persistence,
retry mechanism, job cancellation, structured error handling, metrics,
configuration, unit tests, and queue recovery after service restart.
```
