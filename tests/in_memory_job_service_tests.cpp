#include "taskforge/app/InMemoryJobService.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

taskforge::app::CreateJobRequest valid_request() {
    return taskforge::app::CreateJobRequest{
        "line_count",
        "file-1",
        3,
        2
    };
}

void test_create_job_returns_pending_job() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto job = service.create_job(valid_request());

    expect(job.status() == taskforge::core::JobStatus::Pending, "created job is pending");
    expect(job.type() == "line_count", "created job stores type");
    expect(job.file_id() == "file-1", "created job stores file_id");
    expect(job.priority() == 3, "created job stores priority");
    expect(job.max_attempts() == 2, "created job stores max_attempts");
}

void test_create_job_pushes_id_to_queue() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto job = service.create_job(valid_request());
    const auto queued = queue.wait_and_pop();

    expect(queued.has_value(), "created job pushes id to queue");
    expect(queued == job.id(), "queued id matches created job id");
}

void test_get_created_job() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto job = service.create_job(valid_request());
    const auto stored = service.get_job(job.id());

    expect(stored.has_value(), "created job can be retrieved");
    expect(stored->id() == job.id(), "retrieved job id matches created job id");
}

void test_get_missing_job_returns_empty() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto missing = service.get_job(taskforge::core::JobId{"missing"});

    expect(!missing.has_value(), "missing job returns empty optional");
}

void test_list_jobs_returns_created_jobs() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    service.create_job(valid_request());
    service.create_job(taskforge::app::CreateJobRequest{"word_count", "file-2"});

    const auto jobs = service.list_jobs();

    expect(jobs.size() == 2, "list_jobs returns created jobs");
}

void test_update_status() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto job = service.create_job(valid_request());
    const bool updated = service.update_status(job.id(), taskforge::core::JobStatus::Running);
    const auto stored = service.get_job(job.id());

    expect(updated, "update_status returns true for existing job");
    expect(stored.has_value() && stored->status() == taskforge::core::JobStatus::Running,
           "update_status changes stored job");
}

void test_store_result() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto job = service.create_job(valid_request());
    const bool stored_result = service.store_result(job.id(), "{\"lines\":3}");
    const auto stored = service.get_job(job.id());

    expect(stored_result, "store_result returns true for existing job");
    expect(stored.has_value() && stored->result().has_value() && *stored->result() == "{\"lines\":3}",
           "store_result stores result");
}

void test_store_error() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    const auto job = service.create_job(valid_request());
    const bool stored_error = service.store_error(job.id(), "failed");
    const auto stored = service.get_job(job.id());

    expect(stored_error, "store_error returns true for existing job");
    expect(stored.has_value() && stored->error().has_value() && *stored->error() == "failed",
           "store_error stores error");
}

void test_rejects_empty_type() {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryJobService service{queue};

    bool thrown = false;
    try {
        service.create_job(taskforge::app::CreateJobRequest{"", "file-1"});
    } catch (const std::invalid_argument&) {
        thrown = true;
    }

    expect(thrown, "service rejects empty job type");
}

} // namespace

int run_in_memory_job_service_tests() {
    test_create_job_returns_pending_job();
    test_create_job_pushes_id_to_queue();
    test_get_created_job();
    test_get_missing_job_returns_empty();
    test_list_jobs_returns_created_jobs();
    test_update_status();
    test_store_result();
    test_store_error();
    test_rejects_empty_type();

    return failures == 0 ? 0 : 1;
}
