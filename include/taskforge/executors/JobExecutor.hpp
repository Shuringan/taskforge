#pragma once

#include "taskforge/executors/ExecutionResult.hpp"

#include <filesystem>
#include <string>

namespace taskforge::executors {

class JobExecutor {
public:
    virtual ~JobExecutor() = default;

    virtual std::string type() const = 0;
    virtual ExecutionResult execute(const std::filesystem::path& file_path) = 0;
};

} // namespace taskforge::executors
