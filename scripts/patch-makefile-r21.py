#!/usr/bin/env python3
from pathlib import Path
p=Path("Makefile.engine")
s=p.read_text()
a="$(PROG_NAME).z64: toolchain-check endian-check engine-check"
b="$(PROG_NAME).z64: toolchain-check endian-check cxx-compat-check engine-check"
if b not in s:
    if a not in s: raise SystemExit("ROM prerequisite anchor missing")
    s=s.replace(a,b,1)
a=".PHONY: toolchain-check endian-check engine-check"
b=".PHONY: toolchain-check endian-check cxx-compat-check engine-check"
if b not in s:
    if a not in s: raise SystemExit("PHONY anchor missing")
    s=s.replace(a,b,1)
if "N64 C++ COMPAT PROBE PASSED" not in s:
    marker="engine-check:\n"
    i=s.find(marker)
    if i<0: raise SystemExit("engine-check anchor missing")
    block="""cxx-compat-check:
\t@mkdir -p $(BUILD_DIR)
\t@echo "Checking compat.h against libstdc++ min/max..."
\t@printf '%s\\n' \\
\t\t'#include "compat.h"' \\
\t\t'#include <limits>' \\
\t\t'#include <cmath>' \\
\t\t'int blood64_cxx_probe(void) {' \\
\t\t' int a=min(1,2); int b=max(1,2);' \\
\t\t' int c=std::numeric_limits<int>::min();' \\
\t\t' int d=std::numeric_limits<int>::max();' \\
\t\t' return a+b+(c==d); }' | \\
\t\t$(CXX) $(CXXFLAGS) -x c++ -c -o $(BUILD_DIR)/blood64-cxx-compat-probe.o -
\t@echo "N64 C++ COMPAT PROBE PASSED"

"""
    s=s[:i]+block+s[i:]
p.write_text(s)
