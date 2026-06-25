#pragma once

#include <string>

namespace taskforge::api {

struct ApiResponse {
    int status{200};
    std::string content_type{"application/json"};
    std::string body;
};

} // namespace taskforge::api
