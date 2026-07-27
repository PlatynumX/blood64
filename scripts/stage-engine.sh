#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/vendor/jfblood"
OUT="$ROOT/engine"
OVR="$ROOT/overrides"

# Exact source baseline used by Blood64. Do not mutate vendor source.
JFBLOOD_COMMIT="987f1a184e11f8c1e201368500c332a1cb4f0031"
JFBUILD_COMMIT="068a8cc2e21e3788a4c3325fbc18d2661474aa95"

[[ -d "$SRC/.git" ]] || "$ROOT/scripts/fetch-engine.sh"

# Always restore the vendor checkout to the pinned pristine source first.
git -C "$SRC" reset --hard "$JFBLOOD_COMMIT" >/dev/null
git -C "$SRC" clean -fd >/dev/null
git -C "$SRC" submodule sync --recursive
git -C "$SRC" submodule update --init --recursive --force

actual_jfblood="$(git -C "$SRC" rev-parse HEAD)"
actual_jfbuild="$(git -C "$SRC/jfbuild" rev-parse HEAD)"

[[ "$actual_jfblood" == "$JFBLOOD_COMMIT" ]] || {
    echo "ERROR: JFBlood SHA mismatch: $actual_jfblood" >&2
    exit 1
}
[[ "$actual_jfbuild" == "$JFBUILD_COMMIT" ]] || {
    echo "ERROR: JFBuild SHA mismatch: $actual_jfbuild" >&2
    exit 1
}

# The overrides directory is the entire Blood64 source delta. No patch scripts
# are run here and the pinned vendor checkout remains pristine.
required_overrides=(
    "jfbuild/include/compat.h"
    "jfbuild/src/compat.c"
    "jfbuild/src/defs.c"
    "jfbuild/src/kplib.c"
    "blood/src/db.h"
    "blood/src/db.cpp"
    "blood/src/actor.cpp"
)
for f in "${required_overrides[@]}"; do
    [[ -f "$OVR/$f" ]] || {
        echo "ERROR: missing consolidated override: overrides/$f" >&2
        exit 1
    }
done

rm -rf "$OUT"
mkdir -p "$OUT/jfbuild/src" "$OUT/jfbuild/include" "$OUT/blood/src" "$OUT/blood/include"

# Real JFBuild classic core and headers.
cp -a "$SRC/jfbuild/include/." "$OUT/jfbuild/include/"

# Private/generated JFBuild headers required by the selected core.
find "$SRC/jfbuild" -type f -name '*.h' \
    ! -name 'stdint.h' \
    ! -name 'inttypes.h' \
    -exec cp -f {} "$OUT/jfbuild/include/" \;

for f in a-c.c asmprot.c cache1d.c compat.c crc32.c defs.c engine.c kplib.c osd.c \
         pragmas.c scriptfile.c textfont.c smalltextfont.c mmulti_null.c version.c; do
    cp "$SRC/jfbuild/src/$f" "$OUT/jfbuild/src/$f"
done

# Real Blood gameplay layer.
find "$SRC/src" -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.c' -o -name '*.h' \) \
    -exec cp {} "$OUT/blood/src/" \;

# Auxiliary libraries retained while their platform backends are replaced.
mkdir -p "$OUT/jfmact" "$OUT/jfaudiolib"
cp -a "$SRC/jfmact/." "$OUT/jfmact/"
cp -a "$SRC/jfaudiolib/." "$OUT/jfaudiolib/"

# Apply the consolidated Blood64 delta only by direct file replacement.
cp "$OVR/jfbuild/include/compat.h" "$OUT/jfbuild/include/compat.h"
cp "$OVR/jfbuild/src/compat.c"       "$OUT/jfbuild/src/compat.c"
cp "$OVR/jfbuild/src/defs.c"         "$OUT/jfbuild/src/defs.c"
cp "$OVR/jfbuild/src/kplib.c"        "$OUT/jfbuild/src/kplib.c"
cp "$OVR/blood/src/db.h"             "$OUT/blood/src/db.h"
cp "$OVR/blood/src/db.cpp"           "$OUT/blood/src/db.cpp"
cp "$OVR/blood/src/actor.cpp"        "$OUT/blood/src/actor.cpp"

# Prove staging reproduced the override files byte-for-byte.
for f in "${required_overrides[@]}"; do
    cmp -s "$OVR/$f" "$OUT/$f" || {
        echo "ERROR: staged override differs: $f" >&2
        exit 1
    }
done

cat > "$OUT/UPSTREAM.txt" <<EOF
JFBlood commit: $actual_jfblood
JFBuild commit: $actual_jfbuild
Blood64 source model: pristine pinned upstream + consolidated overrides
EOF

echo "Blood64 engine staged from pristine pinned upstream + consolidated overrides."
