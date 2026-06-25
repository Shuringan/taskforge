#include "taskforge/storage/Database.hpp"
#include "taskforge/storage/SQLiteFileRepository.hpp"
#include "taskforge/storage/SQLiteJobRepository.hpp"

#include <chrono>
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

taskforge::core::FileRecord make_file(const std::string& id = "file-1") {
    return taskforge::core::FileRecord{
        taskforge::core::FileId{id},
        "input.csv",
        "C:/tmp/input.csv",
        42,
        std::chrono::system_clock::now()
    };
}

taskforge::core::Job make_job(const std::string& id = "job-1") {
    return taskforge::core::Job{
        taskforge::core::JobId{id},
        "line_count",
        "file-1",
        5,
        3
    };
}

void insert_file(taskforge::storage::SQLiteFileRepository& files) {
    files.insert(make_file());
}

void test_database_initializes_schema() {
    taskforge::storage::Database database{":memory:"};

    bool succeeded = true;
    try {
        database.execute(
            "INSERT INTO files (id, original_name, path, size, uploaded_at) "
            "VALUES ('schema-file', 'schema.txt', 'schema.txt', 1, '0');"
        );
    } catch (const std::runtime_error&) {
        succeeded = false;
    }

    expect(succeeded, "database initializes files schema");
}

void test_insert_find_file() {
    taskforge::storage::Database database{":memory:"};
    taskforge::storage::SQLiteFileRepository files{database};

    const auto file = make_file();
    files.insert(file);
    const auto stored = files.find_by_id(file.id);

    expect(stored.has_value(), "inserted file can be found");
    expect(stored.has_value() && stored->id == file.id, "stored file id matches");
    expect(stored.has_value() && stored->original_name == "input.csv", "stored file name matches");
    expect(stored.has_value() && stored->size == 42, "stored file size matches");
}

void test_insert_find_job() {
    taskforge::storage::Database database{":memory:"};
    taskforge::storage::SQLiteFileRepository files{database};
    taskforge::storage::SQLiteJobRepository jobs{database};

    insert_file(files);
    const auto job = make_job();
    jobs.insert(job);
    const auto stored = jobs.find_by_id(job.id());

    expect(stored.has_value(), "inserted job can be found");
    expect(stored.has_value() && stored->id() == job.id(), "stored job id matches");
    expect(stored.has_value() && stored->type() == "line_count", "stored job type matches");
    expect(stored.has_value() && stored->file_id() == "file-1", "stored job file id matches");
}

void test_update_job_status() {
    taskforge::storage::Database database{":memory:"};
    taskforge::storage::SQLiteFileRepository files{database};
    taskforge::storage::SQLiteJobRepository jobs{database};

    insert_file(files);
    const auto job = make_job();
    jobs.insert(job);
    const bool updated = jobs.update_status(job.id(), taskforge::core::JobStatus::Done);
    const auto stored = jobs.find_by_id(job.id());

    expect(updated, "update_status returns true");
    expect(stored.has_value() && stored->status() == taskforge::core::JobStatus::Done,
           "job status persists");
}

void test_update_result_and_error() {
    taskforge::storage::Database database{":memory:"};
    taskforge::storage::SQLiteFileRepository files{database};
    taskforge::storage::SQLiteJobRepository jobs{database};

    insert_file(files);
    const auto job = make_job();
    jobs.insert(job);
    const bool result_updated = jobs.update_result(job.id(), "{\"lines\":3}");
    const bool error_updated = jobs.update_error(job.id(), "failed");
    const auto stored = jobs.find_by_id(job.id());

    expect(result_updated, "update_result returns true");
    expect(error_updated, "update_error returns true");
    expect(stored.has_value() && stored->result().has_value() && *stored->result() == "{\"lines\":3}",
           "job result persists");
    expect(stored.has_value() && stored->error().has_value() && *stored->error() == "failed",
           "job error persists");
}

void test_list_jobs() {
    taskforge::storage::Database database{":memory:"};
    taskforge::storage::SQLiteFileRepository files{database};
    taskforge::storage::SQLiteJobRepository jobs{database};

    insert_file(files);
    jobs.insert(make_job("job-1"));
    jobs.insert(make_job("job-2"));

    const auto all_jobs = jobs.list();

    expect(all_jobs.size() == 2, "list returns all jobs");
}

void test_list_jobs_by_status() {
    taskforge::storage::Database database{":memory:"};
    taskforge::storage::SQLiteFileRepository files{database};
    taskforge::storage::SQLiteJobRepository jobs{database};

    insert_file(files);
    const auto pending = make_job("job-1");
    const auto running = make_job("job-2");
    jobs.insert(pending);
    jobs.insert(running);
    jobs.update_status(running.id(), taskforge::core::JobStatus::Running);

    const auto running_jobs = jobs.list_by_status(taskforge::core::JobStatus::Running);

    expect(running_jobs.size() == 1, "list_by_status returns matching jobs");
    expect(running_jobs.size() == 1 && running_jobs.front().id() == running.id(),
           "list_by_status returns expected job");
}

void test_attempts_persist() {
    taskforge::storage::Database database{":memory:"};
    taskforge::storage::SQLiteFileRepository files{database};
    taskforge::storage::SQLiteJobRepository jobs{database};

    insert_file(files);
    const auto job = make_job();
    jobs.insert(job);
    const bool updated = jobs.update_attempts(job.id(), 2);
    const auto stored = jobs.find_by_id(job.id());

    expect(updated, "update_attempts returns true");
    expect(stored.has_value() && stored->attempts() == 2, "attempts persist correctly");
}

} // namespace

int run_sqlite_storage_tests() {
    test_database_initializes_schema();
    test_insert_find_file();
    test_insert_find_job();
    test_update_job_status();
    test_update_result_and_error();
    test_list_jobs();
    test_list_jobs_by_status();
    test_attempts_persist();

    return failures == 0 ? 0 : 1;
}
