#include "taskforge/core/Job.hpp"

namespace taskforge::core {

Job::Job(JobId id, std::string type, int priority, int max_attempts)
    : id_(std::move(id)),
      type_(std::move(type)),
      priority_(priority),
      max_attempts_(max_attempts),
      created_at_(std::chrono::system_clock::now()) {
    if (type_.empty()) {
        throw std::invalid_argument("Job type must not be empty");
    }
    if (max_attempts_ < 1) {
        throw std::invalid_argument("Job max_attempts must be at least 1");
    }
}

const JobId& Job::id() const noexcept {
    return id_;
}

const std::string& Job::type() const noexcept {
    return type_;
}

JobStatus Job::status() const noexcept {
    return status_;
}

int Job::priority() const noexcept {
    return priority_;
}

int Job::attempts() const noexcept {
    return attempts_;
}

int Job::max_attempts() const noexcept {
    return max_attempts_;
}

std::chrono::system_clock::time_point Job::created_at() const noexcept {
    return created_at_;
}

void Job::set_status(JobStatus status) noexcept {
    status_ = status;
}

void Job::increment_attempts() noexcept {
    ++attempts_;
}

} // namespace taskforge::core
