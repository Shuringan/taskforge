#include "taskforge/executors/LineCountExecutor.hpp"

#include <fstream>
#include <string>

namespace taskforge::executors {

std::string LineCountExecutor::type() const {
    return "line_count";
}

ExecutionResult LineCountExecutor::execute(const std::filesystem::path& file_path) {
    std::ifstream input(file_path);
    if (!input) {
        return ExecutionResult::fail("Failed to open file: " + file_path.string());
    }

    std::size_t lines = 0;
    std::string line;
    while (std::getline(input, line)) {
        ++lines;
    }

    return ExecutionResult::ok("{\"lines\":" + std::to_string(lines) + "}");
}

} // namespace taskforge::executors
