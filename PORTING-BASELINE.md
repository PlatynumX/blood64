# Blood64 consolidated source baseline

Pinned upstream:
- JFBlood: 987f1a184e11f8c1e201368500c332a1cb4f0031
- JFBuild: 068a8cc2e21e3788a4c3325fbc18d2661474aa95

Source model:
1. Restore/fetch pristine pinned upstream.
2. Stage the selected JFBuild/Blood source.
3. Replace exactly seven verified files from `overrides/`.
4. Never mutate the vendor checkout.

The overrides consolidate the previously proven fixes for:
- N64/GCC big-endian detection and byte swapping;
- libdragon directory enumeration in JFBuild compat;
- repaired truncated JFBuild defs.c;
- libdragon directory enumeration and MIPS initialization fixes in kplib;
- classic-renderer-only Blood actor/database changes;
- C++-safe min/max handling based on current NBlood.

Legacy patch/mutation scripts are intentionally removed from the active tree.
