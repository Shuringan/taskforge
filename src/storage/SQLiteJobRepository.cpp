#include "taskforge/storage/SQLiteJobRepository.hpp"

#include <sqlite3.h>

#include <chrono>
#include <memory>
#include <optional>
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

std::optional<std::string> optional_text(sqlite3_stmt* statement, int column) {
    if (sqlite3_column_type(statement, column) == SQLITE_NULL) {
        return std::nullopt;
    }

    return std::string{reinterpret_cast<const char*>(sqlite3_column_text(statement, column))};
}

core::Job job_from_statement(sqlite3_stmt* statement) {
    const auto status = core::job_status_from_string(
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 3))
    );
    if (!status.has_value()) {
        throw std::runtime_error("Invalid job status stored in SQLite");
    }

    return core::Job{
        core::JobId{reinterpret_cast<const char*>(sqlite3_column_text(statement, 0))},
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)),
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)),
        *status,
        sqlite3_column_int(statement, 4),
        sqlite3_column_int(statement, 5),
        sqlite3_column_int(statement, 6),
        time_from_storage(sqlite3_column_text(statement, 9)),
        optional_text(statement, 7),
        optional_text(statement, 8)
    };
}

std::vector<core::Job> collect_jobs(sqlite3* db, sqlite3_stmt* statement) {
    std::vector<core::Job> jobs;
    while (true) {
        const int code = sqlite3_step(statement);
        if (code == SQLITE_ROW) {
            jobs.push_back(job_from_statement(statement));
            continue;
        }
        if (code == SQLITE_DONE) {
            break;
        }

        throw std::runtime_error("SQLite step failed: " + std::string(sqlite3_errmsg(db)));
    }

    return jobs;
}

} // namespace

SQLiteJobRepository::SQLiteJobRepository(Database& database)
    : database_(database) {
}

void SQLiteJobRepository::insert(const core::Job& job) {
    auto statement = prepare(
        database_.handle(),
        "INSERT INTO jobs (id, type, file_id, status, priority, attempts, max_attempts, result, error, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
    );
    const auto status = core::to_string(job.status());
    const auto created_at = time_to_storage(job.created_at());

    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 1, job.id().value().c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 2, job.type().c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 3, job.file_id().c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 4, status.c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_int(statement.get(), 5, job.priority()));
    require_ok(database_.handle(), sqlite3_bind_int(statement.get(), 6, job.attempts()));
    require_ok(database_.handle(), sqlite3_bind_int(statement.get(), 7, job.max_attempts()));
    if (job.result().has_value()) {
        require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 8, job.result()->c_str(), -1, SQLITE_TRANSIENT));
    }
    if (job.error().has_value()) {
        require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 9, job.error()->c_str(), -1, SQLITE_TRANSIENT));
    }
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 10, created_at.c_str(), -1, SQLITE_TRANSIENT));

    require_done(database_.handle(), sqlite3_step(statement.get()));
}

std::optional<core::Job> SQLiteJobRepository::find_by_id(const core::JobId& id) const {
    auto statement = prepare(
        database_.handle(),
        "SELECT id, type, file_id, status, priority, attempts, max_attempts, result, error, created_at "
        "FROM jobs WHERE id = ?;"
    );
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 1, id.value().c_str(), -1, SQLITE_TRANSIENT));

    const int code = sqlite3_step(statement.get());
    if (code == SQLITE_ROW) {
        return job_from_statement(statement.get());
    }
    if (code == SQLITE_DONE) {
        return std::nullopt;
    }

    throw std::runtime_error("SQLite step failed: " + std::string(sqlite3_errmsg(database_.handle())));
}

std::vector<core::Job> SQLiteJobRepository::list() const {
    auto statement = prepare(
        database_.handle(),
        "SELECT id, type, file_id, status, priority, attempts, max_attempts, result, error, created_at "
        "FROM jobs ORDER BY id;"
    );

    return collect_jobs(database_.handle(), statement.get());
}

std::vector<core::Job> SQLiteJobRepository::list_by_status(core::JobStatus status) const {
    auto statement = prepare(
        database_.handle(),
        "SELECT id, type, file_id, status, priority, attempts, max_attempts, result, error, created_at "
        "FROM jobs WHERE status = ? ORDER BY id;"
    );
    const auto status_text = core::to_string(status);
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 1, status_text.c_str(), -1, SQLITE_TRANSIENT));

    return collect_jobs(database_.handle(), statement.get());
}

bool SQLiteJobRepository::update_status(const core::JobId& id, core::JobStatus status) {
    auto statement = prepare(database_.handle(), "UPDATE jobs SET status = ? WHERE id = ?;");
    const auto status_text = core::to_string(status);
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 1, status_text.c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 2, id.value().c_str(), -1, SQLITE_TRANSIENT));
    require_done(database_.handle(), sqlite3_step(statement.get()));
    return sqlite3_changes(database_.handle()) > 0;
}

bool SQLiteJobRepository::update_result(const core::JobId& id, std::string result) {
    auto statement = prepare(database_.handle(), "UPDATE jobs SET result = ? WHERE id = ?;");
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 1, result.c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 2, id.value().c_str(), -1, SQLITE_TRANSIENT));
    require_done(database_.handle(), sqlite3_step(statement.get()));
    return sqlite3_changes(database_.handle()) > 0;
}

bool SQLiteJobRepository::update_error(const core::JobId& id, std::string error) {
    auto statement = prepare(database_.handle(), "UPDATE jobs SET error = ? WHERE id = ?;");
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 1, error.c_str(), -1, SQLITE_TRANSIENT));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 2, id.value().c_str(), -1, SQLITE_TRANSIENT));
    require_done(database_.handle(), sqlite3_step(statement.get()));
    return sqlite3_changes(database_.handle()) > 0;
}

bool SQLiteJobRepository::update_attempts(const core::JobId& id, int attempts) {
    if (attempts < 0) {
        throw std::invalid_argument("Job attempts must not be negative");
    }

    auto statement = prepare(database_.handle(), "UPDATE jobs SET attempts = ? WHERE id = ?;");
    require_ok(database_.handle(), sqlite3_bind_int(statement.get(), 1, attempts));
    require_ok(database_.handle(), sqlite3_bind_text(statement.get(), 2, id.value().c_str(), -1, SQLITE_TRANSIENT));
    require_done(database_.handle(), sqlite3_step(statement.get()));
    return sqlite3_changes(database_.handle()) > 0;
}

} // namespace taskforge::storage
