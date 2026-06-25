#pragma once

#include "taskforge/storage/Database.hpp"
#include "taskforge/storage/JobRepository.hpp"

namespace taskforge::storage {

class SQLiteJobRepository final : public JobRepository {
public:
    explicit SQLiteJobRepository(Database& database);

    void insert(const core::Job& job) override;
    std::optional<core::Job> find_by_id(const core::JobId& id) const override;
    std::vector<core::Job> list() const override;
    std::vector<core::Job> list_by_status(core::JobStatus status) const override;
    bool update_status(const core::JobId& id, core::JobStatus status) override;
    bool update_result(const core::JobId& id, std::string result) override;
    bool update_error(const core::JobId& id, std::string error) override;
    bool update_attempts(const core::JobId& id, int attempts) override;

private:
    Database& database_;
};

} // namespace taskforge::storage
