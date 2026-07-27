# Blood64 aggressive bring-up roadmap

## r1 — resource truth test
- 8 MB requirement
- DragonFS
- RFF v2/v3 header + encrypted dictionary
- per-resource bounds validation
- ART v1 metadata parser
- real shareware data

## r2 — E1M1 database
- MAP v7 parser
- shareware E1M1 extraction through RFF
- sectors/walls/sprites
- XSECTOR/XWALL/XSPRITE
- endian conversion
- map counts + player start diagnostics

## r3 — first picture
- trimmed JFBuild classic renderer core
- 320x240 8-bit framebuffer
- palette/shade tables
- static player view
- indexed8 -> N64 framebuffer presentation

## r4 — move
- 120 Hz Build clock
- controller -> GINPUT bridge
- clipping / movement / use
- no AI, no sound

## r5 — game
- current NBlood gameplay layer
- AI, actors, triggers, weapons
- retail-shareware compatibility target only

## r6 — sound
- RAW/SFX on-demand RFF loading
- libdragon mixer backend
- 8-16 voice priority stealing
- ambient residency policy

## r7 — optimization
- measured MAXTILES / X* limits
- tile/resource cache budget
- renderer profiling
- optimize vline/span kernels
- optional RSP assists

## later
- original voxels
- music
- menus/cutscenes
- saving
- registered Blood data support
