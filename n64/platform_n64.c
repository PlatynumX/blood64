#include <libdragon.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "blood64_config.h"

/* Build classic renderer writes 8-bit palette indices here. */
uint8_t *blood64_framebuffer8 = NULL;
static uint16_t blood64_palette16[256];
static uint64_t blood64_start_ticks;

int blood64_platform_init(void) {
    if (!is_memory_expanded())
        return -1;

    controller_init();
    timer_init();
    blood64_start_ticks = get_ticks();

    blood64_framebuffer8 = (uint8_t *)malloc(BLOOD64_XDIM * BLOOD64_YDIM);
    if (!blood64_framebuffer8)
        return -2;
    memset(blood64_framebuffer8, 0, BLOOD64_XDIM * BLOOD64_YDIM);

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    return 0;
}

void blood64_set_palette(const uint8_t pal[768]) {
    for (int i = 0; i < 256; ++i) {
        /* Blood/Build palette channels are 0..63. Expand to RGB5551. */
        uint16_t r = (uint16_t)((pal[i*3+0] * 31u + 31u) / 63u);
        uint16_t g = (uint16_t)((pal[i*3+1] * 31u + 31u) / 63u);
        uint16_t b = (uint16_t)((pal[i*3+2] * 31u + 31u) / 63u);
        blood64_palette16[i] = (uint16_t)((r << 11) | (g << 6) | (b << 1) | 1u);
    }
}

void blood64_present(void) {
    surface_t *fb = display_get();
    if (!fb) return;

    uint16_t *dst = (uint16_t *)fb->buffer;
    for (int y = 0; y < BLOOD64_YDIM; ++y) {
        const uint8_t *src = blood64_framebuffer8 + y * BLOOD64_XDIM;
        uint16_t *row = dst + y * (fb->stride / 2);
        for (int x = 0; x < BLOOD64_XDIM; ++x)
            row[x] = blood64_palette16[src[x]];
    }
    display_show(fb);
}

uint32_t blood64_clock120(void) {
    uint64_t elapsed = get_ticks() - blood64_start_ticks;
    return (uint32_t)((elapsed * 120u) / TICKS_PER_SECOND);
}
