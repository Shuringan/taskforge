#include "taskforge/api/FileController.hpp"
#include "taskforge/api/HealthController.hpp"
#include "taskforge/api/HttpServer.hpp"
#include "taskforge/api/JobController.hpp"
#include "taskforge/api/MetricsController.hpp"
#include "taskforge/app/InMemoryFileService.hpp"
#include "taskforge/app/InMemoryJobService.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

std::filesystem::path temporary_file_path(const std::string& name) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
        / ("taskforge_http_api_" + name + "_" + std::to_string(stamp) + ".txt");
}

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path);
    output << content;
}

struct ApiFixture {
    taskforge::core::JobQueue queue;
    taskforge::app::InMemoryFileService files;
    taskforge::app::InMemoryJobService jobs{queue};
    taskforge::api::HealthController health{queue};
    taskforge::api::MetricsController metrics{queue, jobs};
    taskforge::api::FileController file_controller{files};
    taskforge::api::JobController job_controller{jobs, files};
    taskforge::api::HttpServer server{
        "127.0.0.1",
        8080,
        health,
        metrics,
        file_controller,
        job_controller
    };
};

std::string extract_id(const std::string& body, const std::string& field) {
    const std::string marker = "\"" + field + "\":\"";
    const auto start = body.find(marker);
    if (start == std::string::npos) {
        return "";
    }
    const auto value_start = start + marker.size();
    const auto value_end = body.find('"', value_start);
    return body.substr(value_start, value_end - value_start);
}

std::string register_file(ApiFixture& fixture, const std::filesystem::path& path) {
    const auto response = fixture.server.handle_request(
        "POST",
        "/files",
        "{\"path\":\"" + path.generic_string() + "\",\"original_name\":\"input.txt\"}"
    );
    return extract_id(response.body, "file_id");
}

void test_register_file() {
    ApiFixture fixture;
    const auto path = temporary_file_path("register");
    write_file(path, "hello");

    const auto response = fixture.server.handle_request(
        "POST",
        "/files",
        "{\"path\":\"" + path.generic_string() + "\",\"original_name\":\"input.txt\"}"
    );

    expect(response.status == 200, "POST /files returns 200");
    expect(response.body.find("\"file_id\"") != std::string::npos, "POST /files returns file_id");
    expect(response.body.find("\"size\":5") != std::string::npos, "POST /files returns size");

    std::filesystem::remove(path);
}

void test_get_file_by_id() {
    ApiFixture fixture;
    const auto path = temporary_file_path("get_file");
    write_file(path, "hello");
    const auto file_id = register_file(fixture, path);

    const auto response = fixture.server.handle_request("GET", "/files/" + file_id);

    expect(response.status == 200, "GET /files/{id} returns 200");
    expect(response.body.find(file_id) != std::string::npos, "GET /files/{id} returns file id");

    std::filesystem::remove(path);
}

void test_create_job() {
    ApiFixture fixture;
    const auto path = temporary_file_path("create_job");
    write_file(path, "hello");
    const auto file_id = register_file(fixture, path);

    const auto response = fixture.server.handle_request(
        "POST",
        "/jobs",
        "{\"type\":\"line_count\",\"file_id\":\"" + file_id + "\",\"priority\":0,\"max_attempts\":1}"
    );

    expect(response.status == 200, "POST /jobs returns 200");
    expect(response.body.find("\"job_id\"") != std::string::npos, "POST /jobs returns job_id");
    expect(response.body.find("\"status\":\"pending\"") != std::string::npos, "POST /jobs returns pending status");

    std::filesystem::remove(path);
}

void test_get_job_by_id() {
    ApiFixture fixture;
    const auto path = temporary_file_path("get_job");
    write_file(path, "hello");
    const auto file_id = register_file(fixture, path);
    const auto create_response = fixture.server.handle_request(
        "POST",
        "/jobs",
        "{\"type\":\"line_count\",\"file_id\":\"" + file_id + "\"}"
    );
    const auto job_id = extract_id(create_response.body, "job_id");

    const auto response = fixture.server.handle_request("GET", "/jobs/" + job_id);

    expect(response.status == 200, "GET /jobs/{id} returns 200");
    expect(response.body.find(job_id) != std::string::npos, "GET /jobs/{id} returns job id");

    std::filesystem::remove(path);
}

void test_list_jobs() {
    ApiFixture fixture;
    const auto path = temporary_file_path("list_jobs");
    write_file(path, "hello");
    const auto file_id = register_file(fixture, path);
    fixture.server.handle_request("POST", "/jobs", "{\"type\":\"line_count\",\"file_id\":\"" + file_id + "\"}");

    const auto response = fixture.server.handle_request("GET", "/jobs");

    expect(response.status == 200, "GET /jobs returns 200");
    expect(response.body.find("\"job_id\"") != std::string::npos, "GET /jobs returns jobs");

    std::filesystem::remove(path);
}

void test_get_result_for_not_completed_job_returns_409() {
    ApiFixture fixture;
    const auto path = temporary_file_path("result_pending");
    write_file(path, "hello");
    const auto file_id = register_file(fixture, path);
    const auto create_response = fixture.server.handle_request(
        "POST",
        "/jobs",
        "{\"type\":\"line_count\",\"file_id\":\"" + file_id + "\"}"
    );
    const auto job_id = extract_id(create_response.body, "job_id");

    const auto response = fixture.server.handle_request("GET", "/jobs/" + job_id + "/result");

    expect(response.status == 409, "pending job result returns 409");
    expect(response.body == "{\"error\":\"Job is not completed yet\"}", "pending job result returns JSON error");

    std::filesystem::remove(path);
}

void test_invalid_json_returns_400() {
    ApiFixture fixture;
    const auto response = fixture.server.handle_request("POST", "/files", "{not-json");

    expect(response.status == 400, "invalid JSON returns 400");
    expect(response.body.find("\"error\"") != std::string::npos, "invalid JSON returns JSON error");
}

void test_missing_job_returns_404() {
    ApiFixture fixture;
    const auto response = fixture.server.handle_request("GET", "/jobs/missing");

    expect(response.status == 404, "missing job returns 404");
    expect(response.body == "{\"error\":\"Job not found\"}", "missing job returns JSON error");
}

} // namespace

int run_http_file_job_api_tests() {
    test_register_file();
    test_get_file_by_id();
    test_create_job();
    test_get_job_by_id();
    test_list_jobs();
    test_get_result_for_not_completed_job_returns_409();
    test_invalid_json_returns_400();
    test_missing_job_returns_404();

    return failures == 0 ? 0 : 1;
}
