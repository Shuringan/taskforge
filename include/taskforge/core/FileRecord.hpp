#pragma once

#include "taskforge/core/FileId.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>

namespace taskforge::core {

struct FileRecord {
    FileId id;
    std::string original_name;
    std::filesystem::path path;
    std::uintmax_t size{0};
    std::chrono::system_clock::time_point registered_at;
};

} // namespace taskforge::core
