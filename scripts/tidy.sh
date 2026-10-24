#!/usr/bin/env bash
# Corre clang-tidy (run-clang-tidy) sobre src/ y tests/. Uso: scripts/tidy.sh [--fix] [archivos...]
set -euo pipefail

cd "$(dirname "$0")/.."

fix=()
if [ "${1:-}" = "--fix" ]; then
  fix=(-fix)
  shift
fi

for herramienta in clang-tidy run-clang-tidy; do
  if ! command -v "$herramienta" >/dev/null 2>&1; then
    echo "[hubflow] $herramienta no esta instalado: sudo apt install clang-tidy clang-tools" >&2
    exit 127
  fi
done

if [ ! -f build/debug/compile_commands.json ]; then
  echo "[hubflow] configurando preset debug para generar compile_commands.json"
  cmake --preset debug >/dev/null
fi

raiz=$(pwd)
if [ "$#" -gt 0 ]; then
  archivos=()
  for ruta in "$@"; do
    archivos+=("$(realpath "$ruta")")
  done
else
  archivos=("^${raiz}/(src|tests)/.*\\.cpp$")
fi

echo "[hubflow] clang-tidy con $(nproc) hilos"
run-clang-tidy -p build/debug -j "$(nproc)" -quiet \
  -header-filter "^${raiz}/(src|tests)/" \
  "${fix[@]}" "${archivos[@]}"
