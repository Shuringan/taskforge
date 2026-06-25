#pragma once

#include "taskforge/app/RegisterFileRequest.hpp"
#include "taskforge/core/FileId.hpp"
#include "taskforge/core/FileRecord.hpp"

#include <cstddef>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace taskforge::app {

class InMemoryFileService {
public:
    InMemoryFileService() = default;

    core::FileRecord register_file(const RegisterFileRequest& request);
    std::optional<core::FileRecord> get_file(const core::FileId& id) const;
    std::vector<core::FileRecord> list_files() const;

private:
    core::FileId next_file_id();

    mutable std::mutex mutex_;
    std::unordered_map<std::string, core::FileRecord> files_;
    std::size_t next_id_{1};
};

} // namespace taskforge::app
