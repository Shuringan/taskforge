# TASK_15_DOCUMENTATION_AND_EXAMPLES.md

# Task 15 — Documentation and examples

## Goal

Finalize user-facing documentation and examples.

This task should improve documentation only. It should not add major new functionality.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-14.

Implement only this task.

## Scope

Create or update:

```text
README.md
docs/api.md
docs/architecture.md
docs/examples.md
docs/resume.md
data/samples/
scripts/
```

Only create sample data and scripts that are useful for documentation and manual testing.

## README requirements

README must be in English.

Required sections:

```text
# TaskForge

## Overview
## Features
## Architecture
## Job lifecycle
## API
## Build
## Run
## Tests
## Examples
## Design decisions
## Limitations
## Future improvements
```

README must not claim features that are not actually implemented.

## docs/api.md

Document all implemented endpoints.

For each endpoint include:

- method and path;
- purpose;
- request body if any;
- success response;
- error responses;
- curl example.

Expected endpoints if implemented:

```http
POST /files
GET /files/{file_id}
POST /jobs
GET /jobs
GET /jobs/{job_id}
GET /jobs/{job_id}/result
POST /jobs/{job_id}/retry
POST /jobs/{job_id}/cancel
GET /health
GET /metrics
```

If some endpoints are not implemented, clearly mark them as planned or omit them.

## docs/architecture.md

Explain:

- project layers;
- main components;
- job lifecycle;
- worker pool flow;
- queue behavior;
- SQLite persistence;
- retry;
- cancellation;
- recovery after restart.

Add ASCII diagram:

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

## docs/examples.md

Include complete manual scenarios.

At minimum:

1. register file;
2. create `line_count` job;
3. get job status;
4. get result;
5. create `csv_validate` job;
6. retry failed job scenario if easy to reproduce;
7. cancel pending job scenario if easy to reproduce.

## docs/resume.md

Add a concise portfolio/resume description.

Include:

- short project summary;
- technologies;
- bullet points for implemented features;
- suggested resume entry.

## Scripts

Optional but useful:

```text
scripts/register_file.sh
scripts/create_job.sh
scripts/get_job.sh
scripts/get_result.sh
```

Scripts must be simple and documented.

## Sample data

Add sample files if useful:

```text
data/samples/simple.txt
data/samples/products_valid.csv
data/samples/products_invalid.csv
```

Keep them small.

## Hard constraints

Do not:

- add major new C++ functionality;
- rewrite architecture;
- add unrelated dependencies;
- claim unimplemented features;
- remove tests.

This task is documentation and examples only, except for small sample files/scripts.

## Acceptance criteria

The task is complete when:

1. project still configures;
2. project still builds;
3. tests still pass;
4. README is accurate;
5. API documentation exists;
6. architecture documentation exists;
7. examples are usable;
8. resume description exists.

## Suggested verification commands

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Completion response

After implementation, report:

1. summary of documentation changes;
2. files changed;
3. commands run;
4. build/test result;
5. known documentation limitations.
