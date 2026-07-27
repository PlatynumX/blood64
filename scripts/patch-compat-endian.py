#!/usr/bin/env python3
from pathlib import Path
import sys

def patch(path: Path) -> None:
    s = path.read_text()
    anchor = '''#elif defined VITA
# define B_LITTLE_ENDIAN 1
# define B_BIG_ENDIAN    0
# define B_SWAP64(x) __bswap64(x)
# define B_SWAP32(x) __bswap32(x)
# define B_SWAP16(x) __bswap16(x)
#endif

#if !defined(B_LITTLE_ENDIAN) || !defined(B_BIG_ENDIAN)
# error Unknown endianness
#endif
'''
    repl = '''#elif defined VITA
# define B_LITTLE_ENDIAN 1
# define B_BIG_ENDIAN    0
# define B_SWAP64(x) __bswap64(x)
# define B_SWAP32(x) __bswap32(x)
# define B_SWAP16(x) __bswap16(x)

#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define B_LITTLE_ENDIAN 1
#  define B_BIG_ENDIAN    0
# elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define B_LITTLE_ENDIAN 0
#  define B_BIG_ENDIAN    1
# else
#  error Unsupported GCC byte order
# endif
# define B_ENDIAN_C_INLINE 1

#elif defined(__MIPSEB__) || defined(__MIPSEB) || defined(_MIPSEB)
# define B_LITTLE_ENDIAN 0
# define B_BIG_ENDIAN    1
# define B_ENDIAN_C_INLINE 1

#elif defined(__MIPSEL__) || defined(__MIPSEL) || defined(_MIPSEL)
# define B_LITTLE_ENDIAN 1
# define B_BIG_ENDIAN    0
# define B_ENDIAN_C_INLINE 1
#endif

#if !defined(B_LITTLE_ENDIAN) || !defined(B_BIG_ENDIAN)
# error Unknown endianness
#endif
'''
    if "defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)" in s:
        print(f"{path}: already patched")
        return
    if anchor not in s:
        raise SystemExit(f"{path}: expected endian anchor missing")
    path.write_text(s.replace(anchor, repl, 1))
    print(f"{path}: patched")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: patch-compat-endian.py path/to/compat.h")
    patch(Path(sys.argv[1]))
