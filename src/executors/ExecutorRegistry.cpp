#include "taskforge/executors/ExecutorRegistry.hpp"

#include <stdexcept>
#include <utility>

namespace taskforge::executors {

void ExecutorRegistry::register_executor(std::shared_ptr<JobExecutor> executor) {
    if (!executor) {
        throw std::invalid_argument("Executor must not be null");
    }

    const std::string type = executor->type();
    if (type.empty()) {
        throw std::invalid_argument("Executor type must not be empty");
    }

    std::lock_guard lock(mutex_);
    executors_[type] = std::move(executor);
}

std::shared_ptr<JobExecutor> ExecutorRegistry::find(std::string_view type) const {
    std::lock_guard lock(mutex_);
    const auto it = executors_.find(std::string{type});
    if (it == executors_.end()) {
        return nullptr;
    }

    return it->second;
}

} // namespace taskforge::executors
