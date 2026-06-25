# Resume Notes

## Summary

TaskForge is a C++20 backend service for asynchronous local file processing. It
demonstrates job lifecycle management, background execution, persistence,
HTTP routing, retry, cancellation, and testable backend layering.

## Technologies

- C++20
- CMake
- SQLite
- cpp-httplib
- nlohmann/json
- Standard-library concurrency

## Implemented Features

- REST endpoints for files, jobs, health, and metrics.
- Thread-safe job queue and worker pool.
- File executors for line count, word count, and CSV validation.
- In-memory application services.
- SQLite repositories for files and jobs.
- Startup recovery for persisted pending/running jobs.
- Retry for failed jobs.
- Cooperative cancellation for pending/running jobs.
- Unit and controller tests.

## Suggested Resume Entry

TaskForge - C++20 Async File Processing Backend

- Built a modular C++20 backend for asynchronous file processing with REST
  endpoints, background workers, and a thread-safe job queue.
- Implemented job lifecycle management, retry, cooperative cancellation, and
  startup recovery for interrupted jobs.
- Added SQLite repositories, file-processing executors, and tests covering core,
  application, storage, worker, and API behavior.
