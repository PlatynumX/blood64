#!/usr/bin/env python3
from pathlib import Path

p = Path("Makefile.engine")
s = p.read_text()

if '$(PROG_NAME).z64: toolchain-check endian-check engine-check' not in s:
    old = '$(PROG_NAME).z64: toolchain-check engine-check'
    if old not in s:
        raise SystemExit("ROM prerequisite line not found")
    s = s.replace(old, '$(PROG_NAME).z64: toolchain-check endian-check engine-check', 1)

if '.PHONY: toolchain-check endian-check engine-check' not in s:
    old = '.PHONY: toolchain-check engine-check'
    if old not in s:
        raise SystemExit("phony line not found")
    s = s.replace(old, '.PHONY: toolchain-check endian-check engine-check', 1)

block = '''
endian-check:
\t@mkdir -p $(BUILD_DIR)
\t@echo "Checking actual N64 compiler endian macros..."
\t@printf '%s\\n' \\
\t\t'#if !defined(__BYTE_ORDER__) || !defined(__ORDER_BIG_ENDIAN__)' \\
\t\t'#error GCC endian macros missing' \\
\t\t'#endif' \\
\t\t'#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__' \\
\t\t'#error N64 compiler is not big-endian' \\
\t\t'#endif' \\
\t\t'int blood64_endian_probe(void) { return 0; }' | \\
\t\t$(CC) $(CFLAGS) -x c -c -o $(BUILD_DIR)/blood64-endian-probe.o -
\t@echo "N64 BIG-ENDIAN PROBE PASSED"
\t@echo "Relevant predefined macros:"
\t@printf '' | $(CC) $(CFLAGS) -dM -E -x c - | \\
\t\tgrep -E '^#define (__BYTE_ORDER__|__ORDER_(BIG|LITTLE)_ENDIAN__|__MIPSEB__|__MIPSEL__)' | sort

'''

if "N64 BIG-ENDIAN PROBE PASSED" not in s:
    marker = "engine-check:\n"
    idx = s.find(marker)
    if idx < 0:
        raise SystemExit("engine-check target not found")
    s = s[:idx] + block + s[idx:]

p.write_text(s)
