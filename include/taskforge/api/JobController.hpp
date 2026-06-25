#pragma once

#include "taskforge/api/ApiResponse.hpp"
#include "taskforge/app/InMemoryFileService.hpp"
#include "taskforge/app/InMemoryJobService.hpp"

#include <string>
#include <string_view>

namespace taskforge::api {

class JobController {
public:
    JobController(app::InMemoryJobService& jobs, const app::InMemoryFileService& files);

    ApiResponse create_job(std::string_view body);
    ApiResponse list_jobs() const;
    ApiResponse get_job(const std::string& job_id) const;
    ApiResponse get_result(const std::string& job_id) const;

private:
    app::InMemoryJobService& jobs_;
    const app::InMemoryFileService& files_;
};

} // namespace taskforge::api
