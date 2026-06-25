# AGENTS.md

# Codex instructions for TaskForge

This repository is developed incrementally. Do not try to implement the entire project from `PROJECT_SPEC.md` in one run.

## 1. Primary rule

Implement only the current task file.

If the current task is `tasks/TASK_08_CSV_VALIDATE_EXECUTOR.md`, implement only that task. Do not start `TASK_09` or later.

## 2. Files to read before working

Before making changes, read:

1. `PROJECT_SPEC.md`
2. `TASKS.md`
3. the current task file from `tasks/`

Use `PROJECT_SPEC.md` for product direction only. Use the current task file as the binding implementation scope.

## 3. Scope control

Do not:

- implement future stages early;
- add placeholder systems for future stages unless the current task explicitly requires them;
- create fake implementations that only satisfy names;
- add large abstractions without immediate use;
- rewrite unrelated files;
- change public behavior unrelated to the current task;
- silently remove tests;
- ignore failing tests.

Prefer small, concrete, verifiable changes.

## 4. Build system

Use CMake.

The baseline commands should be:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

If the generator or platform requires a different command, document it in the task summary or README.

## 5. C++ standard

Use C++20.

The project should compile with a modern compiler supporting C++20.

Prefer portable standard C++ where practical.

## 6. Testing

Add or update tests for the behavior implemented in the current task.

Use GoogleTest if it is already configured. If GoogleTest is not yet available in the current stage, follow the current task instructions.

Do not claim tests pass unless they were actually run. If tests were not run, state that explicitly and explain why.

## 7. Code style

General rules:

- keep code simple and readable;
- avoid unnecessary inheritance;
- avoid global mutable state;
- use RAII for resource management;
- prefer `std::chrono` for time;
- prefer `std::optional` where absence is meaningful;
- keep HTTP, storage, and core logic separate once these layers exist;
- do not introduce busy waiting;
- protect shared state with appropriate synchronization;
- use clear names.

## 8. Error handling

Prefer explicit error reporting over silent failure.

For core classes, use clear return values, exceptions, or small result types where appropriate. Do not swallow errors.

For HTTP endpoints, errors must be JSON responses.

## 9. Concurrency rules

For queue and worker code:

- use `std::mutex` and `std::condition_variable`;
- do not spin in loops waiting for work;
- provide a clean shutdown path;
- avoid data races;
- tests should cover shutdown behavior where practical.

## 10. Repository structure

Recommended structure:

```text
include/taskforge/
  core/
  app/
  api/
  storage/
  executors/
  config/
  utils/

src/
  core/
  app/
  api/
  storage/
  executors/
  config/
  utils/

tests/
tasks/
docs/
```

Not every directory must exist from the first task. Create directories only when they are needed.

## 11. Documentation

Update README or documentation only when the current task changes build instructions, public usage, or project structure.

Do not write documentation claiming features that are not implemented yet.

It is acceptable to include a "Planned features" section if it is clearly marked as planned.

## 12. Completion response

At the end of each task, provide:

1. summary of changes;
2. list of important changed files;
3. commands run;
4. test/build result;
5. any known limitations or follow-up work.

## 13. If blocked

If a task cannot be completed:

- stop making unrelated changes;
- explain what blocked implementation;
- identify the file or dependency involved;
- suggest the smallest next step.

Do not hallucinate successful implementation.
