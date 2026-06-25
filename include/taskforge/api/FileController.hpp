#pragma once

#include "taskforge/api/ApiResponse.hpp"
#include "taskforge/app/InMemoryFileService.hpp"

#include <string>
#include <string_view>

namespace taskforge::api {

class FileController {
public:
    explicit FileController(app::InMemoryFileService& files);

    ApiResponse register_file(std::string_view body);
    ApiResponse get_file(const std::string& file_id) const;

private:
    app::InMemoryFileService& files_;
};

} // namespace taskforge::api
