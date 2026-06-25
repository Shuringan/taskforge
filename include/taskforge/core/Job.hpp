#pragma once

#include "taskforge/core/JobId.hpp"
#include "taskforge/core/JobStatus.hpp"

#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>

namespace taskforge::core {

class Job {
public:
    Job(JobId id, std::string type, int priority = 0, int max_attempts = 1);

    const JobId& id() const noexcept;
    const std::string& type() const noexcept;
    JobStatus status() const noexcept;
    int priority() const noexcept;
    int attempts() const noexcept;
    int max_attempts() const noexcept;
    std::chrono::system_clock::time_point created_at() const noexcept;

    void set_status(JobStatus status) noexcept;
    void increment_attempts() noexcept;

private:
    JobId id_;
    std::string type_;
    JobStatus status_{JobStatus::Pending};
    int priority_{0};
    int attempts_{0};
    int max_attempts_{1};
    std::chrono::system_clock::time_point created_at_;
};

} // namespace taskforge::core
