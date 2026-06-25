#pragma once

#include <string>
#include <utility>

namespace taskforge::executors {

struct ExecutionResult {
    bool success{false};
    std::string output;
    std::string error;

    static ExecutionResult ok(std::string output) {
        return ExecutionResult{true, std::move(output), ""};
    }

    static ExecutionResult fail(std::string error) {
        return ExecutionResult{false, "", std::move(error)};
    }
};

} // namespace taskforge::executors
