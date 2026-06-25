#include "taskforge/executors/CsvValidateExecutor.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace taskforge::executors {
namespace {

struct CsvStats {
    std::size_t total_rows{0};
    std::size_t valid_rows{0};
    std::size_t invalid_rows{0};
    std::size_t empty_fields{0};
    std::size_t duplicate_rows{0};
    std::size_t inconsistent_rows{0};
};

std::string trim(std::string value) {
    const auto not_space = [](unsigned char ch) {
        return !std::isspace(ch);
    };

    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }

    if (!line.empty() && line.back() == ',') {
        fields.emplace_back("");
    }

    return fields;
}

std::string normalize_row(const std::vector<std::string>& fields) {
    std::string normalized;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) {
            normalized += ',';
        }
        normalized += fields[i];
    }

    return normalized;
}

std::string to_output(const CsvStats& stats) {
    return "{\"total_rows\":" + std::to_string(stats.total_rows)
        + ",\"valid_rows\":" + std::to_string(stats.valid_rows)
        + ",\"invalid_rows\":" + std::to_string(stats.invalid_rows)
        + ",\"empty_fields\":" + std::to_string(stats.empty_fields)
        + ",\"duplicate_rows\":" + std::to_string(stats.duplicate_rows)
        + ",\"inconsistent_rows\":" + std::to_string(stats.inconsistent_rows)
        + "}";
}

} // namespace

std::string CsvValidateExecutor::type() const {
    return "csv_validate";
}

ExecutionResult CsvValidateExecutor::execute(const std::filesystem::path& file_path) {
    std::ifstream input(file_path);
    if (!input) {
        return ExecutionResult::fail("Failed to open file: " + file_path.string());
    }

    CsvStats stats;
    std::optional<std::size_t> expected_columns;
    std::unordered_set<std::string> seen_data_rows;
    std::string line;

    while (std::getline(input, line)) {
        if (trim(line).empty()) {
            continue;
        }

        const auto fields = split_csv_line(line);
        ++stats.total_rows;

        if (!expected_columns.has_value()) {
            expected_columns = fields.size();
            continue;
        }

        bool invalid = false;
        if (fields.size() != *expected_columns) {
            ++stats.inconsistent_rows;
            invalid = true;
        }

        for (const auto& field : fields) {
            if (field.empty()) {
                ++stats.empty_fields;
                invalid = true;
            }
        }

        const auto normalized = normalize_row(fields);
        if (!invalid && seen_data_rows.contains(normalized)) {
            ++stats.duplicate_rows;
            invalid = true;
        }

        if (invalid) {
            ++stats.invalid_rows;
        } else {
            seen_data_rows.insert(normalized);
            ++stats.valid_rows;
        }
    }

    return ExecutionResult::ok(to_output(stats));
}

} // namespace taskforge::executors
