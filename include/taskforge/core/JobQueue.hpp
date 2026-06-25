#pragma once

#include "taskforge/core/JobId.hpp"

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>

namespace taskforge::core {

class JobQueue {
public:
    JobQueue() = default;

    void push(JobId id);
    std::optional<JobId> wait_and_pop();
    void shutdown();

    bool empty() const;
    std::size_t size() const;
    bool is_shutdown() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<JobId> queue_;
    bool shutdown_{false};
};

} // namespace taskforge::core
