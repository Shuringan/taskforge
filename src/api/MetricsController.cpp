#include "taskforge/api/MetricsController.hpp"

namespace taskforge::api {

MetricsController::MetricsController(const core::JobQueue& queue, const app::InMemoryJobService& jobs)
    : queue_(queue),
      jobs_(jobs) {
}

ApiResponse MetricsController::handle() const {
    std::size_t pending = 0;
    std::size_t running = 0;
    std::size_t done = 0;
    std::size_t failed = 0;
    std::size_t cancelled = 0;

    const auto jobs = jobs_.list_jobs();
    for (const auto& job : jobs) {
        switch (job.status()) {
        case core::JobStatus::Pending:
            ++pending;
            break;
        case core::JobStatus::Running:
            ++running;
            break;
        case core::JobStatus::Done:
            ++done;
            break;
        case core::JobStatus::Failed:
            ++failed;
            break;
        case core::JobStatus::Cancelled:
            ++cancelled;
            break;
        }
    }

    return ApiResponse{
        200,
        "application/json",
        "{\"queue_size\":" + std::to_string(queue_.size())
            + ",\"jobs_total\":" + std::to_string(jobs.size())
            + ",\"jobs_pending\":" + std::to_string(pending)
            + ",\"jobs_running\":" + std::to_string(running)
            + ",\"jobs_done\":" + std::to_string(done)
            + ",\"jobs_failed\":" + std::to_string(failed)
            + ",\"jobs_cancelled\":" + std::to_string(cancelled)
            + "}"
    };
}

} // namespace taskforge::api
