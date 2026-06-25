#pragma once

#include "taskforge/app/InMemoryJobService.hpp"
#include "taskforge/core/JobQueue.hpp"
#include "taskforge/executors/ExecutorRegistry.hpp"

#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

namespace taskforge::core {

class WorkerPool {
public:
    WorkerPool(
        std::size_t worker_count,
        JobQueue& queue,
        app::InMemoryJobService& job_service,
        executors::ExecutorRegistry& executors
    );

    WorkerPool(const WorkerPool&) = delete;
    WorkerPool& operator=(const WorkerPool&) = delete;

    ~WorkerPool();

    void start();
    void stop();

private:
    void worker_loop();
    void process_job(const JobId& id);

    std::size_t worker_count_;
    JobQueue& queue_;
    app::InMemoryJobService& job_service_;
    executors::ExecutorRegistry& executors_;
    std::mutex mutex_;
    bool started_{false};
    std::vector<std::thread> workers_;
};

} // namespace taskforge::core
