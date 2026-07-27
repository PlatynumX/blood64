#!/usr/bin/env python3
from pathlib import Path
import re, sys

p=Path(sys.argv[1])
s=p.read_text(encoding="latin-1")
if "using std::min;" in s and "using std::max;" in s:
    print("already patched")
    raise SystemExit(0)

pat=re.compile(
    r"#ifndef min\n# define min\(a, ?b\) .*?\n#endif\n"
    r"#ifndef max\n# define max\(a, ?b\) .*?\n#endif",
    re.S)
m=pat.search(s)
if not m:
    raise SystemExit("exact adjacent legacy min/max macros not found; refusing blind patch")

new="""#ifdef __cplusplus
# include <algorithm>
using std::min;
using std::max;
#else
#ifndef min
# define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
# define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#endif"""

p.write_text(s[:m.start()]+new+s[m.end():],encoding="latin-1")
print("backported C++ safe min/max handling")
