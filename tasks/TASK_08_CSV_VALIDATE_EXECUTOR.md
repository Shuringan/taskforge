# TASK_08_CSV_VALIDATE_EXECUTOR.md

# Task 08 — CSV validation executor

## Goal

Add `CsvValidateExecutor`.

This executor should analyze a CSV file and return validation/statistics result.

Do not implement HTTP, SQLite, retry, or cancellation in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-07.

Implement only this task.

## Scope

Add CSV executor files.

Recommended files:

```text
include/taskforge/executors/CsvValidateExecutor.hpp
src/executors/CsvValidateExecutor.cpp
tests/csv_validate_executor_tests.cpp
```

If useful, you may add a small CSV parsing helper:

```text
include/taskforge/utils/Csv.hpp
src/utils/Csv.cpp
```

Keep it simple.

## Required behavior

`CsvValidateExecutor` should implement `JobExecutor`.

Executor type:

```text
csv_validate
```

It should return a JSON-like string in `ExecutionResult::output`.

Minimum computed fields:

```json
{
  "total_rows": 0,
  "valid_rows": 0,
  "invalid_rows": 0,
  "empty_fields": 0,
  "duplicate_rows": 0,
  "inconsistent_rows": 0
}
```

## CSV parsing rules

Keep parser intentionally simple:

- delimiter: comma `,`;
- no need to support quoted fields in this task;
- trim whitespace around fields if simple to implement;
- empty line may be treated as invalid or skipped, but behavior must be documented in tests;
- first non-empty row defines expected column count.

Do not over-engineer RFC 4180 support.

## Validation rules

For each row:

- if column count differs from expected, count as inconsistent and invalid;
- if any field is empty after trimming, count empty fields and treat the row as invalid;
- duplicate row detection should compare the whole raw normalized row;
- otherwise row is valid.

Define behavior for the header row:

Preferred behavior:

- treat the first row as header;
- include it in `total_rows`;
- do not count it as data duplicate;
- use it to define expected column count;
- do not count it as valid data row.

Document and test this behavior.

## Error handling

If file cannot be opened:

- return failed `ExecutionResult`;
- include clear error message.

## Test requirements

Add tests covering:

1. valid CSV file;
2. CSV with empty fields;
3. CSV with duplicate rows;
4. CSV with inconsistent column count;
5. missing file returns failure;
6. empty file behavior;
7. executor type is `csv_validate`.

Use temporary files in tests.

## Integration

Register `CsvValidateExecutor` in tests with `ExecutorRegistry` if it already exists.

Do not modify application startup broadly unless there is already a clear place for executor registration.

## CMake requirements

Update CMake so the new source and tests are compiled.

Keep the project buildable.

## Hard constraints

Do not:

- implement full RFC 4180 CSV parser;
- implement HTTP;
- implement SQLite;
- implement retry;
- implement cancellation;
- implement file upload.

This task is only about CSV validation execution.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. all tests pass;
4. `CsvValidateExecutor` returns expected result fields;
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
