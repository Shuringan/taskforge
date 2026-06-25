# API

All error responses are JSON:

```json
{"error":"Message"}
```

## GET /health

Purpose: service health check.

Success:

```json
{"status":"ok","queue_size":0}
```

Example:

```bash
curl http://127.0.0.1:8080/health
```

## GET /metrics

Purpose: basic queue and job counters.

Success:

```json
{"queue_size":0,"jobs_total":0,"jobs_pending":0,"jobs_running":0,"jobs_done":0,"jobs_failed":0,"jobs_cancelled":0}
```

Example:

```bash
curl http://127.0.0.1:8080/metrics
```

## POST /files

Purpose: register an existing local file path.

Request:

```json
{"path":"data/samples/simple.txt","original_name":"simple.txt"}
```

Success:

```json
{"file_id":"file-1","original_name":"simple.txt","path":"data/samples/simple.txt","size":42}
```

Errors:

- `400` invalid JSON or invalid path.

Example:

```bash
curl -X POST http://127.0.0.1:8080/files \
  -H "Content-Type: application/json" \
  -d '{"path":"data/samples/simple.txt","original_name":"simple.txt"}'
```

## GET /files/{file_id}

Purpose: fetch registered file metadata.

Success:

```json
{"file_id":"file-1","original_name":"simple.txt","path":"data/samples/simple.txt","size":42}
```

Errors:

- `404` file not found.

Example:

```bash
curl http://127.0.0.1:8080/files/file-1
```

## POST /jobs

Purpose: create a job and enqueue it.

Request:

```json
{"type":"line_count","file_id":"file-1","priority":0,"max_attempts":1}
```

Success:

```json
{"job_id":"job-1","status":"pending"}
```

Errors:

- `400` invalid JSON or invalid job request.
- `404` file not found.

Example:

```bash
curl -X POST http://127.0.0.1:8080/jobs \
  -H "Content-Type: application/json" \
  -d '{"type":"line_count","file_id":"file-1","priority":0,"max_attempts":1}'
```

## GET /jobs

Purpose: list jobs.

Success:

```json
[
  {"job_id":"job-1","type":"line_count","file_id":"file-1","status":"pending","priority":0,"attempts":0,"max_attempts":1}
]
```

Example:

```bash
curl http://127.0.0.1:8080/jobs
```

## GET /jobs/{job_id}

Purpose: fetch job metadata.

Errors:

- `404` job not found.

Example:

```bash
curl http://127.0.0.1:8080/jobs/job-1
```

## GET /jobs/{job_id}/result

Purpose: fetch completed job result.

If the job is not done:

```json
{"error":"Job is not completed yet"}
```

Errors:

- `404` job not found.
- `409` job is not completed or failed.

Example:

```bash
curl http://127.0.0.1:8080/jobs/job-1/result
```

## POST /jobs/{job_id}/retry

Purpose: retry a failed job when attempts remain.

Success:

```json
{"job_id":"job-1","status":"pending","attempts":1,"max_attempts":3}
```

Errors:

- `404` job not found.
- `409` job is not failed or max attempts were reached.

Example:

```bash
curl -X POST http://127.0.0.1:8080/jobs/job-1/retry
```

## POST /jobs/{job_id}/cancel

Purpose: cancel a pending or running job.

Success:

```json
{"job_id":"job-1","status":"cancelled"}
```

Errors:

- `404` job not found.
- `409` job is already completed, failed, or cancelled.

Example:

```bash
curl -X POST http://127.0.0.1:8080/jobs/job-1/cancel
```
