#pragma once

#include "taskforge/app/CancelJobResult.hpp"
#include "taskforge/app/CreateJobRequest.hpp"
#include "taskforge/app/RetryJobResult.hpp"
#include "taskforge/core/Job.hpp"
#include "taskforge/core/JobQueue.hpp"

#include <cstddef>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace taskforge::app {

class InMemoryJobService {
public:
    explicit InMemoryJobService(core::JobQueue& queue);

    core::Job create_job(const CreateJobRequest& request);

    std::optional<core::Job> get_job(const core::JobId& id) const;
    std::vector<core::Job> list_jobs() const;

    bool update_status(const core::JobId& id, core::JobStatus status);
    bool increment_attempts(const core::JobId& id);
    bool store_result(const core::JobId& id, std::string result);
    bool store_error(const core::JobId& id, std::string error);
    bool clear_error(const core::JobId& id);
    RetryJobResult retry_job(const core::JobId& id);
    CancelJobResult cancel_job(const core::JobId& id);
    bool is_cancellation_requested(const core::JobId& id) const;

private:
    core::JobId next_job_id();

    core::JobQueue& queue_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, core::Job> jobs_;
    std::unordered_set<std::string> cancellation_requests_;
    std::size_t next_id_{1};
};

} // namespace taskforge::app
