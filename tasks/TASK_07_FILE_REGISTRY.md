# TASK_07_FILE_REGISTRY.md

# Task 07 — File registry

## Goal

Add a simple file registry for local files.

The first implementation should register existing local file paths. Do not implement multipart file upload.

Do not implement HTTP or SQLite in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-06.

Implement only this task.

## Scope

Add file metadata model and in-memory file service.

Recommended files:

```text
include/taskforge/core/FileId.hpp
include/taskforge/core/FileRecord.hpp

include/taskforge/app/RegisterFileRequest.hpp
include/taskforge/app/InMemoryFileService.hpp
src/app/InMemoryFileService.cpp

tests/file_registry_tests.cpp
```

You may reuse existing ID generator utilities if available.

## Required behavior

### FileId

Create a strong type similar to `JobId`.

Minimum requirements:

- constructible from non-empty `std::string`;
- rejects empty value;
- exposes value;
- supports equality.

### FileRecord

Minimum fields:

- `FileId id`;
- `std::string original_name`;
- `std::filesystem::path path`;
- `std::uintmax_t size`;
- `std::chrono::system_clock::time_point registered_at`.

### InMemoryFileService

The service should support:

1. register local file path;
2. validate that path exists and is a regular file;
3. store metadata;
4. retrieve file record by ID;
5. list registered files.

Suggested request:

```cpp
struct RegisterFileRequest {
    std::filesystem::path path;
    std::string original_name;
};
```

If `original_name` is empty, use the filename from the path.

## Integration with job service

If `InMemoryJobService` currently accepts a raw `file_id` string, do not over-integrate yet.

At most, update types to use `FileId` if this is simple and does not cause broad rewrites.

Full validation that a job references an existing file may be added later.

## Error handling

Registering a missing file should fail clearly.

Acceptable approaches:

- throw `std::invalid_argument`;
- return `std::optional` or a small result type.

Choose one and test it.

## Test requirements

Add tests covering:

1. registering an existing file works;
2. metadata includes correct size;
3. original name defaults to filename when not provided;
4. retrieving registered file by ID works;
5. retrieving missing file returns empty result;
6. registering missing path fails;
7. registering directory path fails.

Use temporary files/directories in tests.

## CMake requirements

Update CMake so the new sources and tests are compiled.

Keep the project buildable.

## Hard constraints

Do not:

- implement multipart upload;
- implement HTTP endpoints;
- implement SQLite repositories;
- implement CSV validation;
- implement retry;
- implement cancellation.

This task is only about local file registration.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. local files can be registered and retrieved;
5. missing files are rejected;
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
