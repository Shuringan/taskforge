#include "taskforge/api/FileController.hpp"
#include "taskforge/api/HealthController.hpp"
#include "taskforge/api/HttpServer.hpp"
#include "taskforge/api/JobController.hpp"
#include "taskforge/api/MetricsController.hpp"
#include "taskforge/app/InMemoryFileService.hpp"
#include "taskforge/app/InMemoryJobService.hpp"
#include "taskforge/core/WorkerPool.hpp"
#include "taskforge/executors/ExecutorRegistry.hpp"
#include "taskforge/executors/JobExecutor.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
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

class FailOnceExecutor final : public taskforge::executors::JobExecutor {
public:
    std::string type() const override {
        return "fail_once";
    }

    taskforge::executors::ExecutionResult execute(const std::filesystem::path&) override {
        if (calls_.fetch_add(1) == 0) {
            return taskforge::executors::ExecutionResult::fail("first failure");
        }

        return taskforge::executors::ExecutionResult::ok("{\"ok\":true}");
    }

private:
    std::atomic<int> calls_{0};
};

std::filesystem::path temporary_file_path(const std::string& name) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
        / ("taskforge_retry_" + name + "_" + std::to_string(stamp) + ".txt");
}

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path);
    output << content;
}

std::string extract_id(const std::string& body, const std::string& field) {
    const std::string marker = "\"" + field + "\":\"";
    const auto start = body.find(marker);
    if (start == std::string::npos) {
        return "";
    }
    const auto value_start = start + marker.size();
    const auto value_end = body.find('"', value_start);
    return body.substr(value_start, value_end - value_start);
}

void test_failed_job_can_be_retried() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    const auto job = service.create_job({"line_count", "file-1", 0, 2});
    service.update_status(job.id(), taskforge::core::JobStatus::Failed);
    service.store_error(job.id(), "failed");

    const auto result = service.retry_job(job.id());
    const auto stored = service.get_job(job.id());

    expect(result.status == taskforge::app::RetryJobStatus::Retried, "failed job can be retried");
    expect(stored.has_value() && stored->status() == taskforge::core::JobStatus::Pending,
           "retry changes status to pending");
    expect(stored.has_value() && !stored->error().has_value(), "retry clears previous error");
}

void test_retry_pushes_job_id_to_queue() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    const auto job = service.create_job({"line_count", "file-1", 0, 2});
    queue.wait_and_pop();
    service.update_status(job.id(), taskforge::core::JobStatus::Failed);

    service.retry_job(job.id());
    const auto queued = queue.wait_and_pop();

    expect(queued.has_value() && *queued == job.id(), "retry pushes job id to queue");
}

void test_retry_rejects_non_failed_job() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    const auto job = service.create_job({"line_count", "file-1", 0, 2});

    const auto result = service.retry_job(job.id());

    expect(result.status == taskforge::app::RetryJobStatus::NotFailed,
           "retry rejects non-failed job");
}

void test_retry_rejects_missing_job() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto result = service.retry_job(taskforge::core::JobId{"missing"});

    expect(result.status == taskforge::app::RetryJobStatus::NotFound,
           "retry rejects missing job");
}

void test_retry_rejects_exhausted_attempts() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    const auto job = service.create_job({"line_count", "file-1", 0, 1});
    service.increment_attempts(job.id());
    service.update_status(job.id(), taskforge::core::JobStatus::Failed);

    const auto result = service.retry_job(job.id());

    expect(result.status == taskforge::app::RetryJobStatus::AttemptsExhausted,
           "retry rejects exhausted attempts");
}

void test_worker_automatic_retry() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    taskforge::executors::ExecutorRegistry registry;
    registry.register_executor(std::make_shared<FailOnceExecutor>());

    taskforge::core::WorkerPool workers{1, queue, service, registry};
    workers.start();
    const auto job = service.create_job({"fail_once", "file-1", 0, 2});

    const bool done = wait_until([&] {
        const auto stored = service.get_job(job.id());
        return stored.has_value()
            && stored->status() == taskforge::core::JobStatus::Done
            && stored->attempts() == 1
            && stored->result().has_value();
    });

    workers.stop();

    expect(done, "worker automatically retries failed job and then succeeds");
}

void test_http_retry_endpoint_success_and_invalid_state() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryFileService files;
    taskforge::app::InMemoryJobService jobs{queue};
    taskforge::api::HealthController health{queue};
    taskforge::api::MetricsController metrics{queue, jobs};
    taskforge::api::FileController file_controller{files};
    taskforge::api::JobController job_controller{jobs, files};
    taskforge::api::HttpServer server{
        "127.0.0.1",
        8080,
        health,
        metrics,
        file_controller,
        job_controller
    };

    const auto path = temporary_file_path("http");
    write_file(path, "hello");
    const auto file_response = server.handle_request(
        "POST",
        "/files",
        "{\"path\":\"" + path.generic_string() + "\",\"original_name\":\"retry.txt\"}"
    );
    const auto file_id = extract_id(file_response.body, "file_id");
    const auto create_response = server.handle_request(
        "POST",
        "/jobs",
        "{\"type\":\"line_count\",\"file_id\":\"" + file_id + "\",\"max_attempts\":2}"
    );
    const auto job_id = extract_id(create_response.body, "job_id");
    jobs.update_status(taskforge::core::JobId{job_id}, taskforge::core::JobStatus::Failed);

    const auto retry_response = server.handle_request("POST", "/jobs/" + job_id + "/retry");
    const auto invalid_response = server.handle_request("POST", "/jobs/" + job_id + "/retry");

    expect(retry_response.status == 200, "HTTP retry endpoint succeeds for failed job");
    expect(retry_response.body.find("\"status\":\"pending\"") != std::string::npos,
           "HTTP retry endpoint returns pending status");
    expect(invalid_response.status == 409, "HTTP retry endpoint rejects non-failed job");

    std::filesystem::remove(path);
}

} // namespace

int run_retry_tests() {
    test_failed_job_can_be_retried();
    test_retry_pushes_job_id_to_queue();
    test_retry_rejects_non_failed_job();
    test_retry_rejects_missing_job();
    test_retry_rejects_exhausted_attempts();
    test_worker_automatic_retry();
    test_http_retry_endpoint_success_and_invalid_state();

    return failures == 0 ? 0 : 1;
}
