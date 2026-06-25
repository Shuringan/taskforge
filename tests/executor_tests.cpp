#include "taskforge/executors/LineCountExecutor.hpp"
#include "taskforge/executors/WordCountExecutor.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
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
        / ("taskforge_" + name + "_" + std::to_string(stamp) + ".txt");
}

void write_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path);
    output << content;
}

void test_line_count_executor_counts_lines() {
    const auto path = temporary_file_path("line_count");
    write_file(path, "one\ntwo\nthree\n");

    taskforge::executors::LineCountExecutor executor;
    const auto result = executor.execute(path);

    expect(result.success, "LineCountExecutor succeeds for readable file");
    expect(result.output == "{\"lines\":3}", "LineCountExecutor counts lines");
    expect(result.error.empty(), "LineCountExecutor success has empty error");

    std::filesystem::remove(path);
}

void test_line_count_executor_fails_for_missing_file() {
    const auto path = temporary_file_path("missing_line_count");

    taskforge::executors::LineCountExecutor executor;
    const auto result = executor.execute(path);

    expect(!result.success, "LineCountExecutor fails for missing file");
    expect(result.output.empty(), "LineCountExecutor failure has empty output");
    expect(!result.error.empty(), "LineCountExecutor failure has error");
}

void test_word_count_executor_counts_words() {
    const auto path = temporary_file_path("word_count");
    write_file(path, "one two\nthree\tfour five");

    taskforge::executors::WordCountExecutor executor;
    const auto result = executor.execute(path);

    expect(result.success, "WordCountExecutor succeeds for readable file");
    expect(result.output == "{\"words\":5}", "WordCountExecutor counts words");
    expect(result.error.empty(), "WordCountExecutor success has empty error");

    std::filesystem::remove(path);
}

void test_word_count_executor_fails_for_missing_file() {
    const auto path = temporary_file_path("missing_word_count");

    taskforge::executors::WordCountExecutor executor;
    const auto result = executor.execute(path);

    expect(!result.success, "WordCountExecutor fails for missing file");
    expect(result.output.empty(), "WordCountExecutor failure has empty output");
    expect(!result.error.empty(), "WordCountExecutor failure has error");
}

void test_executor_type_values() {
    taskforge::executors::LineCountExecutor line_count;
    taskforge::executors::WordCountExecutor word_count;

    expect(line_count.type() == "line_count", "LineCountExecutor type is stable");
    expect(word_count.type() == "word_count", "WordCountExecutor type is stable");
}

} // namespace

int run_executor_tests() {
    test_line_count_executor_counts_lines();
    test_line_count_executor_fails_for_missing_file();
    test_word_count_executor_counts_words();
    test_word_count_executor_fails_for_missing_file();
    test_executor_type_values();

    return failures == 0 ? 0 : 1;
}
