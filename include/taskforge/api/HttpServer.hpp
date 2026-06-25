#pragma once

#include "taskforge/api/ApiResponse.hpp"
#include "taskforge/api/HealthController.hpp"
#include "taskforge/api/MetricsController.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace httplib {
class Server;
}

namespace taskforge::api {

class HttpServer {
public:
    HttpServer(
        std::string host,
        std::uint16_t port,
        HealthController& health,
        MetricsController& metrics
    );
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    bool listen();
    void stop();
    ApiResponse handle_request(const std::string& method, const std::string& path) const;

private:
    std::string host_;
    std::uint16_t port_;
    HealthController& health_;
    MetricsController& metrics_;
    std::unique_ptr<httplib::Server> server_;
};

} // namespace taskforge::api
