# Blood64 r2 — playable integration branch

This branch deliberately assumes r1's N64/RFF/ART bring-up succeeds and moves directly to the first
playable target.

## Definition of "playable r2"

- shareware E1M1
- 320x240, 4:3
- classic Build software renderer
- N64 analog movement/turning
- Z fire
- A use
- B run
- C-left/right strafe
- C-up jump
- C-down crouch
- L/R previous/next weapon
- 120 Hz Blood/Build simulation clock
- AI, doors, triggers, pickups, weapons
- no music requirement
- no saves requirement
- voxels may be disabled for first boot
- single-player only

## Integration order

1. Import the JFBuild classic core used by JFBlood:
   engine, a-c, cache1d, tiles, fixed-point math, palette/shade support.
2. Compile it for MIPS with NOASM/classic-only switches.
3. Add `video_n64`: 320x240 indexed framebuffer and VI presentation.
4. Bring JFBlood's Blood database/resource/level code across first.
5. Load shareware E1M1 and spawn Caleb.
6. Add `input_n64` -> Blood GINPUT.
7. Bring actors/triggers/weapons/AI.
8. Replace FX backend with libdragon mixer.
9. Profile on real hardware before touching RSP/RDP acceleration.

## Important

The tiny r1 parsers are bring-up probes, not substitutes for Build/JFBlood. For actual gameplay we must
compile the GPL Build/Blood sources and preserve their license notices/source obligations.
