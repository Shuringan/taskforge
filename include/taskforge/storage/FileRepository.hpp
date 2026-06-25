#pragma once

#include "taskforge/core/FileId.hpp"
#include "taskforge/core/FileRecord.hpp"

#include <optional>
#include <vector>

namespace taskforge::storage {

class FileRepository {
public:
    virtual ~FileRepository() = default;

    virtual void insert(const core::FileRecord& file) = 0;
    virtual std::optional<core::FileRecord> find_by_id(const core::FileId& id) const = 0;
    virtual std::vector<core::FileRecord> list() const = 0;
};

} // namespace taskforge::storage
