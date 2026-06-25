# TASK_16_DOCKER_AND_CI.md

# Task 16 — Docker and CI

## Goal

Add reproducible container build and GitHub Actions workflow.

This task should make the project easier to build and verify from a clean environment.

Do not add new application features in this task.

## Context

Read before working:

- `PROJECT_SPEC.md`
- `AGENTS.md`
- `TASKS.md`
- existing code from Tasks 01-15.

Implement only this task.

## Scope

Create or update:

```text
Dockerfile
.dockerignore
.github/workflows/ci.yml
README.md
```

Optionally:

```text
docker-compose.yml
```

if it is genuinely useful.

## Dockerfile requirements

The Dockerfile should:

1. start from a suitable Linux image;
2. install build dependencies;
3. configure CMake;
4. build the project;
5. run tests during build or provide a test stage;
6. produce a runnable image if HTTP server exists.

A multi-stage Dockerfile is preferred but not required.

## Dependencies

Install only required packages.

Likely packages:

```text
cmake
g++
git
libsqlite3-dev
```

Boost packages may be needed if HTTP uses Boost.

Do not install unnecessary large packages.

## Runtime behavior

If the application has an HTTP server, the runtime image should expose port 8080.

Example:

```dockerfile
EXPOSE 8080
CMD ["./taskforge"]
```

Adjust path based on actual build output.

## .dockerignore

Exclude:

```text
build/
.git/
.github/
.vscode/
.idea/
*.zip
*.tar
```

Keep source files included.

## GitHub Actions CI

Create workflow:

```text
.github/workflows/ci.yml
```

The workflow should:

1. run on push and pull request;
2. checkout repository;
3. install dependencies;
4. configure CMake;
5. build;
6. run tests.

Use Ubuntu runner.

Suggested commands:

```yaml
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Adjust as needed.

## README updates

Add:

- Docker build command;
- Docker run command;
- CI badge if desired.

Do not claim Docker works unless the Dockerfile is actually reasonable and build commands are documented.

## Test requirements

No new C++ tests are required.

Existing tests must continue to pass.

## Hard constraints

Do not:

- add new application features;
- rewrite build system unnecessarily;
- remove existing tests;
- ignore dependency failures silently.

This task is only about Docker and CI.

## Acceptance criteria

The task is complete when:

1. project still configures locally;
2. project still builds locally;
3. tests still pass locally;
4. Dockerfile exists;
5. `.dockerignore` exists;
6. GitHub Actions workflow exists;
7. README contains Docker/CI instructions.

## Suggested verification commands

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

If Docker is available:

```bash
docker build -t taskforge .
docker run --rm -p 8080:8080 taskforge
```

## Completion response

After implementation, report:

1. summary of changes;
2. files changed;
3. commands run;
4. local build/test result;
5. Docker build result if run;
6. known limitations.
