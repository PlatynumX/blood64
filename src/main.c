#include <libdragon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blood64_art.h"
#include "blood64_rff.h"

#define BLOOD_RFF "rom:/blood/BLOOD.RFF"
#define SOUND_RFF "rom:/blood/SOUNDS.RFF"
#define SHARE_ART "rom:/blood/SHARE000.ART"

static void print_rff_summary(const char *label, const char *path) {
    char err[96];
    b64_rff_t rff;
    if (b64_rff_open(path, &rff, err, sizeof(err)) != 0) {
        printf("[FAIL] %s: %s\n", label, err);
        return;
    }

    uint32_t maps = 0, seq = 0, qav = 0, raw = 0, sfx = 0, vox = 0;
    uint32_t locked = 0, crypt = 0;
    uint64_t payload = 0;

    for (uint32_t i = 0; i < rff.header.file_count; ++i) {
        const b64_rff_entry_t *e = &rff.entries[i];
        payload += e->size;
        if (!strcmp(e->type, "MAP")) ++maps;
        else if (!strcmp(e->type, "SEQ")) ++seq;
        else if (!strcmp(e->type, "QAV")) ++qav;
        else if (!strcmp(e->type, "RAW")) ++raw;
        else if (!strcmp(e->type, "SFX")) ++sfx;
        else if (!strcmp(e->type, "KVX")) ++vox;
        if (e->flags & B64_RFF_DICT_LOCK) ++locked;
        if (e->flags & B64_RFF_DICT_CRYPT) ++crypt;
    }

    printf("[ OK ] %s RFF v%04X entries=%lu\n",
           label, rff.header.version, (unsigned long)rff.header.file_count);
    printf("       MAP %lu SEQ %lu QAV %lu RAW %lu SFX %lu KVX %lu\n",
           (unsigned long)maps, (unsigned long)seq, (unsigned long)qav,
           (unsigned long)raw, (unsigned long)sfx, (unsigned long)vox);
    printf("       payload=%lluKB lock=%lu crypt=%lu\n",
           (unsigned long long)(payload / 1024u),
           (unsigned long)locked, (unsigned long)crypt);

    /* Exercise real lookup path against a MAP if one exists. */
    for (uint32_t i = 0; i < rff.header.file_count; ++i) {
        const b64_rff_entry_t *e = &rff.entries[i];
        if (!strcmp(e->type, "MAP")) {
            printf("       first MAP: %s.%s id=%lu size=%lu\n",
                   e->name, e->type, (unsigned long)e->id, (unsigned long)e->size);
            break;
        }
    }

    b64_rff_close(&rff);
}

static void print_art_summary(void) {
    char err[96];
    b64_art_info_t art;
    if (b64_art_probe(SHARE_ART, &art, err, sizeof(err)) != 0) {
        printf("[FAIL] SHARE000.ART: %s\n", err);
        return;
    }
    printf("[ OK ] SHARE000.ART v%lu range=%lu..%lu\n",
           (unsigned long)art.version,
           (unsigned long)art.first_tile, (unsigned long)art.last_tile);
    printf("       slots=%lu used=%lu pixels=%lluKB max=%ux%u\n",
           (unsigned long)art.tile_count, (unsigned long)art.nonempty_tiles,
           (unsigned long long)(art.pixel_bytes / 1024u),
           art.max_width, art.max_height);
}

int main(void) {
    debug_init_isviewer();
    console_init();
    console_set_debug(true);
    console_set_render_mode(RENDER_MANUAL);

    printf("\nBLOOD64 BRING-UP r1\n");
    printf("==================\n");
    printf("RAM: %d KB (%s)\n", get_memory_size() / 1024,
           is_memory_expanded() ? "Expansion Pak" : "Jumper Pak");

    if (!is_memory_expanded()) {
        printf("\n[STOP] Blood64 r1 requires the 8 MB Expansion Pak.\n");
        printf("This is intentional for the initial port target.\n");
        console_render();
        while (1) { }
    }

    int dfs = dfs_init(DFS_DEFAULT_LOCATION);
    if (dfs != DFS_ESUCCESS) {
        printf("\n[FAIL] DragonFS init: %d\n", dfs);
        console_render();
        while (1) { }
    }
    printf("[ OK ] DragonFS mounted\n");

    print_rff_summary("BLOOD", BLOOD_RFF);
    print_rff_summary("SOUNDS", SOUND_RFF);
    print_art_summary();

    printf("\nParser pass complete. Total RAM: %d KB\n", get_memory_size() / 1024);

    printf("\nMilestone 1 reached if all three show [ OK ].\n");
    printf("Next: MAP v7 parser + static E1M1 frame.\n");
    console_render();

    while (1) { }
    return 0;
}
