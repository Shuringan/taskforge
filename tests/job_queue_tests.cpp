#include "taskforge/core/JobQueue.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

void test_new_queue_is_empty() {
    const taskforge::core::JobQueue queue;

    expect(queue.empty(), "new queue is empty");
    expect(queue.size() == 0, "new queue has zero size");
    expect(!queue.is_shutdown(), "new queue is not shut down");
}

void test_push_increases_size() {
    taskforge::core::JobQueue queue;
    queue.push(taskforge::core::JobId{"job-1"});

    expect(!queue.empty(), "push makes queue non-empty");
    expect(queue.size() == 1, "push increases size");
}

void test_wait_and_pop_returns_pushed_job() {
    taskforge::core::JobQueue queue;
    queue.push(taskforge::core::JobId{"job-1"});

    const auto popped = queue.wait_and_pop();

    expect(popped.has_value(), "wait_and_pop returns a job");
    expect(popped->value() == "job-1", "wait_and_pop returns pushed job");
    expect(queue.empty(), "wait_and_pop removes job");
}

void test_fifo_behavior() {
    taskforge::core::JobQueue queue;
    queue.push(taskforge::core::JobId{"job-1"});
    queue.push(taskforge::core::JobId{"job-2"});

    const auto first = queue.wait_and_pop();
    const auto second = queue.wait_and_pop();

    expect(first.has_value() && first->value() == "job-1", "first pushed job pops first");
    expect(second.has_value() && second->value() == "job-2", "second pushed job pops second");
}

void test_shutdown_marks_queue() {
    taskforge::core::JobQueue queue;
    queue.shutdown();

    expect(queue.is_shutdown(), "shutdown marks queue as shut down");
}

void test_wait_and_pop_after_shutdown_empty_returns_nullopt() {
    taskforge::core::JobQueue queue;
    queue.shutdown();

    const auto popped = queue.wait_and_pop();

    expect(!popped.has_value(), "shutdown empty queue returns nullopt");
}

void test_waiting_consumer_wakes_after_shutdown() {
    taskforge::core::JobQueue queue;
    std::atomic<bool> waiting{false};
    std::atomic<bool> returned{false};

    std::thread consumer([&] {
        waiting = true;
        const auto popped = queue.wait_and_pop();
        returned = !popped.has_value();
    });

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (!waiting && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    queue.shutdown();
    consumer.join();

    expect(returned, "waiting consumer wakes and returns nullopt after shutdown");
}

void test_push_after_shutdown_throws() {
    taskforge::core::JobQueue queue;
    queue.shutdown();

    bool thrown = false;
    try {
        queue.push(taskforge::core::JobId{"job-1"});
    } catch (const std::runtime_error&) {
        thrown = true;
    }

    expect(thrown, "push after shutdown throws runtime_error");
}

void test_multi_producer_multi_consumer() {
    constexpr int producer_count = 4;
    constexpr int jobs_per_producer = 25;
    constexpr int total_jobs = producer_count * jobs_per_producer;

    taskforge::core::JobQueue queue;
    std::atomic<int> consumed{0};
    std::vector<std::thread> consumers;
    std::vector<std::thread> producers;

    for (int i = 0; i < 4; ++i) {
        consumers.emplace_back([&] {
            while (true) {
                const auto job = queue.wait_and_pop();
                if (!job.has_value()) {
                    break;
                }
                ++consumed;
            }
        });
    }

    for (int producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([&, producer] {
            for (int job = 0; job < jobs_per_producer; ++job) {
                queue.push(taskforge::core::JobId{
                    "job-" + std::to_string(producer) + "-" + std::to_string(job)
                });
            }
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (consumed.load() < total_jobs && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    queue.shutdown();

    for (auto& consumer : consumers) {
        consumer.join();
    }

    expect(consumed.load() == total_jobs, "multi-producer multi-consumer consumes all jobs");
    expect(queue.empty(), "queue is empty after multi-threaded consumption");
}

} // namespace

int run_job_queue_tests() {
    test_new_queue_is_empty();
    test_push_increases_size();
    test_wait_and_pop_returns_pushed_job();
    test_fifo_behavior();
    test_shutdown_marks_queue();
    test_wait_and_pop_after_shutdown_empty_returns_nullopt();
    test_waiting_consumer_wakes_after_shutdown();
    test_push_after_shutdown_throws();
    test_multi_producer_multi_consumer();

    return failures == 0 ? 0 : 1;
}
