#pragma once

#include "taskforge/core/Job.hpp"

#include <optional>
#include <string>

namespace taskforge::app {

enum class CancelJobStatus {
    Cancelled,
    NotFound,
    InvalidState
};

struct CancelJobResult {
    CancelJobStatus status{CancelJobStatus::NotFound};
    std::optional<core::Job> job;
    std::string error;
};

} // namespace taskforge::app
