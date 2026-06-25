#include "taskforge/app/JobRecoveryService.hpp"

namespace taskforge::app {

namespace {

constexpr const char* interrupted_error = "Service was restarted during job execution";

} // namespace

JobRecoveryService::JobRecoveryService(storage::JobRepository& jobs, core::JobQueue& queue)
    : jobs_(jobs),
      queue_(queue) {
}

RecoverySummary JobRecoveryService::recover() {
    RecoverySummary summary;

    for (const auto& job : jobs_.list_by_status(core::JobStatus::Pending)) {
        queue_.push(job.id());
        ++summary.requeued_pending_jobs;
    }

    for (const auto& job : jobs_.list_by_status(core::JobStatus::Running)) {
        jobs_.update_error(job.id(), interrupted_error);
        jobs_.update_status(job.id(), core::JobStatus::Failed);
        ++summary.failed_running_jobs;
    }

    return summary;
}

} // namespace taskforge::app
