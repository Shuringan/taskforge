#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace taskforge::core {

enum class JobStatus {
    Pending,
    Running,
    Done,
    Failed,
    Cancelled
};

std::string to_string(JobStatus status);
std::optional<JobStatus> job_status_from_string(std::string_view value);

} // namespace taskforge::core
