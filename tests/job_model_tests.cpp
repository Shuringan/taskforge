#include "taskforge/core/Job.hpp"
#include "taskforge/core/JobId.hpp"
#include "taskforge/core/JobStatus.hpp"

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

void test_job_id_stores_value() {
    const taskforge::core::JobId id{"job-1"};
    expect(id.value() == "job-1", "JobId stores value");
}

void test_empty_job_id_is_rejected() {
    bool thrown = false;
    try {
        const taskforge::core::JobId id{""};
        (void)id;
    } catch (const std::invalid_argument&) {
        thrown = true;
    }

    expect(thrown, "empty JobId is rejected");
}

void test_job_status_to_string() {
    using taskforge::core::JobStatus;
    using taskforge::core::to_string;

    expect(to_string(JobStatus::Pending) == "pending", "Pending converts to string");
    expect(to_string(JobStatus::Running) == "running", "Running converts to string");
    expect(to_string(JobStatus::Done) == "done", "Done converts to string");
    expect(to_string(JobStatus::Failed) == "failed", "Failed converts to string");
    expect(to_string(JobStatus::Cancelled) == "cancelled", "Cancelled converts to string");
}

void test_job_status_from_valid_string() {
    using taskforge::core::JobStatus;
    using taskforge::core::job_status_from_string;

    expect(job_status_from_string("pending") == JobStatus::Pending, "pending converts to JobStatus");
    expect(job_status_from_string("running") == JobStatus::Running, "running converts to JobStatus");
    expect(job_status_from_string("done") == JobStatus::Done, "done converts to JobStatus");
    expect(job_status_from_string("failed") == JobStatus::Failed, "failed converts to JobStatus");
    expect(job_status_from_string("cancelled") == JobStatus::Cancelled, "cancelled converts to JobStatus");
}

void test_job_status_from_invalid_string() {
    expect(!taskforge::core::job_status_from_string("missing").has_value(),
           "invalid status returns empty optional");
}

void test_new_job_defaults() {
    const taskforge::core::Job job{taskforge::core::JobId{"job-1"}, "line_count"};

    expect(job.status() == taskforge::core::JobStatus::Pending, "new Job has Pending status");
    expect(job.attempts() == 0, "new Job has zero attempts");
    expect(job.priority() == 0, "new Job has default priority");
    expect(job.max_attempts() == 1, "new Job has default max attempts");
}

void test_increment_attempts() {
    taskforge::core::Job job{taskforge::core::JobId{"job-1"}, "line_count"};
    job.increment_attempts();

    expect(job.attempts() == 1, "increment_attempts increments attempts");
}

void test_set_status() {
    taskforge::core::Job job{taskforge::core::JobId{"job-1"}, "line_count"};
    job.set_status(taskforge::core::JobStatus::Running);

    expect(job.status() == taskforge::core::JobStatus::Running, "set_status changes status");
}

} // namespace

int run_job_model_tests() {
    test_job_id_stores_value();
    test_empty_job_id_is_rejected();
    test_job_status_to_string();
    test_job_status_from_valid_string();
    test_job_status_from_invalid_string();
    test_new_job_defaults();
    test_increment_attempts();
    test_set_status();

    return failures == 0 ? 0 : 1;
}
