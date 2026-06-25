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
    MetricsController& metrics
)
    : host_(std::move(host)),
      port_(port),
      health_(health),
      metrics_(metrics),
      server_(std::make_unique<httplib::Server>()) {
    server_->Get("/health", [this](const httplib::Request&, httplib::Response& response) {
        apply_response(health_.handle(), response);
    });

    server_->Get("/metrics", [this](const httplib::Request&, httplib::Response& response) {
        apply_response(metrics_.handle(), response);
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

ApiResponse HttpServer::handle_request(const std::string& method, const std::string& path) const {
    if (method == "GET" && path == "/health") {
        return health_.handle();
    }
    if (method == "GET" && path == "/metrics") {
        return metrics_.handle();
    }

    return ApiResponse{404, "application/json", "{\"error\":\"Not found\"}"};
}

} // namespace taskforge::api
