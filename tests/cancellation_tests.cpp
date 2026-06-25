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

class CountingExecutor final : public taskforge::executors::JobExecutor {
public:
    explicit CountingExecutor(std::atomic<int>& calls)
        : calls_(calls) {
    }

    std::string type() const override {
        return "counting";
    }

    taskforge::executors::ExecutionResult execute(const std::filesystem::path&) override {
        ++calls_;
        return taskforge::executors::ExecutionResult::ok("{\"ok\":true}");
    }

private:
    std::atomic<int>& calls_;
};

std::filesystem::path temporary_file_path(const std::string& name) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
        / ("taskforge_cancel_" + name + "_" + std::to_string(stamp) + ".txt");
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

void test_pending_job_can_be_cancelled() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    const auto job = service.create_job({"line_count", "file-1"});

    const auto result = service.cancel_job(job.id());
    const auto stored = service.get_job(job.id());

    expect(result.status == taskforge::app::CancelJobStatus::Cancelled,
           "pending job can be cancelled");
    expect(stored.has_value() && stored->status() == taskforge::core::JobStatus::Cancelled,
           "cancelled pending job status is cancelled");
}

void test_cancelled_pending_job_is_not_executed() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    taskforge::executors::ExecutorRegistry registry;
    std::atomic<int> calls{0};
    registry.register_executor(std::make_shared<CountingExecutor>(calls));

    const auto job = service.create_job({"counting", "file-1"});
    service.cancel_job(job.id());

    taskforge::core::WorkerPool workers{1, queue, service, registry};
    workers.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    workers.stop();

    const auto stored = service.get_job(job.id());
    expect(calls.load() == 0, "worker skips cancelled pending job");
    expect(stored.has_value() && stored->status() == taskforge::core::JobStatus::Cancelled,
           "worker leaves cancelled job cancelled");
}

void test_cancelling_missing_job_fails() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto result = service.cancel_job(taskforge::core::JobId{"missing"});

    expect(result.status == taskforge::app::CancelJobStatus::NotFound,
           "cancelling missing job fails");
}

void test_cancelling_completed_jobs_fails() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};
    const auto done = service.create_job({"line_count", "file-1"});
    const auto failed = service.create_job({"line_count", "file-1"});
    const auto cancelled = service.create_job({"line_count", "file-1"});
    service.update_status(done.id(), taskforge::core::JobStatus::Done);
    service.update_status(failed.id(), taskforge::core::JobStatus::Failed);
    service.cancel_job(cancelled.id());

    const auto done_result = service.cancel_job(done.id());
    const auto failed_result = service.cancel_job(failed.id());
    const auto cancelled_result = service.cancel_job(cancelled.id());

    expect(done_result.status == taskforge::app::CancelJobStatus::InvalidState,
           "cancelling done job fails");
    expect(failed_result.status == taskforge::app::CancelJobStatus::InvalidState,
           "cancelling failed job fails");
    expect(cancelled_result.status == taskforge::app::CancelJobStatus::InvalidState,
           "cancelling already cancelled job fails consistently");
}

void test_http_cancel_endpoint() {
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
        "{\"path\":\"" + path.generic_string() + "\",\"original_name\":\"cancel.txt\"}"
    );
    const auto file_id = extract_id(file_response.body, "file_id");
    const auto create_response = server.handle_request(
        "POST",
        "/jobs",
        "{\"type\":\"line_count\",\"file_id\":\"" + file_id + "\"}"
    );
    const auto job_id = extract_id(create_response.body, "job_id");

    const auto cancel_response = server.handle_request("POST", "/jobs/" + job_id + "/cancel");
    const auto second_cancel_response = server.handle_request("POST", "/jobs/" + job_id + "/cancel");

    expect(cancel_response.status == 200, "HTTP cancel endpoint succeeds for pending job");
    expect(cancel_response.body.find("\"status\":\"cancelled\"") != std::string::npos,
           "HTTP cancel endpoint returns cancelled status");
    expect(second_cancel_response.status == 409,
           "HTTP cancel endpoint rejects already cancelled job");

    std::filesystem::remove(path);
}

} // namespace

int run_cancellation_tests() {
    test_pending_job_can_be_cancelled();
    test_cancelled_pending_job_is_not_executed();
    test_cancelling_missing_job_fails();
    test_cancelling_completed_jobs_fails();
    test_http_cancel_endpoint();

    return failures == 0 ? 0 : 1;
}
