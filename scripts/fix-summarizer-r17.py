#!/usr/bin/env python3
from pathlib import Path

p = Path("scripts/summarize-engine-errors.py")
new = '''#!/usr/bin/env python3
import re
import sys
from pathlib import Path

path = Path(sys.argv[1] if len(sys.argv) > 1 else "engine-r16-build.log")
if not path.exists():
    print("No build log found.")
    raise SystemExit(0)

ansi = re.compile(r"\\x1b\\[[0-9;]*[A-Za-z]")
lines = [ansi.sub("", line) for line in path.read_text(errors="replace").splitlines()]

errors = []
for line in lines:
    if re.search(r": (?:fatal )?error:", line) or "undefined reference to" in line:
        errors.append(line)

print("Blood64 engine compile summary")
print("================================")
print(f"Compiler/link errors found: {len(errors)}")
for line in errors[:250]:
    print(line)
if len(errors) > 250:
    print(f"... {len(errors)-250} additional errors omitted")
'''
p.write_text(new)
