#!/usr/bin/env sh
set -eu

TYPE_VALUE="${1:-line_count}"
FILE_ID="${2:?file id is required}"

curl -X POST http://127.0.0.1:8080/jobs \
  -H "Content-Type: application/json" \
  -d "{\"type\":\"${TYPE_VALUE}\",\"file_id\":\"${FILE_ID}\",\"priority\":0,\"max_attempts\":1}"
