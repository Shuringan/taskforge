#include "taskforge/api/FileController.hpp"

#include <nlohmann/json.hpp>

#include <exception>

namespace taskforge::api {
namespace {

ApiResponse json_error(int status, const std::string& message) {
    nlohmann::json body{{"error", message}};
    return ApiResponse{status, "application/json", body.dump()};
}

nlohmann::json file_to_json(const core::FileRecord& file) {
    return nlohmann::json{
        {"file_id", file.id.value()},
        {"original_name", file.original_name},
        {"path", file.path.string()},
        {"size", file.size}
    };
}

} // namespace

FileController::FileController(app::InMemoryFileService& files)
    : files_(files) {
}

ApiResponse FileController::register_file(std::string_view body) {
    try {
        const auto request = nlohmann::json::parse(body);
        const auto path = request.at("path").get<std::string>();
        const auto original_name = request.value("original_name", std::string{});

        const auto record = files_.register_file({path, original_name});
        return ApiResponse{200, "application/json", file_to_json(record).dump()};
    } catch (const nlohmann::json::exception&) {
        return json_error(400, "Invalid request body");
    } catch (const std::invalid_argument& error) {
        return json_error(400, error.what());
    } catch (const std::exception&) {
        return json_error(500, "Unexpected server error");
    }
}

ApiResponse FileController::get_file(const std::string& file_id) const {
    try {
        const auto file = files_.get_file(core::FileId{file_id});
        if (!file.has_value()) {
            return json_error(404, "File not found");
        }

        return ApiResponse{200, "application/json", file_to_json(*file).dump()};
    } catch (const std::invalid_argument&) {
        return json_error(404, "File not found");
    }
}

} // namespace taskforge::api
