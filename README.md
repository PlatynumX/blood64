# Blood64 r3 — real engine integration

This revision stops building around Blood and puts the actual engine/game source into the N64 build.

## Upstream pinned

- JFBlood 3DS: `987f1a184e11f8c1e201368500c332a1cb4f0031`
- JFBuild: the `068a8cc` submodule line pinned by that JFBlood snapshot.

The engine is fetched by commit, verified, then staged into `engine/`.

## What is compiled now

Real JFBuild classic core:
`a-c`, `asmprot`, `cache1d`, `compat`, `crc32`, `defs`, `engine`, `kplib`,
`osd`, `pragmas`, `scriptfile`, fonts, null multiplayer, version.

Real Blood gameplay:
actors, all AI modules, Caleb/player, weapons, database/map code, triggers,
sector effects, QAV/SEQ, palettes, RFF resources, view, menus, levels, etc.

Polymost is deliberately not in the N64 object list.

## Why r3 is a compile milestone rather than another fake ROM

The original JFBlood constrained-console build compiles this same classic JFBuild core and full
Blood object graph. Blood64 now forces that real source through the libdragon MIPS compiler.
The expected first run may expose platform/API errors; CI captures every one into a downloadable
build report so we close the actual port deltas instead of inventing stubs blindly.

## Run

```bash
./scripts/fetch-engine.sh
./scripts/stage-engine.sh
libdragon make -f Makefile.engine -j2
```

GitHub Actions does those steps automatically.

## N64 layer already present

`n64/platform_n64.c` allocates a real 320x240 indexed Build framebuffer, initializes controllers
and VI, converts the Build 6-bit palette to RGB5551, presents frames, and exposes a 120 Hz clock.

## Next

Fix the concrete MIPS compile errors from this real source graph, then link the executable and
route JFBuild's platform/video calls into `platform_n64.c`. No more parser-only milestones.
