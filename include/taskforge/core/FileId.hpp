#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace taskforge::core {

class FileId {
public:
    explicit FileId(std::string value)
        : value_(std::move(value)) {
        if (value_.empty()) {
            throw std::invalid_argument("FileId must not be empty");
        }
    }

    const std::string& value() const noexcept {
        return value_;
    }

    friend bool operator==(const FileId&, const FileId&) = default;

private:
    std::string value_;
};

} // namespace taskforge::core
