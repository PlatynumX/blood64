#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/vendor/jfblood"
OUT="$ROOT/engine"

[[ -d "$SRC/.git" ]] || "$ROOT/scripts/fetch-engine.sh"
rm -rf "$OUT"
mkdir -p "$OUT/jfbuild/src" "$OUT/jfbuild/include" "$OUT/blood/src" "$OUT/blood/include"

# Bring in the real JFBuild core/header tree. We exclude platform frontends/renderers
# we are explicitly replacing on N64, not the engine itself.
cp -a "$SRC/jfbuild/include/." "$OUT/jfbuild/include/"
A_H="$(find "$SRC/jfbuild" -name a.h -print -quit)"
test -n "$A_H" || { echo "ERROR: JFBuild a.h not found" >&2; exit 1; }
cp "$A_H" "$OUT/jfbuild/include/a.h"
for f in a-c.c asmprot.c cache1d.c compat.c crc32.c defs.c engine.c kplib.c osd.c \
         pragmas.c scriptfile.c textfont.c smalltextfont.c mmulti_null.c version.c; do
    cp "$SRC/jfbuild/src/$f" "$OUT/jfbuild/src/$f"
done

# Bring in the actual Blood gameplay source + headers wholesale.
find "$SRC/src" -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.c' -o -name '*.h' \) \
    -exec cp {} "$OUT/blood/src/" \;

# Keep required auxiliary libraries' headers/source available while we replace their
# platform implementations incrementally.
mkdir -p "$OUT/jfmact" "$OUT/jfaudiolib"
cp -a "$SRC/jfmact/." "$OUT/jfmact/"
cp -a "$SRC/jfaudiolib/." "$OUT/jfaudiolib/"

cat > "$OUT/UPSTREAM.txt" <<EOF
JFBlood commit: $(git -C "$SRC" rev-parse HEAD)
JFBuild commit: $(git -C "$SRC/jfbuild" rev-parse HEAD)
EOF

echo "Actual JFBuild + Blood sources staged under engine/"
