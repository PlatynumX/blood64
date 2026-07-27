#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$ROOT/.shareware-tmp"
OUT="$ROOT/shareware"
URL="https://www.dosgames.com/files/DOSBOX_BLOOD.ZIP"
ZIP="$TMP/DOSBOX_BLOOD.ZIP"

need() {
  command -v "$1" >/dev/null 2>&1 || { echo "ERROR: missing required command: $1" >&2; exit 1; }
}
need curl
need unzip
need find
need sha256sum

rm -rf "$TMP"
mkdir -p "$TMP/unpacked" "$OUT"

echo "Downloading installed Blood shareware package..."
curl -L --fail --retry 3 --connect-timeout 20 -o "$ZIP" "$URL"

echo "Archive SHA-256:"
sha256sum "$ZIP" | tee "$TMP/archive.sha256"

echo "Extracting..."
unzip -q "$ZIP" -d "$TMP/unpacked"

# Locate the installed game directory by finding BLOOD.RFF.
BLOOD_RFF="$(find "$TMP/unpacked" -type f -iname 'BLOOD.RFF' -print -quit)"
if [[ -z "$BLOOD_RFF" ]]; then
  echo "ERROR: extracted package does not contain BLOOD.RFF" >&2
  exit 1
fi
GAMEDIR="$(dirname "$BLOOD_RFF")"

echo "Installed shareware data found at: $GAMEDIR"
rm -rf "$OUT"/*

# Files needed by the Blood engine/resource layer. Shareware uses SHARE000.ART
# instead of the registered TILES000.ART series.
patterns=(
  'BLOOD.INI' 'BLOOD.RFF' 'GUI.RFF' 'SOUNDS.RFF'
  'SURFACE.DAT' 'TABLES.DAT' 'VOXEL.DAT' 'SHARE000.ART'
  'BLOOD*.DEM' '*.SMK' '*.MID'
)

copy_matches() {
  local pat="$1"
  while IFS= read -r -d '' f; do
    cp -f "$f" "$OUT/$(basename "$f")"
  done < <(find "$GAMEDIR" -maxdepth 1 -type f -iname "$pat" -print0)
}
for pat in "${patterns[@]}"; do copy_matches "$pat"; done

required=(BLOOD.INI BLOOD.RFF SOUNDS.RFF SURFACE.DAT TABLES.DAT SHARE000.ART)
for f in "${required[@]}"; do
  if [[ ! -f "$OUT/$f" && ! -f "$OUT/${f,,}" ]]; then
    # Case-insensitive check independent of source filename case.
    if ! find "$OUT" -maxdepth 1 -type f -iname "$f" -print -quit | grep -q .; then
      echo "ERROR: required shareware file missing after extraction: $f" >&2
      exit 1
    fi
  fi
done

# Normalize core filenames to uppercase for deterministic ROM builds.
for wanted in BLOOD.INI BLOOD.RFF GUI.RFF SOUNDS.RFF SURFACE.DAT TABLES.DAT VOXEL.DAT SHARE000.ART; do
  src="$(find "$OUT" -maxdepth 1 -type f -iname "$wanted" -print -quit || true)"
  if [[ -n "$src" && "$(basename "$src")" != "$wanted" ]]; then
    mv -f "$src" "$OUT/$wanted"
  fi
done

(
  cd "$OUT"
  find . -maxdepth 1 -type f -printf '%f\n' | LC_ALL=C sort > FILES.txt
  sha256sum $(find . -maxdepth 1 -type f ! -name SHA256SUMS.txt ! -name FILES.txt -printf '%f\n' | LC_ALL=C sort) > SHA256SUMS.txt
)

cat > "$OUT/SHAREWARE-NOTES.txt" <<'NOTES'
Blood64 test data: Blood v1.11 shareware episode.

Important compatibility details:
- Shareware uses SHARE000.ART rather than the registered TILES000.ART series.
- The shareware BLOOD.RFF/SOUNDS.RFF and maps differ from registered Blood.
- Blood64 should support these formats directly; do NOT fake TILES000.ART by
  copying/renaming SHARE000.ART as an implementation assumption.
- Keep this directory as immutable reference data. Build-time converted or packed
  assets should go into a separate generated directory.
NOTES

echo
echo "Shareware data ready in: $OUT"
echo "Files:"
cat "$OUT/FILES.txt"
