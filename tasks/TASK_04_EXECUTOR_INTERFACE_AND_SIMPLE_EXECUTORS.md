# TASK_04_EXECUTOR_INTERFACE_AND_SIMPLE_EXECUTORS.md

# Task 04 — Executor interface and simple executors

## Goal

Add the executor abstraction and two simple file-processing executors:

- `LineCountExecutor`;
- `WordCountExecutor`.

This task must not implement job service, worker pool, HTTP, SQLite, retry, or cancellation.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-03.

Implement only this task.

## Scope

Create an `executors` layer.

Recommended files:

```text
include/taskforge/executors/ExecutionResult.hpp
include/taskforge/executors/JobExecutor.hpp
include/taskforge/executors/LineCountExecutor.hpp
include/taskforge/executors/WordCountExecutor.hpp

src/executors/LineCountExecutor.cpp
src/executors/WordCountExecutor.cpp

tests/executor_tests.cpp
```

## Required behavior

### ExecutionResult

Create a small result type.

Suggested shape:

```cpp
struct ExecutionResult {
    bool success;
    std::string output;
    std::string error;

    static ExecutionResult ok(std::string output);
    static ExecutionResult fail(std::string error);
};
```

`output` may be a JSON-like string for now.

### JobExecutor

Create an abstract interface.

Suggested API:

```cpp
class JobExecutor {
public:
    virtual ~JobExecutor() = default;

    virtual std::string type() const = 0;
    virtual ExecutionResult execute(const std::filesystem::path& file_path) = 0;
};
```

### LineCountExecutor

Reads a file and returns number of lines.

Example output:

```json
{"lines":3}
```

### WordCountExecutor

Reads a file and returns number of words separated by whitespace.

Example output:

```json
{"words":12}
```

## Error handling

If the file cannot be opened, return failed `ExecutionResult`.

Do not throw from normal execution failures unless there is a strong reason. The executor should return a failure result.

## Test requirements

Add tests covering:

1. `LineCountExecutor` counts lines correctly.
2. `LineCountExecutor` fails for missing file.
3. `WordCountExecutor` counts words correctly.
4. `WordCountExecutor` fails for missing file.
5. Executor `type()` values are stable:
   - `line_count`;
   - `word_count`.

Use temporary files in tests.

## CMake requirements

Update CMake so the new sources and tests are compiled.

Keep the project buildable.

## Hard constraints

Do not:

- implement `WorkerPool`;
- implement `JobService`;
- implement HTTP;
- implement SQLite;
- implement CSV validation;
- implement retry;
- implement cancellation.

This task is only about executor abstractions and two simple executors.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. line count and word count executors work;
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
