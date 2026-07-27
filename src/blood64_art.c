#include "blood64_art.h"
#include "blood64_endian.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void seterr(char *err, size_t n, const char *msg) {
    if (err && n) snprintf(err, n, "%s", msg ? msg : "unknown error");
}

int b64_art_probe(const char *path, b64_art_info_t *out, char *err, size_t err_len) {
    uint8_t hdr[16];
    FILE *fp = NULL;
    long file_len = 0;
    uint8_t *sizes = NULL;

    if (!path || !out) {
        seterr(err, err_len, "invalid arguments");
        return -1;
    }
    memset(out, 0, sizeof(*out));

    fp = fopen(path, "rb");
    if (!fp) {
        seterr(err, err_len, "cannot open ART");
        return -1;
    }
    if (fseek(fp, 0, SEEK_END) != 0 || (file_len = ftell(fp)) < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        seterr(err, err_len, "cannot determine ART size");
        fclose(fp);
        return -1;
    }
    if (fread(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        seterr(err, err_len, "short ART header");
        fclose(fp);
        return -1;
    }

    out->version = b64_le32(hdr + 0);
    out->numtiles_legacy = b64_le32(hdr + 4);
    out->first_tile = b64_le32(hdr + 8);
    out->last_tile = b64_le32(hdr + 12);

    if (out->version != 1) {
        seterr(err, err_len, "unsupported ART version");
        fclose(fp);
        return -1;
    }
    if (out->last_tile < out->first_tile) {
        seterr(err, err_len, "invalid ART tile range");
        fclose(fp);
        return -1;
    }
    uint64_t count64 = (uint64_t)out->last_tile - out->first_tile + 1u;
    if (count64 > 65536u) {
        seterr(err, err_len, "implausible ART tile count");
        fclose(fp);
        return -1;
    }
    out->tile_count = (uint32_t)count64;

    uint64_t meta64 = 16u + count64 * 2u + count64 * 2u + count64 * 4u;
    if (meta64 > (uint64_t)file_len || count64 * 4u > SIZE_MAX) {
        seterr(err, err_len, "ART metadata outside file");
        fclose(fp);
        return -1;
    }

    sizes = (uint8_t *)malloc((size_t)count64 * 4u);
    if (!sizes) {
        seterr(err, err_len, "out of memory reading ART sizes");
        fclose(fp);
        return -1;
    }

    if (fseek(fp, 16, SEEK_SET) != 0 ||
        fread(sizes, 1, (size_t)count64 * 4u, fp) != (size_t)count64 * 4u) {
        seterr(err, err_len, "cannot read ART dimensions");
        free(sizes);
        fclose(fp);
        return -1;
    }

    const uint8_t *xs = sizes;
    const uint8_t *ys = sizes + count64 * 2u;
    uint64_t pixel_bytes = 0;
    for (uint32_t i = 0; i < out->tile_count; ++i) {
        uint16_t w = b64_le16(xs + (size_t)i * 2u);
        uint16_t h = b64_le16(ys + (size_t)i * 2u);
        if (w && h) {
            ++out->nonempty_tiles;
            if (w > out->max_width) out->max_width = w;
            if (h > out->max_height) out->max_height = h;
            pixel_bytes += (uint64_t)w * (uint64_t)h;
        }
    }
    out->pixel_bytes = pixel_bytes;

    if (meta64 + pixel_bytes > (uint64_t)file_len) {
        seterr(err, err_len, "ART pixel data outside file");
        free(sizes);
        fclose(fp);
        return -1;
    }

    free(sizes);
    fclose(fp);
    return 0;
}
