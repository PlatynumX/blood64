#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
mkdir -p build
cc -std=c11 -Wall -Wextra -Werror -Iinclude \
   src/blood64_rff.c src/blood64_art.c tests/host_tests.c \
   -o build/host_tests
./build/host_tests
