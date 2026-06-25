#include <iostream>

int run_executor_tests();
int run_in_memory_job_service_tests();
int run_job_model_tests();
int run_job_queue_tests();

int main() {
    if (2 + 2 != 4) {
        std::cerr << "Sanity check failed\n";
        return 1;
    }

    const int executor_result = run_executor_tests();
    const int in_memory_job_service_result = run_in_memory_job_service_tests();
    const int job_model_result = run_job_model_tests();
    const int job_queue_result = run_job_queue_tests();

    return executor_result == 0
        && in_memory_job_service_result == 0
        && job_model_result == 0
        && job_queue_result == 0 ? 0 : 1;
}
