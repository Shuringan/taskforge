#pragma once

#include <filesystem>
#include <string>

namespace taskforge::app {

struct RegisterFileRequest {
    std::filesystem::path path;
    std::string original_name;
};

} // namespace taskforge::app
