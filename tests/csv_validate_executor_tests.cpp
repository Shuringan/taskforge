#include "taskforge/executors/CsvValidateExecutor.hpp"
#include "taskforge/executors/ExecutorRegistry.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

std::filesystem::path temporary_file_path(const std::string& name) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
        / ("taskforge_csv_" + name + "_" + std::to_string(stamp) + ".csv");
}

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path);
    output << content;
}

std::string expected_output(
    int total_rows,
    int valid_rows,
    int invalid_rows,
    int empty_fields,
    int duplicate_rows,
    int inconsistent_rows
) {
    return "{\"total_rows\":" + std::to_string(total_rows)
        + ",\"valid_rows\":" + std::to_string(valid_rows)
        + ",\"invalid_rows\":" + std::to_string(invalid_rows)
        + ",\"empty_fields\":" + std::to_string(empty_fields)
        + ",\"duplicate_rows\":" + std::to_string(duplicate_rows)
        + ",\"inconsistent_rows\":" + std::to_string(inconsistent_rows)
        + "}";
}

void test_valid_csv_file() {
    const auto path = temporary_file_path("valid");
    write_file(path, "name,age\nAda,36\nGrace,85\n");

    taskforge::executors::CsvValidateExecutor executor;
    const auto result = executor.execute(path);

    expect(result.success, "CsvValidateExecutor succeeds for valid CSV");
    expect(result.output == expected_output(3, 2, 0, 0, 0, 0),
           "valid CSV counts header and valid data rows");

    std::filesystem::remove(path);
}

void test_csv_with_empty_fields() {
    const auto path = temporary_file_path("empty_fields");
    write_file(path, "name,age\nAda,\nGrace,85\n");

    taskforge::executors::CsvValidateExecutor executor;
    const auto result = executor.execute(path);

    expect(result.success, "CsvValidateExecutor succeeds with empty fields");
    expect(result.output == expected_output(3, 1, 1, 1, 0, 0),
           "empty field makes data row invalid");

    std::filesystem::remove(path);
}

void test_csv_with_duplicate_rows() {
    const auto path = temporary_file_path("duplicates");
    write_file(path, "name,age\nAda,36\nAda,36\nGrace,85\n");

    taskforge::executors::CsvValidateExecutor executor;
    const auto result = executor.execute(path);

    expect(result.success, "CsvValidateExecutor succeeds with duplicate rows");
    expect(result.output == expected_output(4, 2, 1, 0, 1, 0),
           "duplicate data row is counted invalid");

    std::filesystem::remove(path);
}

void test_csv_with_inconsistent_column_count() {
    const auto path = temporary_file_path("inconsistent");
    write_file(path, "name,age\nAda,36,extra\nGrace,85\n");

    taskforge::executors::CsvValidateExecutor executor;
    const auto result = executor.execute(path);

    expect(result.success, "CsvValidateExecutor succeeds with inconsistent rows");
    expect(result.output == expected_output(3, 1, 1, 0, 0, 1),
           "inconsistent column count makes row invalid");

    std::filesystem::remove(path);
}

void test_missing_file_returns_failure() {
    const auto path = temporary_file_path("missing");

    taskforge::executors::CsvValidateExecutor executor;
    const auto result = executor.execute(path);

    expect(!result.success, "missing CSV returns failure");
    expect(result.output.empty(), "missing CSV failure has empty output");
    expect(!result.error.empty(), "missing CSV failure has error");
}

void test_empty_file_behavior() {
    const auto path = temporary_file_path("empty");
    write_file(path, "");

    taskforge::executors::CsvValidateExecutor executor;
    const auto result = executor.execute(path);

    expect(result.success, "empty CSV file succeeds");
    expect(result.output == expected_output(0, 0, 0, 0, 0, 0),
           "empty CSV file returns zero counts");

    std::filesystem::remove(path);
}

void test_executor_type_and_registry() {
    taskforge::executors::CsvValidateExecutor executor;
    taskforge::executors::ExecutorRegistry registry;
    registry.register_executor(std::make_shared<taskforge::executors::CsvValidateExecutor>());

    expect(executor.type() == "csv_validate", "CsvValidateExecutor type is stable");
    expect(registry.find("csv_validate") != nullptr, "CsvValidateExecutor can be registered");
}

} // namespace

int run_csv_validate_executor_tests() {
    test_valid_csv_file();
    test_csv_with_empty_fields();
    test_csv_with_duplicate_rows();
    test_csv_with_inconsistent_column_count();
    test_missing_file_returns_failure();
    test_empty_file_behavior();
    test_executor_type_and_registry();

    return failures == 0 ? 0 : 1;
}
