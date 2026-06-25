#include "taskforge/api/HttpServer.hpp"

#include <httplib.h>

namespace taskforge::api {

namespace {

void apply_response(const ApiResponse& source, httplib::Response& target) {
    target.status = source.status;
    target.set_content(source.body, source.content_type);
}

} // namespace

HttpServer::HttpServer(
    std::string host,
    std::uint16_t port,
    HealthController& health,
    MetricsController& metrics,
    FileController& files,
    JobController& jobs
)
    : host_(std::move(host)),
      port_(port),
      health_(health),
      metrics_(metrics),
      files_(files),
      jobs_(jobs),
      server_(std::make_unique<httplib::Server>()) {
    server_->Get("/health", [this](const httplib::Request&, httplib::Response& response) {
        apply_response(health_.handle(), response);
    });

    server_->Get("/metrics", [this](const httplib::Request&, httplib::Response& response) {
        apply_response(metrics_.handle(), response);
    });

    server_->Post("/files", [this](const httplib::Request& request, httplib::Response& response) {
        apply_response(files_.register_file(request.body), response);
    });

    server_->Get(R"(/files/([^/]+))", [this](const httplib::Request& request, httplib::Response& response) {
        apply_response(files_.get_file(request.matches[1]), response);
    });

    server_->Post("/jobs", [this](const httplib::Request& request, httplib::Response& response) {
        apply_response(jobs_.create_job(request.body), response);
    });

    server_->Post(R"(/jobs/([^/]+)/retry)", [this](const httplib::Request& request, httplib::Response& response) {
        (void)request;
        apply_response(jobs_.retry_job(request.matches[1]), response);
    });

    server_->Get("/jobs", [this](const httplib::Request&, httplib::Response& response) {
        apply_response(jobs_.list_jobs(), response);
    });

    server_->Get(R"(/jobs/([^/]+)/result)", [this](const httplib::Request& request, httplib::Response& response) {
        apply_response(jobs_.get_result(request.matches[1]), response);
    });

    server_->Get(R"(/jobs/([^/]+))", [this](const httplib::Request& request, httplib::Response& response) {
        apply_response(jobs_.get_job(request.matches[1]), response);
    });

    server_->Get(R"(/.*)", [](const httplib::Request&, httplib::Response& response) {
        response.status = 404;
        response.set_content("{\"error\":\"Not found\"}", "application/json");
    });

    server_->set_error_handler([](const httplib::Request&, httplib::Response& response) {
        if (response.status == 404) {
            response.set_content("{\"error\":\"Not found\"}", "application/json");
        }
    });
}

HttpServer::~HttpServer() = default;

bool HttpServer::listen() {
    return server_->listen(host_, port_);
}

void HttpServer::stop() {
    server_->stop();
}

ApiResponse HttpServer::handle_request(
    const std::string& method,
    const std::string& path,
    std::string_view body
) const {
    if (method == "GET" && path == "/health") {
        return health_.handle();
    }
    if (method == "GET" && path == "/metrics") {
        return metrics_.handle();
    }
    if (method == "POST" && path == "/files") {
        return files_.register_file(body);
    }
    if (method == "POST" && path == "/jobs") {
        return jobs_.create_job(body);
    }
    if (method == "POST" && path.rfind("/jobs/", 0) == 0) {
        const std::string prefix = "/jobs/";
        const std::string suffix = "/retry";
        const auto tail = path.substr(prefix.size());
        if (tail.size() > suffix.size()
            && tail.compare(tail.size() - suffix.size(), suffix.size(), suffix) == 0) {
            return jobs_.retry_job(tail.substr(0, tail.size() - suffix.size()));
        }
    }
    if (method == "GET" && path == "/jobs") {
        return jobs_.list_jobs();
    }
    if (method == "GET" && path.rfind("/files/", 0) == 0) {
        return files_.get_file(path.substr(std::string{"/files/"}.size()));
    }
    if (method == "GET" && path.rfind("/jobs/", 0) == 0) {
        const std::string prefix = "/jobs/";
        const std::string suffix = "/result";
        const auto tail = path.substr(prefix.size());
        if (tail.size() > suffix.size()
            && tail.compare(tail.size() - suffix.size(), suffix.size(), suffix) == 0) {
            return jobs_.get_result(tail.substr(0, tail.size() - suffix.size()));
        }

        return jobs_.get_job(tail);
    }

    return ApiResponse{404, "application/json", "{\"error\":\"Not found\"}"};
}

} // namespace taskforge::api
