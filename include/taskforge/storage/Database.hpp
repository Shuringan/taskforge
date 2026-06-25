#pragma once

#include <filesystem>
#include <string>

struct sqlite3;

namespace taskforge::storage {

class Database {
public:
    explicit Database(const std::filesystem::path& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* handle() const noexcept;
    void execute(const std::string& sql) const;

private:
    void initialize_schema();

    sqlite3* db_{nullptr};
};

} // namespace taskforge::storage
