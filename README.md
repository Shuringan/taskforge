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

## Planned Functionality

Future tasks will add the job model, queue, worker pool, file processing,
storage, and HTTP API. These features are not implemented in the bootstrap
stage.
