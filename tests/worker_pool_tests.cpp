#include "taskforge/app/InMemoryJobService.hpp"
#include "taskforge/core/WorkerPool.hpp"
#include "taskforge/executors/ExecutorRegistry.hpp"
#include "taskforge/executors/LineCountExecutor.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

std::filesystem::path temporary_file_path(const std::string& name) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
        / ("taskforge_worker_" + name + "_" + std::to_string(stamp) + ".txt");
}

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path);
    output << content;
}

template <typename Predicate>
bool wait_until(Predicate predicate, std::chrono::milliseconds timeout = std::chrono::seconds(2)) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return predicate();
}

void register_line_count(taskforge::executors::ExecutorRegistry& registry) {
    registry.register_executor(std::make_shared<taskforge::executors::LineCountExecutor>());
}

void test_worker_executes_line_count_job() {
    const auto path = temporary_file_path("line_count");
    write_file(path, "one\ntwo\nthree\n");

    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    taskforge::executors::ExecutorRegistry registry;
    register_line_count(registry);

    taskforge::core::WorkerPool workers{1, queue, service, registry};
    workers.start();
    const auto job = service.create_job({"line_count", path.string()});

    const bool done = wait_until([&] {
        const auto stored = service.get_job(job.id());
        return stored.has_value() && stored->status() == taskforge::core::JobStatus::Done;
    });

    workers.stop();
    std::filesystem::remove(path);

    expect(done, "worker executes line_count job and marks it done");
}

void test_worker_stores_result() {
    const auto path = temporary_file_path("result");
    write_file(path, "one\ntwo\n");

    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    taskforge::executors::ExecutorRegistry registry;
    register_line_count(registry);

    taskforge::core::WorkerPool workers{1, queue, service, registry};
    workers.start();
    const auto job = service.create_job({"line_count", path.string()});

    const bool has_result = wait_until([&] {
        const auto stored = service.get_job(job.id());
        return stored.has_value()
            && stored->status() == taskforge::core::JobStatus::Done
            && stored->result().has_value()
            && *stored->result() == "{\"lines\":2}";
    });

    workers.stop();
    std::filesystem::remove(path);

    expect(has_result, "worker stores executor result");
}

void test_missing_executor_sets_failed() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    taskforge::executors::ExecutorRegistry registry;

    taskforge::core::WorkerPool workers{1, queue, service, registry};
    workers.start();
    const auto job = service.create_job({"unknown", "file-1"});

    const bool failed = wait_until([&] {
        const auto stored = service.get_job(job.id());
        return stored.has_value()
            && stored->status() == taskforge::core::JobStatus::Failed
            && stored->error().has_value();
    });

    workers.stop();

    expect(failed, "missing executor marks job failed");
}

void test_executor_failure_sets_failed() {
    const auto path = temporary_file_path("missing_input");

    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    taskforge::executors::ExecutorRegistry registry;
    register_line_count(registry);

    taskforge::core::WorkerPool workers{1, queue, service, registry};
    workers.start();
    const auto job = service.create_job({"line_count", path.string()});

    const bool failed = wait_until([&] {
        const auto stored = service.get_job(job.id());
        return stored.has_value()
            && stored->status() == taskforge::core::JobStatus::Failed
            && stored->error().has_value();
    });

    workers.stop();

    expect(failed, "executor failure marks job failed");
}

void test_worker_pool_stops_cleanly() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    taskforge::executors::ExecutorRegistry registry;

    taskforge::core::WorkerPool workers{2, queue, service, registry};
    workers.start();
    workers.stop();
    workers.stop();

    expect(queue.is_shutdown(), "worker pool stop shuts down queue");
}

void test_multiple_jobs_can_be_processed() {
    std::vector<std::filesystem::path> paths;
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    taskforge::executors::ExecutorRegistry registry;
    register_line_count(registry);

    for (int i = 0; i < 5; ++i) {
        const auto path = temporary_file_path("multi_" + std::to_string(i));
        write_file(path, "one\ntwo\n");
        paths.push_back(path);
    }

    taskforge::core::WorkerPool workers{3, queue, service, registry};
    workers.start();

    std::vector<taskforge::core::JobId> ids;
    for (const auto& path : paths) {
        ids.push_back(service.create_job({"line_count", path.string()}).id());
    }

    const bool all_done = wait_until([&] {
        for (const auto& id : ids) {
            const auto stored = service.get_job(id);
            if (!stored.has_value() || stored->status() != taskforge::core::JobStatus::Done) {
                return false;
            }
        }
        return true;
    });

    workers.stop();

    for (const auto& path : paths) {
        std::filesystem::remove(path);
    }

    expect(all_done, "multiple jobs can be processed");
}

} // namespace

int run_worker_pool_tests() {
    test_worker_executes_line_count_job();
    test_worker_stores_result();
    test_missing_executor_sets_failed();
    test_executor_failure_sets_failed();
    test_worker_pool_stops_cleanly();
    test_multiple_jobs_can_be_processed();

    return failures == 0 ? 0 : 1;
}
