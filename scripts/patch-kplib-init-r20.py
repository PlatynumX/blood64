#!/usr/bin/env python3
from pathlib import Path
import sys

def one(s, old, new, label):
    if new in s:
        return s
    if old not in s:
        raise SystemExit(f"{label}: expected exact declaration not found")
    return s.replace(old,new,1)

def patch(path):
    # kplib.c contains legacy non-UTF8 bytes; latin-1 preserves every byte.
    s=path.read_text(encoding="latin-1")

    # yrbrend: j starts at 1 and is decremented before cr/cb are consumed,
    # so valid execution overwrites both before first use. Initializers only
    # make that invariant explicit to GCC.
    s=one(s,
        "int i, j, ox, oy, xx, yy, xxx, yyy, xxxend, yyyend, yv, cr, cb, *odc, *dc, *dc2;",
        "int i, j, ox, oy, xx, yy, xxx, yyy, xxxend, yyyend, yv, cr = 0, cb = 0, *odc, *dc, *dc2;",
        "yrbrend cr/cb")

    # These match current NBlood's hardened kpegrend initialization strategy.
    s=one(s,
        "int i, j, p, v, leng, xdim, ydim, index, prec, restartcnt, restartinterval;",
        "int i, j, p, v, leng, xdim = 0, ydim = 0, index, prec, restartcnt, restartinterval;",
        "jpeg dimensions")
    s=one(s,
        "int x, y, z, xx, yy, zz, *dc, *dc2, num, curbits, c, daval, dabits, *hqval, *hqbits, hqcnt, *quanptr;",
        "int x, y, z, xx, yy, zz, *dc = NULL, *dc2, num, curbits, c, daval, dabits, *hqval, *hqbits, hqcnt, *quanptr = NULL;",
        "jpeg pointers")
    s=one(s,
        "int passcnt = 0, ghsampmax, gvsampmax, glhsampmax, glvsampmax, glhstep, glvstep;",
        "int passcnt = 0, ghsampmax = 0, gvsampmax = 0, glhsampmax = 0, glvsampmax = 0, glhstep, glvstep;",
        "jpeg sampling")
    s=one(s,
        "short *dctbuf = 0, *dctptr[12], *ldctptr[12], *dcs;",
        "short *dctbuf = 0, *dctptr[12], *ldctptr[12], *dcs = NULL;",
        "jpeg dcs")

    # Current NBlood initializes these palette pointers to NULL too.
    s=one(s,
        "const unsigned char *ptr, *cptr;",
        "const unsigned char *ptr, *cptr = NULL;",
        "gif palette pointer")
    s=one(s,
        "const unsigned char *fptr, *cptr, *nptr;",
        "const unsigned char *fptr, *cptr = NULL, *nptr;",
        "tga palette pointer")

    # In the old GIF renderer p is deliberately never dereferenced while x is
    # forced out of the unsigned clipping range. Give it a defined sentinel
    # value for the compiler without changing the clipping/state machine.
    s=one(s,
        "INT_PTR p;\n\tunsigned char numbits, startnumbits, chunkind, ilacefirst;",
        "INT_PTR p = 0;\n\tunsigned char numbits, startnumbits, chunkind, ilacefirst;",
        "gif framebuffer pointer")

    path.write_text(s,encoding="latin-1")
    print(f"{path}: r20 initialization patch applied")

if __name__=="__main__":
    if len(sys.argv)!=2:
        raise SystemExit("usage: patch-kplib-init-r20.py path/to/kplib.c")
    patch(Path(sys.argv[1]))
