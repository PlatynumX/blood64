#!/usr/bin/env python3
from pathlib import Path
import sys

MARKER = "\t\t\tcase T_UNDEFTEXTURE:\n\t\t\tcase T_UNDEFTEXTURERANGE:\n"

COMPLETE_TAIL = '''\t\t\tcase T_UNDEFTEXTURE:
\t\t\tcase T_UNDEFTEXTURERANGE:
\t\t\t\t{
\t\t\t\t\tint r0,r1,i;

\t\t\t\t\tif (scriptfile_getsymbol(script,&r0)) break;
\t\t\t\t\tif (tokn == T_UNDEFTEXTURERANGE) {
\t\t\t\t\t\tif (scriptfile_getsymbol(script,&r1)) break;
\t\t\t\t\t\tif (r1 < r0) {
\t\t\t\t\t\t\tint t = r1;
\t\t\t\t\t\t\tr1 = r0;
\t\t\t\t\t\t\tr0 = t;
\t\t\t\t\t\t\tbuildprintf("Warning: backwards tile range on line %s:%d\\n", script->filename, scriptfile_getlinum(script,cmdtokptr));
\t\t\t\t\t\t}
\t\t\t\t\t\tif (r0 < 0 || r1 >= MAXTILES) {
\t\t\t\t\t\t\tbuildprintf("Error: invalid tile range on line %s:%d\\n", script->filename, scriptfile_getlinum(script,cmdtokptr));
\t\t\t\t\t\t\tbreak;
\t\t\t\t\t\t}
\t\t\t\t\t} else {
\t\t\t\t\t\tr1 = r0;
\t\t\t\t\t\tif ((unsigned)r0 >= (unsigned)MAXTILES) {
\t\t\t\t\t\t\tbuildprintf("Error: invalid tile number on line %s:%d\\n", script->filename, scriptfile_getlinum(script,cmdtokptr));
\t\t\t\t\t\t\tbreak;
\t\t\t\t\t\t}
\t\t\t\t\t}

#if USE_POLYMOST && USE_OPENGL
\t\t\t\t\tfor (; r0 <= r1; r0++)
\t\t\t\t\t\tfor (i=MAXPALOOKUPS-1; i>=0; i--)
\t\t\t\t\t\t\thicclearsubst(r0,i);
#endif
\t\t\t\t}
\t\t\t\tbreak;

\t\t\tdefault:
\t\t\t\tbuildputs("Unknown token.\\n"); break;
\t\t}
\t}
\treturn 0;
}

int loaddefinitionsfile(const char *fn)
{
\tscriptfile *script;

\tscript = scriptfile_fromfile(fn);
\tif (!script) return -1;

\tdefsparser(script);

\tscriptfile_close(script);
\tscriptfile_clearsymbols();

\treturn 0;
}

// vim:ts=4:
'''

def main(path_str):
    path = Path(path_str)
    s = path.read_text()
    if "int loaddefinitionsfile(const char *fn)" in s and s.rstrip().endswith("// vim:ts=4:"):
        print(f"{path}: already complete")
        return
    pos = s.find(MARKER)
    if pos < 0:
        raise SystemExit(f"{path}: marker not found")
    path.write_text(s[:pos] + COMPLETE_TAIL)
    print(f"{path}: repaired")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: patch-defs.py path/to/defs.c")
    main(sys.argv[1])
