#pragma once

#include "taskforge/storage/Database.hpp"
#include "taskforge/storage/FileRepository.hpp"

namespace taskforge::storage {

class SQLiteFileRepository final : public FileRepository {
public:
    explicit SQLiteFileRepository(Database& database);

    void insert(const core::FileRecord& file) override;
    std::optional<core::FileRecord> find_by_id(const core::FileId& id) const override;
    std::vector<core::FileRecord> list() const override;

private:
    Database& database_;
};

} // namespace taskforge::storage
