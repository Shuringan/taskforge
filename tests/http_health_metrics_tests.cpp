#include "taskforge/api/HealthController.hpp"
#include "taskforge/api/HttpServer.hpp"
#include "taskforge/api/MetricsController.hpp"
#include "taskforge/api/FileController.hpp"
#include "taskforge/api/JobController.hpp"
#include "taskforge/app/InMemoryFileService.hpp"
#include "taskforge/app/InMemoryJobService.hpp"

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

void test_health_returns_200() {
    ApiFixture fixture;
    const auto response = fixture.server.handle_request("GET", "/health");

    expect(response.status == 200, "/health returns 200");
    expect(response.body == "{\"status\":\"ok\",\"queue_size\":0}", "/health returns JSON body");
}

void test_metrics_returns_200() {
    ApiFixture fixture;
    const auto first = fixture.jobs.create_job({"line_count", "file-1"});
    const auto second = fixture.jobs.create_job({"word_count", "file-2"});
    fixture.jobs.update_status(first.id(), taskforge::core::JobStatus::Running);
    fixture.jobs.update_status(second.id(), taskforge::core::JobStatus::Done);

    const auto response = fixture.server.handle_request("GET", "/metrics");

    expect(response.status == 200, "/metrics returns 200");
    expect(response.body == "{\"queue_size\":2,\"jobs_total\":2,\"jobs_pending\":0,"
                            "\"jobs_running\":1,\"jobs_done\":1,\"jobs_failed\":0,"
                            "\"jobs_cancelled\":0}",
           "/metrics returns real queue and job counts");
}

void test_unknown_route_returns_404() {
    ApiFixture fixture;
    const auto response = fixture.server.handle_request("GET", "/unknown");

    expect(response.status == 404, "unknown route returns 404");
    expect(response.body == "{\"error\":\"Not found\"}", "unknown route returns JSON error");
}

} // namespace

int run_http_health_metrics_tests() {
    test_health_returns_200();
    test_metrics_returns_200();
    test_unknown_route_returns_404();

    return failures == 0 ? 0 : 1;
}
