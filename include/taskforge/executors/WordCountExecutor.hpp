#pragma once

#include "taskforge/executors/JobExecutor.hpp"

namespace taskforge::executors {

class WordCountExecutor final : public JobExecutor {
public:
    std::string type() const override;
    ExecutionResult execute(const std::filesystem::path& file_path) override;
};

} // namespace taskforge::executors
