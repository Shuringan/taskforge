#include "taskforge/core/JobQueue.hpp"

#include <stdexcept>
#include <utility>

namespace taskforge::core {

void JobQueue::push(JobId id) {
    {
        std::lock_guard lock(mutex_);
        if (shutdown_) {
            throw std::runtime_error("Cannot push to a shut down JobQueue");
        }

        queue_.push(std::move(id));
    }

    condition_.notify_one();
}

std::optional<JobId> JobQueue::wait_and_pop() {
    std::unique_lock lock(mutex_);
    condition_.wait(lock, [this] {
        return shutdown_ || !queue_.empty();
    });

    if (queue_.empty()) {
        return std::nullopt;
    }

    JobId id = std::move(queue_.front());
    queue_.pop();
    return id;
}

void JobQueue::shutdown() {
    {
        std::lock_guard lock(mutex_);
        shutdown_ = true;
    }

    condition_.notify_all();
}

bool JobQueue::empty() const {
    std::lock_guard lock(mutex_);
    return queue_.empty();
}

std::size_t JobQueue::size() const {
    std::lock_guard lock(mutex_);
    return queue_.size();
}

bool JobQueue::is_shutdown() const {
    std::lock_guard lock(mutex_);
    return shutdown_;
}

} // namespace taskforge::core
