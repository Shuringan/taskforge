#!/usr/bin/env sh
set -eu

PATH_VALUE="${1:-data/samples/simple.txt}"
NAME_VALUE="${2:-simple.txt}"

curl -X POST http://127.0.0.1:8080/files \
  -H "Content-Type: application/json" \
  -d "{\"path\":\"${PATH_VALUE}\",\"original_name\":\"${NAME_VALUE}\"}"
