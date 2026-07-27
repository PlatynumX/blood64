#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="${1:-$ROOT/shareware-bootstrap/shareware}"
DST="$ROOT/filesystem/blood"

required=(BLOOD.RFF SOUNDS.RFF SHARE000.ART)
for f in "${required[@]}"; do
    if [[ ! -f "$SRC/$f" ]]; then
        echo "ERROR: missing $SRC/$f" >&2
        echo "Run ./shareware-bootstrap/scripts/fetch-shareware.sh first." >&2
        exit 1
    fi
done

rm -rf "$DST"
mkdir -p "$DST"

# r1 only embeds what its diagnostics actually consume.
for f in BLOOD.RFF SOUNDS.RFF SHARE000.ART; do
    cp -f "$SRC/$f" "$DST/$f"
done

# Stage these too if available because the next map/render milestone needs them.
for f in GUI.RFF SURFACE.DAT TABLES.DAT VOXEL.DAT BLOOD.INI; do
    [[ -f "$SRC/$f" ]] && cp -f "$SRC/$f" "$DST/$f"
done

(
  cd "$DST"
  find . -maxdepth 1 -type f -printf '%f\n' | LC_ALL=C sort
)

echo "Blood64 ROMFS staged: $DST"
