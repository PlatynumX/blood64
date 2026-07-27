#!/usr/bin/env python3
import re, sys
from pathlib import Path

p = Path(sys.argv[1] if len(sys.argv) > 1 else "engine-r3-build.log")
if not p.exists():
    print("No build log found.")
    raise SystemExit(0)

lines = p.read_text(errors="replace").splitlines()
errs = []
for line in lines:
    if re.search(r": (fatal )?error:", line) or "undefined reference to" in line:
        errs.append(line)

print("Blood64 r3 engine compile summary")
print("================================")
print(f"Compiler/link errors found: {len(errs)}")
for line in errs[:200]:
    print(line)
if len(errs) > 200:
    print(f"... {len(errs)-200} additional errors omitted")
