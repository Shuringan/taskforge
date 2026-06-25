#include <iostream>

int run_job_model_tests();

int main() {
    if (2 + 2 != 4) {
        std::cerr << "Sanity check failed\n";
        return 1;
    }

    return run_job_model_tests();
}
