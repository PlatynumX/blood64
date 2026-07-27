#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

bash -n scripts/*.sh shareware-bootstrap/scripts/*.sh
./scripts/host-test.sh

test -f Makefile
test -f .github/workflows/build.yml
test -f src/main.c
test -f src/blood64_rff.c
test -f src/blood64_art.c
grep -q 'RFF\\x1a' src/blood64_rff.c
grep -q 'SHARE000.ART' src/main.c

echo "Blood64 source-package verification: PASS"
