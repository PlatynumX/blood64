#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

./scripts/host-test.sh

if [[ ! -f shareware-bootstrap/shareware/BLOOD.RFF ]]; then
    ./shareware-bootstrap/scripts/fetch-shareware.sh
fi
./scripts/stage-shareware.sh

if command -v libdragon >/dev/null 2>&1; then
    exec libdragon make -j2
fi

if [[ -z "${N64_INST:-}" ]]; then
    echo "ERROR: neither libdragon CLI nor N64_INST is available." >&2
    echo "Install the current libdragon environment, then rerun." >&2
    exit 1
fi

exec make -j2
