#include "taskforge/app/InMemoryFileService.hpp"

#include <chrono>
#include <stdexcept>

namespace taskforge::app {

core::FileRecord InMemoryFileService::register_file(const RegisterFileRequest& request) {
    if (!std::filesystem::exists(request.path)) {
        throw std::invalid_argument("File path does not exist: " + request.path.string());
    }
    if (!std::filesystem::is_regular_file(request.path)) {
        throw std::invalid_argument("Path is not a regular file: " + request.path.string());
    }

    std::string original_name = request.original_name;
    if (original_name.empty()) {
        original_name = request.path.filename().string();
    }

    core::FileRecord record{
        next_file_id(),
        original_name,
        request.path,
        std::filesystem::file_size(request.path),
        std::chrono::system_clock::now()
    };

    {
        std::lock_guard lock(mutex_);
        files_.emplace(record.id.value(), record);
    }

    return record;
}

std::optional<core::FileRecord> InMemoryFileService::get_file(const core::FileId& id) const {
    std::lock_guard lock(mutex_);
    const auto it = files_.find(id.value());
    if (it == files_.end()) {
        return std::nullopt;
    }

    return it->second;
}

std::vector<core::FileRecord> InMemoryFileService::list_files() const {
    std::lock_guard lock(mutex_);
    std::vector<core::FileRecord> files;
    files.reserve(files_.size());

    for (const auto& [id, record] : files_) {
        (void)id;
        files.push_back(record);
    }

    return files;
}

core::FileId InMemoryFileService::next_file_id() {
    std::lock_guard lock(mutex_);
    return core::FileId{"file-" + std::to_string(next_id_++)};
}

} // namespace taskforge::app
