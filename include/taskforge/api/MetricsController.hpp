#pragma once

#include "taskforge/api/ApiResponse.hpp"
#include "taskforge/app/InMemoryJobService.hpp"
#include "taskforge/core/JobQueue.hpp"

namespace taskforge::api {

class MetricsController {
public:
    MetricsController(const core::JobQueue& queue, const app::InMemoryJobService& jobs);

    ApiResponse handle() const;

private:
    const core::JobQueue& queue_;
    const app::InMemoryJobService& jobs_;
};

} // namespace taskforge::api
