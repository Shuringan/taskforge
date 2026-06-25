#include "taskforge/storage/SQLiteFileRepository.hpp"

#include <sqlite3.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

namespace taskforge::storage {
namespace {

using StatementPtr = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

StatementPtr prepare(sqlite3* db, const char* sql) {
    sqlite3_stmt* raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &raw, nullptr) != SQLITE_OK) {
        throw std::runtime_error("SQLite prepare failed: " + std::string(sqlite3_errmsg(db)));
    }

    return StatementPtr(raw, sqlite3_finalize);
}

void require_ok(sqlite3* db, int code) {
    if (code != SQLITE_OK) {
        throw std::runtime_error("SQLite bind failed: " + std::string(sqlite3_errmsg(db)));
    }
}

void require_done(sqlite3* db, int code) {
    if (code != SQLITE_DONE) {
        throw std::runtime_error("SQLite step failed: " + std::string(sqlite3_errmsg(db)));
    }
}

std::string time_to_storage(std::chrono::system_clock::time_point value) {
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
        value.time_since_epoch()
    ).count();
    return std::to_string(millis);
}

std::chrono::system_clock::time_point time_from_storage(const unsigned char* value) {
    const auto millis = std::stoll(reinterpret_cast<const char*>(value));
    return std::chrono::system_clock::time_point{std::chrono::milliseconds{millis}};
}

core::FileRecord file_from_statement(sqlite3_stmt* statement) {
    return core::FileRecord{
        core::FileId{reinterpret_cast<const char*>(sqlite3_column_text(statement, 0))},
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)),
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)),
        static_cast<std::uintmax_t>(sqlite3_column_int64(statement, 3)),
        time_from_storage(sqlite3_column_text(statement, 4))
    };
}

} // namespace

SQLiteFileRepository::SQLiteFileRepository(Database& database)
    : database_(database) {
}

void SQLiteFileRepository::insert(const core::FileRecord& file) {
    auto statement = prepare(
        database_.handle(),
        "INSERT INTO files (id, original_name, path, size, uploaded_at) VALUES (?, ?, ?, ?, ?);"
    );
    const auto uploaded_at = time_to_storage(file.registered_at);

    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 1, file.id.value().c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 2, file.original_name.c_str(), -1, SQLITE_TRANSIENT));
    const auto path = file.path.string();
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 3, path.c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_int64(statement.get(), 4, static_cast<sqlite3_int64>(file.size)));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 5, uploaded_at.c_str(), -1, SQLITE_TRANSIENT));

    require_done(database_.handle(), sqlite3_step(statement.get()));
}

std::optional<core::FileRecord> SQLiteFileRepository::find_by_id(const core::FileId& id) const {
    auto statement = prepare(
        database_.handle(),
        "SELECT id, original_name, path, size, uploaded_at FROM files WHERE id = ?;"
    );
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 1, id.value().c_str(), -1, SQLITE_TRANSIENT));

    const int code = sqlite3_step(statement.get());
    if (code == SQLITE_ROW) {
        return file_from_statement(statement.get());
    }
    if (code == SQLITE_DONE) {
        return std::nullopt;
    }

    throw std::runtime_error("SQLite step failed: " + std::string(sqlite3_errmsg(database_.handle())));
}

std::vector<core::FileRecord> SQLiteFileRepository::list() const {
    auto statement = prepare(
        database_.handle(),
        "SELECT id, original_name, path, size, uploaded_at FROM files ORDER BY id;"
    );

    std::vector<core::FileRecord> files;
    while (true) {
        const int code = sqlite3_step(statement.get());
        if (code == SQLITE_ROW) {
            files.push_back(file_from_statement(statement.get()));
            continue;
        }
        if (code == SQLITE_DONE) {
            break;
        }

        throw std::runtime_error("SQLite step failed: " + std::string(sqlite3_errmsg(database_.handle())));
    }

    return files;
}

} // namespace taskforge::storage
