# Blood64 Shareware Bootstrap r1

This package prepares the **Blood v1.11 shareware episode** as the initial Blood64 test-data set.

The actual shareware game data is not embedded in this bootstrap ZIP because the current ChatGPT runtime could verify the public archive but could not materialize binary web downloads. The included script downloads an installed shareware package directly, extracts only the engine data, verifies that the required files exist, and produces SHA-256 checksums for the resulting test set.

## Termux

```sh
unzip blood64-shareware-bootstrap-r1.zip
cd blood64-shareware-bootstrap-r1
bash scripts/fetch-shareware-termux.sh
```

The resulting data is placed in `shareware/`.

## Core shareware differences we must support

The shareware release uses `SHARE000.ART`, not the registered `TILES000.ART` through `TILES017.ART` layout. Its RFF resources and maps also differ from registered Blood. Blood64 should treat the shareware layout as a native target rather than renaming the ART file and hoping current NBlood accepts it.

## Initial Blood64 milestone

Use these files for the first bring-up target:

1. initialize the trimmed Build core;
2. mount/read the shareware data;
3. parse `BLOOD.RFF`;
4. open `SHARE000.ART`;
5. load the first shareware map;
6. render a static 320x240 frame.

No audio, menus, saves, or gameplay are required for that first ROM.
