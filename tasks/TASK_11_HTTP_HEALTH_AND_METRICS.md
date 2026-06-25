# TASK_11_HTTP_HEALTH_AND_METRICS.md

# Task 11 — HTTP health and metrics

## Goal

Add the initial HTTP server with two endpoints:

```http
GET /health
GET /metrics
```

Do not implement file API or job API in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-10.

Implement only this task.

## Scope

Add API layer.

Recommended files:

```text
include/taskforge/api/HttpServer.hpp
src/api/HttpServer.cpp

include/taskforge/api/HealthController.hpp
src/api/HealthController.cpp

include/taskforge/api/MetricsController.hpp
src/api/MetricsController.cpp

tests/http_health_metrics_tests.cpp
```

Depending on chosen HTTP library, file names may differ.

## HTTP library

Preferred: Boost.Asio / Boost.Beast.

If using another lightweight library, document why.

Keep implementation simple and maintainable.

## Required behavior

### GET /health

Return HTTP 200 and JSON:

```json
{
  "status": "ok"
}
```

If available, include:

```json
{
  "status": "ok",
  "workers": 4,
  "queue_size": 0
}
```

### GET /metrics

Return HTTP 200 and JSON with available metrics.

Minimum fields:

```json
{
  "queue_size": 0,
  "jobs_total": 0,
  "jobs_pending": 0,
  "jobs_running": 0,
  "jobs_done": 0,
  "jobs_failed": 0,
  "jobs_cancelled": 0
}
```

If some counts are not yet available, implement them from current service/repository state if practical. Do not fake values unless clearly they are real defaults from empty state.

### Unknown route

Return 404 JSON:

```json
{
  "error": "Not found"
}
```

## Server configuration

For this task, server may use default host/port in code.

Preferred defaults:

```text
host: 127.0.0.1
port: 8080
```

Full config loading can be added later if not already available.

## Main executable

Update `src/main.cpp` to optionally start the HTTP server.

If this makes tests difficult, keep server startup simple and controlled.

## Test requirements

Add tests if practical.

Tests should cover:

1. `/health` returns 200;
2. `/metrics` returns 200;
3. unknown route returns 404.

If integration HTTP tests are too complex on the current platform, add controller/unit tests and document the limitation.

## Hard constraints

Do not:

- implement `POST /files`;
- implement `POST /jobs`;
- implement `GET /jobs`;
- implement retry or cancel endpoints;
- implement authentication;
- implement multipart upload.

This task is only about initial HTTP server and health/metrics endpoints.

## Acceptance criteria

The task is complete when:

1. project configures;
2. project builds;
3. tests pass where practical;
4. server can respond to `/health`;
5. server can respond to `/metrics`;
6. unknown route returns JSON 404;
7. no future-stage HTTP endpoints are implemented.

## Suggested verification commands

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Manual check example:

```bash
./build/taskforge
curl http://127.0.0.1:8080/health
curl http://127.0.0.1:8080/metrics
```

Adjust executable path for platform/generator.

## Completion response

After implementation, report:

1. summary of changes;
2. files changed;
3. commands run;
4. build/test result;
5. manual HTTP checks if run;
6. known limitations.
