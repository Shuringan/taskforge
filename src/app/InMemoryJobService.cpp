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

bool InMemoryJobService::increment_attempts(const core::JobId& id) {
    std::lock_guard lock(mutex_);
    const auto it = jobs_.find(id.value());
    if (it == jobs_.end()) {
        return false;
    }

    it->second.increment_attempts();
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

bool InMemoryJobService::clear_error(const core::JobId& id) {
    std::lock_guard lock(mutex_);
    const auto it = jobs_.find(id.value());
    if (it == jobs_.end()) {
        return false;
    }

    it->second.clear_error();
    return true;
}

RetryJobResult InMemoryJobService::retry_job(const core::JobId& id) {
    core::Job job_to_return{core::JobId{"temporary"}, "temporary"};
    {
        std::lock_guard lock(mutex_);
        const auto it = jobs_.find(id.value());
        if (it == jobs_.end()) {
            return RetryJobResult{RetryJobStatus::NotFound, std::nullopt, "Job not found"};
        }
        if (it->second.status() != core::JobStatus::Failed) {
            return RetryJobResult{RetryJobStatus::NotFailed, it->second, "Only failed jobs can be retried"};
        }
        if (it->second.attempts() >= it->second.max_attempts()) {
            return RetryJobResult{RetryJobStatus::AttemptsExhausted, it->second, "Maximum retry attempts reached"};
        }

        it->second.set_status(core::JobStatus::Pending);
        it->second.clear_error();
        job_to_return = it->second;
    }

    queue_.push(id);
    return RetryJobResult{RetryJobStatus::Retried, job_to_return, ""};
}

core::JobId InMemoryJobService::next_job_id() {
    std::lock_guard lock(mutex_);
    return core::JobId{"job-" + std::to_string(next_id_++)};
}

} // namespace taskforge::app
