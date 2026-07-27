#include "blood64_rff.h"
#include "blood64_endian.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define RFF_HEADER_SIZE 32u
#define RFF_DICT_ENTRY_SIZE 48u

static void seterr(char *err, size_t n, const char *msg) {
    if (err && n) {
        snprintf(err, n, "%s", msg ? msg : "unknown error");
    }
}

static void rff_crypt(uint8_t *p, size_t length, uint16_t key) {
    for (size_t i = 0; i < length; ++i, ++key) {
        p[i] ^= (uint8_t)(key >> 1);
    }
}

static int type_eq(const char a[4], const char *b) {
    for (int i = 0; i < 3; ++i) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)(b && b[i] ? b[i] : 0);
        if (toupper(ca) != toupper(cb)) return 0;
        if (ca == 0 && cb == 0) break;
    }
    return 1;
}

static int name_eq(const char a[9], const char *b) {
    for (int i = 0; i < 8; ++i) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)(b && b[i] ? b[i] : 0);
        if (toupper(ca) != toupper(cb)) return 0;
        if (ca == 0 && cb == 0) break;
    }
    return 1;
}

int b64_rff_open(const char *path, b64_rff_t *out, char *err, size_t err_len) {
    uint8_t hdr[RFF_HEADER_SIZE];
    uint8_t *dict = NULL;
    FILE *fp = NULL;
    long file_len = 0;

    if (!path || !out) {
        seterr(err, err_len, "invalid arguments");
        return -1;
    }
    memset(out, 0, sizeof(*out));

    fp = fopen(path, "rb");
    if (!fp) {
        seterr(err, err_len, "cannot open RFF");
        return -1;
    }
    if (fseek(fp, 0, SEEK_END) != 0 || (file_len = ftell(fp)) < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        seterr(err, err_len, "cannot determine RFF size");
        fclose(fp);
        return -1;
    }
    if (file_len < (long)RFF_HEADER_SIZE || fread(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        seterr(err, err_len, "short RFF header");
        fclose(fp);
        return -1;
    }
    if (memcmp(hdr, "RFF\x1a", 4) != 0) {
        seterr(err, err_len, "bad RFF signature");
        fclose(fp);
        return -1;
    }

    out->header.version = b64_le16(hdr + 4);
    out->header.dict_offset = b64_le32(hdr + 8);
    out->header.file_count = b64_le32(hdr + 12);

    switch (out->header.version & 0xff00u) {
        case 0x0200u: out->header.encrypted_dictionary = 0; break;
        case 0x0300u: out->header.encrypted_dictionary = 1; break;
        default:
            seterr(err, err_len, "unsupported RFF version");
            fclose(fp);
            return -1;
    }

    if (out->header.file_count > 200000u) {
        seterr(err, err_len, "implausible RFF entry count");
        fclose(fp);
        return -1;
    }

    uint64_t dict_bytes64 = (uint64_t)out->header.file_count * RFF_DICT_ENTRY_SIZE;
    uint64_t dict_end = (uint64_t)out->header.dict_offset + dict_bytes64;
    if (dict_end > (uint64_t)file_len || dict_bytes64 > SIZE_MAX) {
        seterr(err, err_len, "RFF dictionary outside file");
        fclose(fp);
        return -1;
    }

    size_t dict_bytes = (size_t)dict_bytes64;
    if (dict_bytes) {
        dict = (uint8_t *)malloc(dict_bytes);
        out->entries = (b64_rff_entry_t *)calloc(out->header.file_count, sizeof(b64_rff_entry_t));
        if (!dict || !out->entries) {
            seterr(err, err_len, "out of memory reading RFF dictionary");
            free(dict);
            free(out->entries);
            out->entries = NULL;
            fclose(fp);
            return -1;
        }
        if (fseek(fp, (long)out->header.dict_offset, SEEK_SET) != 0 ||
            fread(dict, 1, dict_bytes, fp) != dict_bytes) {
            seterr(err, err_len, "cannot read RFF dictionary");
            free(dict);
            b64_rff_close(out);
            fclose(fp);
            return -1;
        }

        if (out->header.encrypted_dictionary) {
            uint32_t key32 = out->header.dict_offset +
                (uint32_t)(out->header.version & 0xffu) * out->header.dict_offset;
            rff_crypt(dict, dict_bytes, (uint16_t)key32);
        }

        for (uint32_t i = 0; i < out->header.file_count; ++i) {
            const uint8_t *d = dict + (size_t)i * RFF_DICT_ENTRY_SIZE;
            b64_rff_entry_t *e = &out->entries[i];
            e->offset = b64_le32(d + 16);
            e->size = b64_le32(d + 20);
            e->flags = d[32];
            memcpy(e->type, d + 33, 3);
            e->type[3] = 0;
            memcpy(e->name, d + 36, 8);
            e->name[8] = 0;
            e->id = b64_le32(d + 44);

            if ((uint64_t)e->offset + (uint64_t)e->size > (uint64_t)file_len) {
                seterr(err, err_len, "RFF resource outside file");
                free(dict);
                b64_rff_close(out);
                fclose(fp);
                return -1;
            }
        }
    }

    free(dict);
    fclose(fp);
    return 0;
}

void b64_rff_close(b64_rff_t *rff) {
    if (!rff) return;
    free(rff->entries);
    memset(rff, 0, sizeof(*rff));
}

const b64_rff_entry_t *b64_rff_find_name(const b64_rff_t *rff, const char *name, const char *type) {
    if (!rff || !name || !type) return NULL;
    for (uint32_t i = 0; i < rff->header.file_count; ++i) {
        const b64_rff_entry_t *e = &rff->entries[i];
        if (name_eq(e->name, name) && type_eq(e->type, type)) return e;
    }
    return NULL;
}

const b64_rff_entry_t *b64_rff_find_id(const b64_rff_t *rff, uint32_t id, const char *type) {
    if (!rff || !type) return NULL;
    for (uint32_t i = 0; i < rff->header.file_count; ++i) {
        const b64_rff_entry_t *e = &rff->entries[i];
        if (e->id == id && type_eq(e->type, type)) return e;
    }
    return NULL;
}

int b64_rff_read_entry(const char *path, const b64_rff_entry_t *entry, void *dst, size_t dst_len,
                       char *err, size_t err_len) {
    if (!path || !entry || !dst || dst_len < entry->size) {
        seterr(err, err_len, "invalid resource read");
        return -1;
    }
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        seterr(err, err_len, "cannot reopen RFF");
        return -1;
    }
    if (fseek(fp, (long)entry->offset, SEEK_SET) != 0 ||
        fread(dst, 1, entry->size, fp) != entry->size) {
        seterr(err, err_len, "cannot read RFF resource");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    if (entry->flags & B64_RFF_DICT_CRYPT) {
        size_t n = entry->size > 0x100u ? 0x100u : entry->size;
        rff_crypt((uint8_t *)dst, n, 0);
    }
    return 0;
}
