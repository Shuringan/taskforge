#pragma once

#include "taskforge/core/Job.hpp"
#include "taskforge/core/JobId.hpp"
#include "taskforge/core/JobStatus.hpp"

#include <optional>
#include <string>
#include <vector>

namespace taskforge::storage {

class JobRepository {
public:
    virtual ~JobRepository() = default;

    virtual void insert(const core::Job& job) = 0;
    virtual std::optional<core::Job> find_by_id(const core::JobId& id) const = 0;
    virtual std::vector<core::Job> list() const = 0;
    virtual std::vector<core::Job> list_by_status(core::JobStatus status) const = 0;
    virtual bool update_status(const core::JobId& id, core::JobStatus status) = 0;
    virtual bool update_result(const core::JobId& id, std::string result) = 0;
    virtual bool update_error(const core::JobId& id, std::string error) = 0;
    virtual bool update_attempts(const core::JobId& id, int attempts) = 0;
};

} // namespace taskforge::storage
