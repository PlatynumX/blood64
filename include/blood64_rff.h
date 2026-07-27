#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    B64_RFF_DICT_ID    = 1,
    B64_RFF_DICT_LOAD  = 4,
    B64_RFF_DICT_LOCK  = 8,
    B64_RFF_DICT_CRYPT = 16
};

typedef struct {
    uint16_t version;
    uint32_t dict_offset;
    uint32_t file_count;
    int encrypted_dictionary;
} b64_rff_header_t;

typedef struct {
    uint32_t offset;
    uint32_t size;
    uint8_t flags;
    char type[4];
    char name[9];
    uint32_t id;
} b64_rff_entry_t;

typedef struct {
    b64_rff_header_t header;
    b64_rff_entry_t *entries;
} b64_rff_t;

int b64_rff_open(const char *path, b64_rff_t *out, char *err, size_t err_len);
void b64_rff_close(b64_rff_t *rff);
const b64_rff_entry_t *b64_rff_find_name(const b64_rff_t *rff, const char *name, const char *type);
const b64_rff_entry_t *b64_rff_find_id(const b64_rff_t *rff, uint32_t id, const char *type);
int b64_rff_read_entry(const char *path, const b64_rff_entry_t *entry, void *dst, size_t dst_len,
                       char *err, size_t err_len);

#ifdef __cplusplus
}
#endif
