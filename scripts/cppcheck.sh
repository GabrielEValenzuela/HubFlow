#!/usr/bin/env bash
# Corre cppcheck sobre src/ y tests/ con cache incremental en build/cppcheck.
set -euo pipefail

cd "$(dirname "$0")/.."

if ! command -v cppcheck >/dev/null 2>&1; then
  echo "[hubflow] cppcheck no esta instalado: sudo apt install cppcheck" >&2
  exit 127
fi

mkdir -p build/cppcheck

echo "[hubflow] cppcheck sobre src/ y tests/"
cppcheck \
  --enable=warning,style,performance,portability \
  --std=c++17 \
  --inline-suppr \
  --error-exitcode=1 \
  --suppress=missingIncludeSystem \
  --cppcheck-build-dir=build/cppcheck \
  -j "$(nproc)" \
  -i build \
  --quiet \
  -I src \
  src tests
