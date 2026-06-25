#pragma once

#include "taskforge/executors/JobExecutor.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace taskforge::executors {

class ExecutorRegistry {
public:
    void register_executor(std::shared_ptr<JobExecutor> executor);
    std::shared_ptr<JobExecutor> find(std::string_view type) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<JobExecutor>> executors_;
};

} // namespace taskforge::executors
