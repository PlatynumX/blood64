#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t version;
    uint32_t numtiles_legacy;
    uint32_t first_tile;
    uint32_t last_tile;
    uint32_t tile_count;
    uint64_t pixel_bytes;
    uint32_t nonempty_tiles;
    uint16_t max_width;
    uint16_t max_height;
} b64_art_info_t;

int b64_art_probe(const char *path, b64_art_info_t *out, char *err, size_t err_len);

#ifdef __cplusplus
}
#endif
