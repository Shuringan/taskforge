#!/usr/bin/env sh
set -eu

JOB_ID="${1:?job id is required}"

curl "http://127.0.0.1:8080/jobs/${JOB_ID}/result"
