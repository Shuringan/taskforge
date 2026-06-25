# TaskForge

TaskForge is a planned C++20 backend service for asynchronous file processing.

Current status: bootstrap only. The project currently provides a minimal CMake
build, a simple executable, and a basic test target. Full functionality will be
implemented task by task.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Run

After building, run the executable from the build directory:

```bash
./build/taskforge
```

On Windows with a multi-configuration generator, the executable may be under a
configuration subdirectory such as `build/Debug/taskforge.exe`.

To start the current HTTP server:

```bash
./build/taskforge --serve
```

The initial HTTP API exposes only:

- `GET /health`
- `GET /metrics`

The HTTP layer uses `cpp-httplib` as a small header-only dependency downloaded
by CMake. Boost.Beast was not used in this stage to keep the Windows bootstrap
small and practical.

## Planned Functionality

Future tasks will add the job model, queue, worker pool, file processing,
storage, and HTTP API. These features are not implemented in the bootstrap
stage.
