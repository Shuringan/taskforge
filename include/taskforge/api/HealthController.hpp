#pragma once

#include "taskforge/api/ApiResponse.hpp"
#include "taskforge/core/JobQueue.hpp"

namespace taskforge::api {

class HealthController {
public:
    explicit HealthController(const core::JobQueue& queue);

    ApiResponse handle() const;

private:
    const core::JobQueue& queue_;
};

} // namespace taskforge::api
