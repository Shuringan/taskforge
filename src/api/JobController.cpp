#include "taskforge/api/JobController.hpp"

#include <nlohmann/json.hpp>

#include <exception>

namespace taskforge::api {
namespace {

ApiResponse json_error(int status, const std::string& message) {
    nlohmann::json body{{"error", message}};
    return ApiResponse{status, "application/json", body.dump()};
}

nlohmann::json job_to_json(const core::Job& job) {
    nlohmann::json body{
        {"job_id", job.id().value()},
        {"type", job.type()},
        {"file_id", job.file_id()},
        {"status", core::to_string(job.status())},
        {"priority", job.priority()},
        {"attempts", job.attempts()},
        {"max_attempts", job.max_attempts()}
    };

    if (job.result().has_value()) {
        body["result"] = *job.result();
    }
    if (job.error().has_value()) {
        body["error"] = *job.error();
    }

    return body;
}

} // namespace

JobController::JobController(app::InMemoryJobService& jobs, const app::InMemoryFileService& files)
    : jobs_(jobs),
      files_(files) {
}

ApiResponse JobController::create_job(std::string_view body) {
    try {
        const auto request = nlohmann::json::parse(body);
        const auto type = request.at("type").get<std::string>();
        const auto file_id = request.at("file_id").get<std::string>();
        const int priority = request.value("priority", 0);
        const int max_attempts = request.value("max_attempts", 1);

        if (type.empty()) {
            return json_error(400, "Job type must not be empty");
        }
        if (file_id.empty()) {
            return json_error(400, "File ID must not be empty");
        }
        if (max_attempts < 1) {
            return json_error(400, "max_attempts must be positive");
        }
        if (!files_.get_file(core::FileId{file_id}).has_value()) {
            return json_error(404, "File not found");
        }

        const auto job = jobs_.create_job({type, file_id, priority, max_attempts});
        nlohmann::json response{
            {"job_id", job.id().value()},
            {"status", core::to_string(job.status())}
        };
        return ApiResponse{200, "application/json", response.dump()};
    } catch (const nlohmann::json::exception&) {
        return json_error(400, "Invalid request body");
    } catch (const std::invalid_argument& error) {
        return json_error(400, error.what());
    } catch (const std::exception&) {
        return json_error(500, "Unexpected server error");
    }
}

ApiResponse JobController::list_jobs() const {
    nlohmann::json response = nlohmann::json::array();
    for (const auto& job : jobs_.list_jobs()) {
        response.push_back(job_to_json(job));
    }

    return ApiResponse{200, "application/json", response.dump()};
}

ApiResponse JobController::get_job(const std::string& job_id) const {
    try {
        const auto job = jobs_.get_job(core::JobId{job_id});
        if (!job.has_value()) {
            return json_error(404, "Job not found");
        }

        return ApiResponse{200, "application/json", job_to_json(*job).dump()};
    } catch (const std::invalid_argument&) {
        return json_error(404, "Job not found");
    }
}

ApiResponse JobController::get_result(const std::string& job_id) const {
    try {
        const auto job = jobs_.get_job(core::JobId{job_id});
        if (!job.has_value()) {
            return json_error(404, "Job not found");
        }

        if (job->status() == core::JobStatus::Failed) {
            return json_error(409, job->error().value_or("Job failed"));
        }
        if (job->status() != core::JobStatus::Done) {
            return json_error(409, "Job is not completed yet");
        }

        nlohmann::json response{{"result", job->result().value_or("")}};
        return ApiResponse{200, "application/json", response.dump()};
    } catch (const std::invalid_argument&) {
        return json_error(404, "Job not found");
    }
}

ApiResponse JobController::retry_job(const std::string& job_id) {
    try {
        const auto result = jobs_.retry_job(core::JobId{job_id});
        switch (result.status) {
        case app::RetryJobStatus::Retried: {
            nlohmann::json response{
                {"job_id", result.job->id().value()},
                {"status", core::to_string(result.job->status())},
                {"attempts", result.job->attempts()},
                {"max_attempts", result.job->max_attempts()}
            };
            return ApiResponse{200, "application/json", response.dump()};
        }
        case app::RetryJobStatus::NotFound:
            return json_error(404, "Job not found");
        case app::RetryJobStatus::NotFailed:
            return json_error(409, "Only failed jobs can be retried");
        case app::RetryJobStatus::AttemptsExhausted:
            return json_error(409, "Maximum retry attempts reached");
        }
    } catch (const std::invalid_argument&) {
        return json_error(404, "Job not found");
    }

    return json_error(500, "Unexpected server error");
}

} // namespace taskforge::api
