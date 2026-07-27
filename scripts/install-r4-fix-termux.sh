#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
cd ~/blood64-engine-r3

python3 scripts/patch-defs.py vendor/jfblood/jfbuild/src/defs.c

python3 - <<'PY'
from pathlib import Path
p = Path("scripts/stage-engine.sh")
s = p.read_text()
needle = 'rm -rf "$OUT"\n'
insert = 'python3 "$ROOT/scripts/patch-defs.py" "$SRC/jfbuild/src/defs.c"\n\n'
if insert not in s:
    if needle not in s:
        raise SystemExit("stage-engine.sh insertion point not found")
    s = s.replace(needle, insert + needle, 1)
    p.write_text(s)
PY

rm -rf engine
./scripts/stage-engine.sh

grep -q 'int loaddefinitionsfile(const char \*fn)' engine/jfbuild/src/defs.c
echo "defs.c repair verified"

git add scripts/patch-defs.py scripts/stage-engine.sh
git commit -m "Blood64 r4: repair truncated JFBuild defs.c"
git push
