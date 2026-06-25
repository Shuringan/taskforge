#pragma once

#include <string>

namespace taskforge::app {

struct CreateJobRequest {
    std::string type;
    std::string file_id;
    int priority{0};
    int max_attempts{1};
};

} // namespace taskforge::app
