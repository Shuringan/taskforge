#include "taskforge/core/WorkerPool.hpp"

#include <stdexcept>

namespace taskforge::core {

WorkerPool::WorkerPool(
    std::size_t worker_count,
    JobQueue& queue,
    app::InMemoryJobService& job_service,
    executors::ExecutorRegistry& executors
)
    : worker_count_(worker_count),
      queue_(queue),
      job_service_(job_service),
      executors_(executors) {
    if (worker_count_ == 0) {
        throw std::invalid_argument("WorkerPool requires at least one worker");
    }
}

WorkerPool::~WorkerPool() {
    stop();
}

void WorkerPool::start() {
    std::lock_guard lock(mutex_);
    if (started_) {
        return;
    }

    workers_.reserve(worker_count_);
    for (std::size_t i = 0; i < worker_count_; ++i) {
        workers_.emplace_back([this] {
            worker_loop();
        });
    }

    started_ = true;
}

void WorkerPool::stop() {
    std::vector<std::thread> workers;
    {
        std::lock_guard lock(mutex_);
        if (!started_) {
            return;
        }

        queue_.shutdown();
        workers.swap(workers_);
        started_ = false;
    }

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void WorkerPool::worker_loop() {
    while (true) {
        const auto id = queue_.wait_and_pop();
        if (!id.has_value()) {
            break;
        }

        process_job(*id);
    }
}

void WorkerPool::process_job(const JobId& id) {
    const auto job = job_service_.get_job(id);
    if (!job.has_value()) {
        return;
    }

    const auto executor = executors_.find(job->type());
    if (!executor) {
        job_service_.store_error(id, "No executor registered for job type: " + job->type());
        job_service_.update_status(id, JobStatus::Failed);
        return;
    }

    job_service_.update_status(id, JobStatus::Running);
    const auto result = executor->execute(job->file_id());
    if (result.success) {
        job_service_.store_result(id, result.output);
        job_service_.update_status(id, JobStatus::Done);
        return;
    }

    job_service_.store_error(id, result.error);
    job_service_.update_status(id, JobStatus::Failed);
}

} // namespace taskforge::core
