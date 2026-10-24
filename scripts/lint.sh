#!/usr/bin/env bash
# Corre format --check, clang-tidy y cppcheck; resume el resultado de cada uno.
set -uo pipefail

cd "$(dirname "$0")/.." || exit 1

nombres=()
estados=()

correr() {
  local nombre=$1
  shift
  echo "[hubflow] ==> $nombre"
  "$@"
  local codigo=$?
  nombres+=("$nombre")
  case $codigo in
    0) estados+=("OK") ;;
    127) estados+=("OMITIDO") ;;
    *) estados+=("FALLO") ;;
  esac
}

correr clang-format scripts/format.sh --check
correr clang-tidy scripts/tidy.sh
correr cppcheck scripts/cppcheck.sh

echo
echo "[hubflow] resumen"
printf '  %-14s %s\n' HERRAMIENTA RESULTADO
fallo=0
for i in "${!nombres[@]}"; do
  printf '  %-14s %s\n' "${nombres[$i]}" "${estados[$i]}"
  [ "${estados[$i]}" = FALLO ] && fallo=1
done
exit $fallo
