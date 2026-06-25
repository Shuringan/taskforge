#include <iostream>

int run_job_model_tests();
int run_job_queue_tests();

int main() {
    if (2 + 2 != 4) {
        std::cerr << "Sanity check failed\n";
        return 1;
    }

    const int job_model_result = run_job_model_tests();
    const int job_queue_result = run_job_queue_tests();

    return job_model_result == 0 && job_queue_result == 0 ? 0 : 1;
}
