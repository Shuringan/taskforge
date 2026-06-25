#pragma once

#include "taskforge/core/JobQueue.hpp"
#include "taskforge/storage/JobRepository.hpp"

#include <cstddef>

namespace taskforge::app {

struct RecoverySummary {
    std::size_t requeued_pending_jobs{0};
    std::size_t failed_running_jobs{0};
};

class JobRecoveryService {
public:
    JobRecoveryService(storage::JobRepository& jobs, core::JobQueue& queue);

    RecoverySummary recover();

private:
    storage::JobRepository& jobs_;
    core::JobQueue& queue_;
};

} // namespace taskforge::app
