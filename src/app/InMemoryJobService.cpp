#include "taskforge/app/InMemoryJobService.hpp"

#include <stdexcept>
#include <utility>

namespace taskforge::app {

InMemoryJobService::InMemoryJobService(core::JobQueue& queue)
    : queue_(queue) {
}

core::Job InMemoryJobService::create_job(const CreateJobRequest& request) {
    if (request.type.empty()) {
        throw std::invalid_argument("Job type must not be empty");
    }

    core::Job job{next_job_id(), request.type, request.file_id, request.priority, request.max_attempts};
    const core::JobId id = job.id();

    {
        std::lock_guard lock(mutex_);
        jobs_.emplace(id.value(), job);
    }

    queue_.push(id);
    return job;
}

std::optional<core::Job> InMemoryJobService::get_job(const core::JobId& id) const {
    std::lock_guard lock(mutex_);
    const auto it = jobs_.find(id.value());
    if (it == jobs_.end()) {
        return std::nullopt;
    }

    return it->second;
}

std::vector<core::Job> InMemoryJobService::list_jobs() const {
    std::lock_guard lock(mutex_);
    std::vector<core::Job> jobs;
    jobs.reserve(jobs_.size());

    for (const auto& [id, job] : jobs_) {
        (void)id;
        jobs.push_back(job);
    }

    return jobs;
}

bool InMemoryJobService::update_status(const core::JobId& id, core::JobStatus status) {
    std::lock_guard lock(mutex_);
    const auto it = jobs_.find(id.value());
    if (it == jobs_.end()) {
        return false;
    }

    it->second.set_status(status);
    return true;
}

bool InMemoryJobService::store_result(const core::JobId& id, std::string result) {
    std::lock_guard lock(mutex_);
    const auto it = jobs_.find(id.value());
    if (it == jobs_.end()) {
        return false;
    }

    it->second.set_result(std::move(result));
    return true;
}

bool InMemoryJobService::store_error(const core::JobId& id, std::string error) {
    std::lock_guard lock(mutex_);
    const auto it = jobs_.find(id.value());
    if (it == jobs_.end()) {
        return false;
    }

    it->second.set_error(std::move(error));
    return true;
}

core::JobId InMemoryJobService::next_job_id() {
    std::lock_guard lock(mutex_);
    return core::JobId{"job-" + std::to_string(next_id_++)};
}

} // namespace taskforge::app
