#include "taskforge/storage/Database.hpp"

#include <sqlite3.h>

#include <stdexcept>

namespace taskforge::storage {

Database::Database(const std::filesystem::path& path) {
    if (sqlite3_open(path.string().c_str(), &db_) != SQLITE_OK) {
        const std::string message = db_ ? sqlite3_errmsg(db_) : "unknown SQLite error";
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
        throw std::runtime_error("Failed to open SQLite database: " + message);
    }

    execute("PRAGMA foreign_keys = ON;");
    initialize_schema();
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

sqlite3* Database::handle() const noexcept {
    return db_;
}

void Database::execute(const std::string& sql) const {
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error ? error : "unknown SQLite error";
        sqlite3_free(error);
        throw std::runtime_error("SQLite execute failed: " + message);
    }
}

void Database::initialize_schema() {
    execute(
        "CREATE TABLE IF NOT EXISTS files ("
        "id TEXT PRIMARY KEY,"
        "original_name TEXT NOT NULL,"
        "path TEXT NOT NULL,"
        "size INTEGER NOT NULL,"
        "uploaded_at TEXT NOT NULL"
        ");"
    );

    execute(
        "CREATE TABLE IF NOT EXISTS jobs ("
        "id TEXT PRIMARY KEY,"
        "type TEXT NOT NULL,"
        "file_id TEXT NOT NULL,"
        "status TEXT NOT NULL,"
        "priority INTEGER NOT NULL DEFAULT 0,"
        "attempts INTEGER NOT NULL DEFAULT 0,"
        "max_attempts INTEGER NOT NULL DEFAULT 1,"
        "result TEXT,"
        "error TEXT,"
        "created_at TEXT NOT NULL,"
        "started_at TEXT,"
        "finished_at TEXT,"
        "FOREIGN KEY(file_id) REFERENCES files(id)"
        ");"
    );
}

} // namespace taskforge::storage
