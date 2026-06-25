#include "taskforge/app/InMemoryFileService.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

std::filesystem::path temporary_path(const std::string& name) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
        / ("taskforge_file_registry_" + name + "_" + std::to_string(stamp));
}

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path);
    output << content;
}

void test_register_existing_file() {
    const auto path = temporary_path("existing.txt");
    write_file(path, "hello");

    taskforge::app::InMemoryFileService service;
    const auto record = service.register_file({path, "input.txt"});

    expect(!record.id.value().empty(), "registered file has id");
    expect(record.original_name == "input.txt", "registered file stores original name");
    expect(record.path == path, "registered file stores path");

    std::filesystem::remove(path);
}

void test_metadata_includes_size() {
    const auto path = temporary_path("size.txt");
    write_file(path, "hello");

    taskforge::app::InMemoryFileService service;
    const auto record = service.register_file({path, "size.txt"});

    expect(record.size == 5, "metadata includes correct file size");

    std::filesystem::remove(path);
}

void test_original_name_defaults_to_filename() {
    const auto path = temporary_path("default_name.txt");
    write_file(path, "data");

    taskforge::app::InMemoryFileService service;
    const auto record = service.register_file({path, ""});

    expect(record.original_name == path.filename().string(),
           "empty original_name defaults to filename");

    std::filesystem::remove(path);
}

void test_get_registered_file() {
    const auto path = temporary_path("get.txt");
    write_file(path, "data");

    taskforge::app::InMemoryFileService service;
    const auto record = service.register_file({path, "get.txt"});
    const auto stored = service.get_file(record.id);

    expect(stored.has_value(), "registered file can be retrieved");
    expect(stored.has_value() && stored->id == record.id, "retrieved file id matches");

    std::filesystem::remove(path);
}

void test_get_missing_file_returns_empty() {
    taskforge::app::InMemoryFileService service;
    const auto missing = service.get_file(taskforge::core::FileId{"missing"});

    expect(!missing.has_value(), "missing file returns empty optional");
}

void test_register_missing_path_fails() {
    const auto path = temporary_path("missing.txt");
    taskforge::app::InMemoryFileService service;

    bool thrown = false;
    try {
        service.register_file({path, ""});
    } catch (const std::invalid_argument&) {
        thrown = true;
    }

    expect(thrown, "registering missing path fails");
}

void test_register_directory_path_fails() {
    const auto path = temporary_path("directory");
    std::filesystem::create_directory(path);
    taskforge::app::InMemoryFileService service;

    bool thrown = false;
    try {
        service.register_file({path, ""});
    } catch (const std::invalid_argument&) {
        thrown = true;
    }

    expect(thrown, "registering directory path fails");

    std::filesystem::remove(path);
}

} // namespace

int run_file_registry_tests() {
    test_register_existing_file();
    test_metadata_includes_size();
    test_original_name_defaults_to_filename();
    test_get_registered_file();
    test_get_missing_file_returns_empty();
    test_register_missing_path_fails();
    test_register_directory_path_fails();

    return failures == 0 ? 0 : 1;
}
