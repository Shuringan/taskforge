#include "taskforge/api/FileController.hpp"
#include "taskforge/api/HealthController.hpp"
#include "taskforge/api/HttpServer.hpp"
#include "taskforge/api/JobController.hpp"
#include "taskforge/api/MetricsController.hpp"
#include "taskforge/app/InMemoryFileService.hpp"
#include "taskforge/app/InMemoryJobService.hpp"
#include "taskforge/core/JobQueue.hpp"

#include <iostream>
#include <cstdlib>
#include <string_view>

int main(int argc, char** argv) {
    if (argc > 1 && std::string_view{argv[1]} == "--serve") {
        taskforge::core::JobQueue queue;
        taskforge::app::InMemoryFileService files;
        taskforge::app::InMemoryJobService jobs{queue};
        taskforge::api::HealthController health{queue};
        taskforge::api::MetricsController metrics{queue, jobs};
        taskforge::api::FileController file_controller{files};
        taskforge::api::JobController job_controller{jobs, files};
        const char* host_env = std::getenv("TASKFORGE_HOST");
        const std::string_view host = host_env ? std::string_view{host_env} : "127.0.0.1";

        taskforge::api::HttpServer server{
            std::string{host},
            8080,
            health,
            metrics,
            file_controller,
            job_controller
        };

        std::cout << "TaskForge listening on http://" << host << ":8080\n";
        return server.listen() ? 0 : 1;
    }

    std::cout << "TaskForge bootstrap\n";
    return 0;
}
