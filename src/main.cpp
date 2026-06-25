#include "taskforge/api/HealthController.hpp"
#include "taskforge/api/HttpServer.hpp"
#include "taskforge/api/MetricsController.hpp"
#include "taskforge/app/InMemoryJobService.hpp"
#include "taskforge/core/JobQueue.hpp"

#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
    if (argc > 1 && std::string_view{argv[1]} == "--serve") {
        taskforge::core::JobQueue queue;
        taskforge::app::InMemoryJobService jobs{queue};
        taskforge::api::HealthController health{queue};
        taskforge::api::MetricsController metrics{queue, jobs};
        taskforge::api::HttpServer server{"127.0.0.1", 8080, health, metrics};

        std::cout << "TaskForge listening on http://127.0.0.1:8080\n";
        return server.listen() ? 0 : 1;
    }

    std::cout << "TaskForge bootstrap\n";
    return 0;
}
