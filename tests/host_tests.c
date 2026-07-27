#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blood64_art.h"
#include "blood64_rff.h"

static void wr16le(uint8_t *p, uint16_t v) { p[0] = v; p[1] = v >> 8; }
static void wr32le(uint8_t *p, uint32_t v) {
    p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}
static void crypt(uint8_t *p, size_t n, uint16_t key) {
    for (size_t i = 0; i < n; ++i, ++key) p[i] ^= (uint8_t)(key >> 1);
}

static void make_art(const char *path) {
    /* ART v1 with 3 tiles: 2x2, empty, 1x3. */
    uint8_t buf[16 + 6 + 6 + 12 + 7] = {0};
    wr32le(buf + 0, 1);
    wr32le(buf + 4, 3);
    wr32le(buf + 8, 0);
    wr32le(buf + 12, 2);
    wr16le(buf + 16, 2); wr16le(buf + 18, 0); wr16le(buf + 20, 1);
    wr16le(buf + 22, 2); wr16le(buf + 24, 0); wr16le(buf + 26, 3);
    for (int i = 0; i < 7; ++i) buf[40 + i] = (uint8_t)i;
    FILE *f = fopen(path, "wb"); assert(f);
    assert(fwrite(buf, 1, sizeof(buf), f) == sizeof(buf));
    fclose(f);
}

static void make_rff(const char *path) {
    /* RFF v3 with one encrypted dictionary entry and one plain payload. */
    const uint32_t dict_off = 32;
    const uint32_t payload_off = 32 + 48;
    uint8_t buf[32 + 48 + 4] = {0};
    memcpy(buf, "RFF\x1a", 4);
    wr16le(buf + 4, 0x0301);
    wr32le(buf + 8, dict_off);
    wr32le(buf + 12, 1);

    uint8_t *d = buf + 32;
    wr32le(d + 16, payload_off);
    wr32le(d + 20, 4);
    d[32] = B64_RFF_DICT_ID;
    memcpy(d + 33, "MAP", 3);
    memcpy(d + 36, "E1M1", 4);
    wr32le(d + 44, 42);

    uint32_t key32 = dict_off + (0x0301u & 0xffu) * dict_off;
    crypt(d, 48, (uint16_t)key32);

    memcpy(buf + payload_off, "TEST", 4);

    FILE *f = fopen(path, "wb"); assert(f);
    assert(fwrite(buf, 1, sizeof(buf), f) == sizeof(buf));
    fclose(f);
}

int main(void) {
    char err[128];
    const char *artp = "build/test.art";
    const char *rffp = "build/test.rff";

    make_art(artp);
    b64_art_info_t art;
    assert(b64_art_probe(artp, &art, err, sizeof(err)) == 0);
    assert(art.version == 1);
    assert(art.tile_count == 3);
    assert(art.nonempty_tiles == 2);
    assert(art.pixel_bytes == 7);
    assert(art.max_width == 2);
    assert(art.max_height == 3);

    make_rff(rffp);
    b64_rff_t rff;
    assert(b64_rff_open(rffp, &rff, err, sizeof(err)) == 0);
    assert(rff.header.version == 0x0301);
    assert(rff.header.file_count == 1);
    const b64_rff_entry_t *e = b64_rff_find_name(&rff, "e1m1", "map");
    assert(e);
    assert(e->id == 42);
    char payload[4];
    assert(b64_rff_read_entry(rffp, e, payload, sizeof(payload), err, sizeof(err)) == 0);
    assert(memcmp(payload, "TEST", 4) == 0);
    b64_rff_close(&rff);

    puts("blood64 host parser tests: PASS");
    return 0;
}
