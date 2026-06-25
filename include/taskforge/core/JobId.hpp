#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace taskforge::core {

class JobId {
public:
    explicit JobId(std::string value)
        : value_(std::move(value)) {
        if (value_.empty()) {
            throw std::invalid_argument("JobId must not be empty");
        }
    }

    const std::string& value() const noexcept {
        return value_;
    }

    friend bool operator==(const JobId&, const JobId&) = default;

private:
    std::string value_;
};

} // namespace taskforge::core
