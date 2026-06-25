# TaskForge

## Overview

TaskForge is a C++20 backend service for asynchronous local file processing.
It exposes a small HTTP API for registering existing local files, creating jobs,
checking job state, retrieving results, retrying failed work, and cancelling
pending work.

## Features

- CMake-based C++20 build.
- In-memory file and job services.
- Thread-safe job queue.
- Worker pool for background execution.
- Executors for `line_count`, `word_count`, and `csv_validate`.
- SQLite repository layer and startup recovery component.
- HTTP endpoints for files, jobs, health, and metrics.
- Retry support for failed jobs.
- Cooperative cancellation support for pending and running jobs.
- Unit/controller tests.

## Architecture

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

More detail is available in [docs/architecture.md](docs/architecture.md).

## Job lifecycle

```text
pending -> running -> done
pending -> running -> failed
pending -> cancelled
running -> cancelled
failed -> pending
```

The last transition happens when a failed job is retried.

## API

Implemented endpoints:

- `GET /health`
- `GET /metrics`
- `POST /files`
- `GET /files/{file_id}`
- `POST /jobs`
- `GET /jobs`
- `GET /jobs/{job_id}`
- `GET /jobs/{job_id}/result`
- `POST /jobs/{job_id}/retry`
- `POST /jobs/{job_id}/cancel`

See [docs/api.md](docs/api.md) for request and response examples.

## Build

```bash
cmake -S . -B build
cmake --build build
```

CMake downloads small source dependencies during configure:

- SQLite amalgamation;
- `cpp-httplib`;
- `nlohmann/json`.

## Run

Default executable behavior:

```bash
./build/taskforge
```

Start the HTTP server:

```bash
./build/taskforge --serve
```

On Windows with a multi-configuration generator, use:

```powershell
.\build\Debug\taskforge.exe --serve
```

Default server address:

```text
http://127.0.0.1:8080
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

On multi-configuration generators:

```bash
ctest --test-dir build --output-on-failure -C Debug
```

## Docker

Build the image:

```bash
docker build -t taskforge .
```

Run the HTTP server:

```bash
docker run --rm -p 8080:8080 taskforge
```

The Docker image sets `TASKFORGE_HOST=0.0.0.0` so the HTTP server is reachable
through the published container port.

## CI

GitHub Actions workflow: [.github/workflows/ci.yml](.github/workflows/ci.yml).

The workflow configures CMake, builds the project, and runs the test suite on
Ubuntu for pushes and pull requests.

## Examples

Sample files are available in [data/samples](data/samples).

Manual scenarios are documented in [docs/examples.md](docs/examples.md).

Simple curl helper scripts are available in [scripts](scripts).

## Design decisions

- `cpp-httplib` is used instead of Boost.Beast to keep the local Windows build
  small and practical.
- HTTP state is currently wired to in-memory services.
- SQLite repositories are implemented and tested, but the default HTTP server
  still uses the in-memory services.
- CSV parsing is intentionally simple: comma delimiter, trimmed fields, no
  quoted-field support.
- Cancellation is cooperative. Workers skip cancelled jobs and avoid overwriting
  cancelled running jobs after executor completion.

## Limitations

- No authentication.
- No multipart upload.
- No persistent HTTP application wiring yet.
- No OpenAPI generator.
- No delayed retry or exponential backoff.
- CSV parsing is not full RFC 4180.

## Future improvements

- Wire HTTP services to SQLite repositories.
- Add configuration loading.
- Add structured logging.
- Add OpenAPI documentation.
- Add persistent HTTP application wiring.
