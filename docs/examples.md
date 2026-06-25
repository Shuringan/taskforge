# Examples

Start the server:

```bash
./build/taskforge --serve
```

On Windows:

```powershell
.\build\Debug\taskforge.exe --serve
```

## Register a File

```bash
curl -X POST http://127.0.0.1:8080/files \
  -H "Content-Type: application/json" \
  -d '{"path":"data/samples/simple.txt","original_name":"simple.txt"}'
```

Save the returned `file_id`.

## Create a line_count Job

```bash
curl -X POST http://127.0.0.1:8080/jobs \
  -H "Content-Type: application/json" \
  -d '{"type":"line_count","file_id":"file-1","priority":0,"max_attempts":1}'
```

## Get Job Status

```bash
curl http://127.0.0.1:8080/jobs/job-1
```

## Get Job Result

```bash
curl http://127.0.0.1:8080/jobs/job-1/result
```

The default HTTP server currently creates jobs but does not start a worker pool,
so newly created jobs remain `pending` unless a worker is wired by the caller.

## Create a csv_validate Job

Register a CSV file first:

```bash
curl -X POST http://127.0.0.1:8080/files \
  -H "Content-Type: application/json" \
  -d '{"path":"data/samples/products_valid.csv","original_name":"products_valid.csv"}'
```

Create the job:

```bash
curl -X POST http://127.0.0.1:8080/jobs \
  -H "Content-Type: application/json" \
  -d '{"type":"csv_validate","file_id":"file-1","priority":0,"max_attempts":2}'
```

## Retry a Failed Job

Retry works for jobs whose status is `failed` and whose attempts are below
`max_attempts`:

```bash
curl -X POST http://127.0.0.1:8080/jobs/job-1/retry
```

## Cancel a Pending Job

```bash
curl -X POST http://127.0.0.1:8080/jobs/job-1/cancel
```
