# TASKS.md

# TaskForge implementation tasks

This file contains the project roadmap. Codex should not implement this whole file at once.

Each implementation run should use exactly one task file from `tasks/`.

---

## TASK 01 — Bootstrap project

File: `tasks/TASK_01_BOOTSTRAP.md`

Create a minimal buildable C++20 project with CMake, a simple executable, a minimal test target, and a README.

---

## TASK 02 — Job model

File: `tasks/TASK_02_JOB_MODEL.md`

Add the basic job domain model: `JobId`, `JobStatus`, status string conversion, and `Job`.

---

## TASK 03 — Thread-safe job queue

File: `tasks/TASK_03_JOB_QUEUE.md`

Implement a thread-safe blocking queue for `JobId`.

---

## TASK 04 — Executor interface and simple executors

File: `tasks/TASK_04_EXECUTOR_INTERFACE_AND_SIMPLE_EXECUTORS.md`

Add the executor abstraction and two simple executors: `LineCountExecutor` and `WordCountExecutor`.

---

## TASK 05 — In-memory job service

File: `tasks/TASK_05_IN_MEMORY_JOB_SERVICE.md`

Create an in-memory application service that stores jobs and files, creates jobs, and exposes core operations without HTTP.

---

## TASK 06 — Worker pool

File: `tasks/TASK_06_WORKER_POOL.md`

Implement a worker pool that consumes jobs from `JobQueue`, executes them, and updates job state.

---

## TASK 07 — File registry

File: `tasks/TASK_07_FILE_REGISTRY.md`

Add file registration by local path and file metadata handling.

---

## TASK 08 — CSV validation executor

File: `tasks/TASK_08_CSV_VALIDATE_EXECUTOR.md`

Add `CsvValidateExecutor` with CSV statistics and validation.

---

## TASK 09 — SQLite storage

File: `tasks/TASK_09_SQLITE_STORAGE.md`

Add SQLite repositories for files and jobs.

---

## TASK 10 — Recovery after restart

File: `tasks/TASK_10_RECOVERY_AFTER_RESTART.md`

Add startup recovery for pending/running jobs.

---

## TASK 11 — HTTP health and metrics

File: `tasks/TASK_11_HTTP_HEALTH_AND_METRICS.md`

Add an HTTP server with `/health` and `/metrics`.

---

## TASK 12 — HTTP file and job API

File: `tasks/TASK_12_HTTP_FILE_AND_JOB_API.md`

Add REST endpoints for file registration and job operations.

---

## TASK 13 — Retry

File: `tasks/TASK_13_RETRY.md`

Add retry for failed jobs with attempts tracking and max attempts logic.

---

## TASK 14 — Cancellation

File: `tasks/TASK_14_CANCELLATION.md`

Add cancellation for pending and running jobs.

---

## TASK 15 — Documentation and examples

File: `tasks/TASK_15_DOCUMENTATION_AND_EXAMPLES.md`

Finalize README, API docs, architecture docs, and usage examples.

---

## TASK 16 — Docker and CI

File: `tasks/TASK_16_DOCKER_AND_CI.md`

Add Dockerfile and GitHub Actions workflow for build and tests.

---

## General rule

Do not ask Codex to execute multiple tasks from this list in one run unless the previous task is already complete, committed, and verified.
