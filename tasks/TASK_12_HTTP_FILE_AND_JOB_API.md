# TASK_12_HTTP_FILE_AND_JOB_API.md

# Task 12 — HTTP file and job API

## Goal

Add REST endpoints for file registration and job operations.

This task should expose already implemented application functionality through HTTP.

Do not implement retry or cancellation endpoints in this task. They have separate tasks.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-11.

Implement only this task.

## Scope

Add or update controllers/routes for:

```http
POST /files
GET /files/{file_id}

POST /jobs
GET /jobs
GET /jobs/{job_id}
GET /jobs/{job_id}/result
```

Recommended files:

```text
include/taskforge/api/FileController.hpp
src/api/FileController.cpp

include/taskforge/api/JobController.hpp
src/api/JobController.cpp

tests/http_file_job_api_tests.cpp
```

File names can differ depending on existing HTTP structure.

## JSON dependency

Use `nlohmann/json` if already available or add it if practical.

If another JSON approach exists in the codebase, use it consistently.

## Required endpoint behavior

### POST /files

Registers an existing local file path.

Request:

```json
{
  "path": "data/samples/products.csv",
  "original_name": "products.csv"
}
```

Response:

```json
{
  "file_id": "file_xxx",
  "original_name": "products.csv",
  "path": "data/samples/products.csv",
  "size": 1234
}
```

### GET /files/{file_id}

Returns file metadata.

If not found, return 404 JSON.

### POST /jobs

Creates a job.

Request:

```json
{
  "type": "csv_validate",
  "file_id": "file_xxx",
  "priority": 0,
  "max_attempts": 1
}
```

Response:

```json
{
  "job_id": "job_xxx",
  "status": "pending"
}
```

The endpoint should validate:

- non-empty type;
- known or at least syntactically valid file ID;
- positive max_attempts.

If file registry is available, verify that file exists.

### GET /jobs

Returns job list.

Support optional query parameters if practical:

```text
status
limit
offset
```

Minimum acceptable behavior: return all jobs.

### GET /jobs/{job_id}

Returns job metadata.

### GET /jobs/{job_id}/result

If job is `done`, return result.

If job is not done, return 409 JSON:

```json
{
  "error": "Job is not completed yet"
}
```

If job failed, include error if appropriate.

## Error responses

All API errors must be JSON.

Examples:

```json
{
  "error": "Invalid request body"
}
```

```json
{
  "error": "Job not found"
}
```

Use suitable HTTP status codes:

- 400 for invalid request;
- 404 for missing resource;
- 409 for invalid state;
- 500 for unexpected error.

## Integration with worker pool

Creating a job should put it into the queue using the existing service logic.

If the main application already starts workers, jobs may execute. If not, job creation must still work and return `pending`.

Do not over-rewrite application wiring.

## Test requirements

Add tests if practical.

Tests should cover:

1. register file;
2. get file by ID;
3. create job;
4. get job by ID;
5. list jobs;
6. get result for not completed job returns 409;
7. invalid JSON returns 400;
8. missing job returns 404.

If full HTTP integration tests are difficult, add controller tests and manual curl examples in README or docs.

## Hard constraints

Do not:

- implement retry endpoint;
- implement cancellation endpoint;
- implement multipart upload;
- implement authentication;
- rewrite worker pool unnecessarily.

This task is only about file and job HTTP API.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. tests pass where practical;
4. endpoints listed in this task work;
5. errors are JSON;
6. retry/cancel endpoints are not implemented yet.

## Suggested verification commands

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual check examples:

```bash
curl -X POST http://127.0.0.1:8080/files \
  -H "Content-Type: application/json" \
  -d '{"path":"data/samples/products.csv","original_name":"products.csv"}'

curl -X POST http://127.0.0.1:8080/jobs \
  -H "Content-Type: application/json" \
  -d '{"type":"csv_validate","file_id":"file_xxx","priority":0,"max_attempts":1}'
```

## Completion response

After implementation, report:

1. summary of changes;
2. files changed;
3. commands run;
4. build/test result;
5. manual HTTP checks if run;
6. known limitations.
