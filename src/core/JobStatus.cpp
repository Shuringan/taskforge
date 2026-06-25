#include "taskforge/core/JobStatus.hpp"

namespace taskforge::core {

std::string to_string(JobStatus status) {
    switch (status) {
    case JobStatus::Pending:
        return "pending";
    case JobStatus::Running:
        return "running";
    case JobStatus::Done:
        return "done";
    case JobStatus::Failed:
        return "failed";
    case JobStatus::Cancelled:
        return "cancelled";
    }

    return "unknown";
}

std::optional<JobStatus> job_status_from_string(std::string_view value) {
    if (value == "pending") {
        return JobStatus::Pending;
    }
    if (value == "running") {
        return JobStatus::Running;
    }
    if (value == "done") {
        return JobStatus::Done;
    }
    if (value == "failed") {
        return JobStatus::Failed;
    }
    if (value == "cancelled") {
        return JobStatus::Cancelled;
    }

    return std::nullopt;
}

} // namespace taskforge::core
