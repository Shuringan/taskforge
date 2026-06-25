#pragma once

#include "taskforge/core/Job.hpp"

#include <optional>
#include <string>

namespace taskforge::app {

enum class RetryJobStatus {
    Retried,
    NotFound,
    NotFailed,
    AttemptsExhausted
};

struct RetryJobResult {
    RetryJobStatus status{RetryJobStatus::NotFound};
    std::optional<core::Job> job;
    std::string error;
};

} // namespace taskforge::app
