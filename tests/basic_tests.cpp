#include <iostream>

int run_csv_validate_executor_tests();
int run_executor_tests();
int run_file_registry_tests();
int run_http_health_metrics_tests();
int run_in_memory_job_service_tests();
int run_job_model_tests();
int run_job_queue_tests();
int run_job_recovery_tests();
int run_sqlite_storage_tests();
int run_worker_pool_tests();

int main() {
    if (2 + 2 != 4) {
        std::cerr << "Sanity check failed\n";
        return 1;
    }

    const int csv_validate_executor_result = run_csv_validate_executor_tests();
    const int executor_result = run_executor_tests();
    const int file_registry_result = run_file_registry_tests();
    const int http_health_metrics_result = run_http_health_metrics_tests();
    const int in_memory_job_service_result = run_in_memory_job_service_tests();
    const int job_model_result = run_job_model_tests();
    const int job_queue_result = run_job_queue_tests();
    const int job_recovery_result = run_job_recovery_tests();
    const int sqlite_storage_result = run_sqlite_storage_tests();
    const int worker_pool_result = run_worker_pool_tests();

    return csv_validate_executor_result == 0
        && executor_result == 0
        && in_memory_job_service_result == 0
        && file_registry_result == 0
        && http_health_metrics_result == 0
        && job_model_result == 0
        && job_queue_result == 0
        && job_recovery_result == 0
        && sqlite_storage_result == 0
        && worker_pool_result == 0 ? 0 : 1;
}
