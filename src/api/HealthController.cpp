#include "taskforge/api/HealthController.hpp"

namespace taskforge::api {

HealthController::HealthController(const core::JobQueue& queue)
    : queue_(queue) {
}

ApiResponse HealthController::handle() const {
    return ApiResponse{
        200,
        "application/json",
        "{\"status\":\"ok\",\"queue_size\":" + std::to_string(queue_.size()) + "}"
    };
}

} // namespace taskforge::api
