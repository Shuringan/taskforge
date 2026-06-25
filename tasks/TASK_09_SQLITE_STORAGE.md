# TASK_09_SQLITE_STORAGE.md

# Task 09 — SQLite storage

## Goal

Add SQLite persistence for files and jobs.

This task introduces repository interfaces and SQLite implementations.

Do not implement HTTP, retry, cancellation, or recovery in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-08.

Implement only this task.

## Scope

Add storage layer.

Recommended files:

```text
include/taskforge/storage/Database.hpp
src/storage/Database.cpp

include/taskforge/storage/JobRepository.hpp
include/taskforge/storage/SQLiteJobRepository.hpp
src/storage/SQLiteJobRepository.cpp

include/taskforge/storage/FileRepository.hpp
include/taskforge/storage/SQLiteFileRepository.hpp
src/storage/SQLiteFileRepository.cpp

tests/sqlite_storage_tests.cpp
```

You may adjust names if needed.

## Dependency

Use SQLite.

Acceptable approaches:

- link against system SQLite;
- use a small CMake-compatible SQLite dependency approach;
- document setup if system package is required.

Keep this task practical and buildable.

## Required tables

Create tables if they do not exist.

### files

```sql
CREATE TABLE IF NOT EXISTS files (
    id TEXT PRIMARY KEY,
    original_name TEXT NOT NULL,
    path TEXT NOT NULL,
    size INTEGER NOT NULL,
    uploaded_at TEXT NOT NULL
);
```

### jobs

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

If existing domain names use `registered_at` instead of `uploaded_at`, adapt consistently.

## Required repository behavior

### FileRepository

Support:

- insert file;
- find file by ID;
- list files.

### JobRepository

Support:

- insert job;
- find job by ID;
- list jobs;
- list jobs by status;
- update status;
- update result;
- update error;
- update attempts.

## Serialization

Store timestamps as ISO-like strings or Unix timestamps.

Pick one approach and keep it consistent.

## Error handling

SQLite errors must not be swallowed.

Acceptable approaches:

- throw `std::runtime_error`;
- return result types.

Use one consistent approach.

## Test requirements

Use temporary database file or in-memory SQLite if reliable.

Add tests covering:

1. database initializes schema;
2. insert/find file;
3. insert/find job;
4. update job status;
5. update result and error;
6. list jobs;
7. list jobs by status;
8. attempts persist correctly.

## CMake requirements

Update CMake for SQLite.

Keep the project buildable.

If SQLite linking requires platform-specific handling, document it in README.

## Hard constraints

Do not:

- implement HTTP;
- implement service recovery;
- implement retry logic;
- implement cancellation;
- rewrite worker pool unless required for compilation.

This task is only about storage.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. SQLite schema is created;
5. repositories can persist and load files/jobs;
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
