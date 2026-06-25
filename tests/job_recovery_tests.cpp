#include "taskforge/app/JobRecoveryService.hpp"
#include "taskforge/storage/Database.hpp"
#include "taskforge/storage/SQLiteFileRepository.hpp"
#include "taskforge/storage/SQLiteJobRepository.hpp"

#include <chrono>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

taskforge::core::FileRecord make_file() {
    return taskforge::core::FileRecord{
        taskforge::core::FileId{"file-1"},
        "input.txt",
        "input.txt",
        10,
        std::chrono::system_clock::now()
    };
}

taskforge::core::Job make_job(const std::string& id, taskforge::core::JobStatus status) {
    taskforge::core::Job job{
        taskforge::core::JobId{id},
        "line_count",
        "file-1"
    };
    job.set_status(status);
    return job;
}

struct RecoveryFixture {
    taskforge::storage::Database database{":memory:"};
    taskforge::storage::SQLiteFileRepository files{database};
    taskforge::storage::SQLiteJobRepository jobs{database};
    taskforge::core::JobQueue queue;

    RecoveryFixture() {
        files.insert(make_file());
    }
};

void test_pending_jobs_are_pushed_to_queue() {
    RecoveryFixture fixture;
    const auto pending = make_job("job-pending", taskforge::core::JobStatus::Pending);
    fixture.jobs.insert(pending);

    taskforge::app::JobRecoveryService recovery{fixture.jobs, fixture.queue};
    recovery.recover();
    const auto queued = fixture.queue.wait_and_pop();

    expect(queued.has_value(), "pending job is pushed to queue");
    expect(queued.has_value() && *queued == pending.id(), "queued pending job id matches");
}

void test_running_jobs_are_marked_failed_with_error() {
    RecoveryFixture fixture;
    const auto running = make_job("job-running", taskforge::core::JobStatus::Running);
    fixture.jobs.insert(running);

    taskforge::app::JobRecoveryService recovery{fixture.jobs, fixture.queue};
    recovery.recover();
    const auto stored = fixture.jobs.find_by_id(running.id());

    expect(stored.has_value() && stored->status() == taskforge::core::JobStatus::Failed,
           "running job is marked failed");
    expect(stored.has_value()
               && stored->error().has_value()
               && *stored->error() == "Service was restarted during job execution",
           "running job receives interruption error");
}

void test_terminal_jobs_are_unchanged() {
    RecoveryFixture fixture;
    auto done = make_job("job-done", taskforge::core::JobStatus::Done);
    auto failed = make_job("job-failed", taskforge::core::JobStatus::Failed);
    auto cancelled = make_job("job-cancelled", taskforge::core::JobStatus::Cancelled);
    failed.set_error("original error");

    fixture.jobs.insert(done);
    fixture.jobs.insert(failed);
    fixture.jobs.insert(cancelled);

    taskforge::app::JobRecoveryService recovery{fixture.jobs, fixture.queue};
    recovery.recover();

    const auto stored_done = fixture.jobs.find_by_id(done.id());
    const auto stored_failed = fixture.jobs.find_by_id(failed.id());
    const auto stored_cancelled = fixture.jobs.find_by_id(cancelled.id());

    expect(stored_done.has_value() && stored_done->status() == taskforge::core::JobStatus::Done,
           "done job is unchanged");
    expect(stored_failed.has_value()
               && stored_failed->status() == taskforge::core::JobStatus::Failed
               && stored_failed->error().has_value()
               && *stored_failed->error() == "original error",
           "failed job is unchanged");
    expect(stored_cancelled.has_value()
               && stored_cancelled->status() == taskforge::core::JobStatus::Cancelled,
           "cancelled job is unchanged");
}

void test_recovery_summary_counts() {
    RecoveryFixture fixture;
    fixture.jobs.insert(make_job("job-pending-1", taskforge::core::JobStatus::Pending));
    fixture.jobs.insert(make_job("job-pending-2", taskforge::core::JobStatus::Pending));
    fixture.jobs.insert(make_job("job-running", taskforge::core::JobStatus::Running));
    fixture.jobs.insert(make_job("job-done", taskforge::core::JobStatus::Done));

    taskforge::app::JobRecoveryService recovery{fixture.jobs, fixture.queue};
    const auto summary = recovery.recover();

    expect(summary.requeued_pending_jobs == 2, "summary counts requeued pending jobs");
    expect(summary.failed_running_jobs == 1, "summary counts failed running jobs");
}

} // namespace

int run_job_recovery_tests() {
    test_pending_jobs_are_pushed_to_queue();
    test_running_jobs_are_marked_failed_with_error();
    test_terminal_jobs_are_unchanged();
    test_recovery_summary_counts();

    return failures == 0 ? 0 : 1;
}
