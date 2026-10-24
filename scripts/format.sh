#!/usr/bin/env bash
# Aplica o verifica clang-format sobre src/ y tests/ (--check no modifica nada).
set -euo pipefail

cd "$(dirname "$0")/.."

modo=aplicar
case "${1:-}" in
  "") ;;
  --check) modo=verificar ;;
  *) echo "[hubflow] uso: scripts/format.sh [--check]" >&2; exit 2 ;;
esac

if ! command -v clang-format >/dev/null 2>&1; then
  echo "[hubflow] clang-format no esta instalado: sudo apt install clang-format" >&2
  exit 127
fi

mapfile -t archivos < <(find src tests \( -name '*.cpp' -o -name '*.hpp' \) | sort)

if [ "$modo" = verificar ]; then
  echo "[hubflow] verificando formato de ${#archivos[@]} archivos"
  clang-format --dry-run --Werror "${archivos[@]}"
  echo "[hubflow] formato correcto"
else
  echo "[hubflow] formateando ${#archivos[@]} archivos"
  clang-format -i "${archivos[@]}"
fi
