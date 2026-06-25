#include "taskforge/executors/WordCountExecutor.hpp"

#include <fstream>
#include <string>

namespace taskforge::executors {

std::string WordCountExecutor::type() const {
    return "word_count";
}

ExecutionResult WordCountExecutor::execute(const std::filesystem::path& file_path) {
    std::ifstream input(file_path);
    if (!input) {
        return ExecutionResult::fail("Failed to open file: " + file_path.string());
    }

    std::size_t words = 0;
    std::string word;
    while (input >> word) {
        ++words;
    }

    return ExecutionResult::ok("{\"words\":" + std::to_string(words) + "}");
}

} // namespace taskforge::executors
