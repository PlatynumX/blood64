#include <libdragon.h>
#include <string.h>
#include "blood64_runtime.h"

void b64_input_poll(b64_input_t *in) {
    memset(in, 0, sizeof(*in));
    controller_scan();
    struct controller_data held = get_keys_held();
    struct controller_data down = get_keys_down();
    struct controller_data keys = get_keys_pressed();

    float sx = held.c[0].x / 80.0f;
    float sy = held.c[0].y / 80.0f;
    if (sx > 1.f) sx = 1.f;
    if (sx < -1.f) sx = -1.f;
    if (sy > 1.f) sy = 1.f;
    if (sy < -1.f) sy = -1.f;

    in->move_fwd = sy;
    in->turn = sx;
    in->fire = held.c[0].Z;
    in->use = down.c[0].A;
    in->jump = held.c[0].C_up;
    in->crouch = held.c[0].C_down;
    in->run = held.c[0].B;
    in->move_side = (held.c[0].C_right ? 1.f : 0.f) - (held.c[0].C_left ? 1.f : 0.f);
    in->next_weapon = keys.c[0].R;
    in->prev_weapon = keys.c[0].L;
}

void b64_clock_init(b64_clock_t *c) {
    memset(c, 0, sizeof(*c));
}

int b64_clock_step(b64_clock_t *c) {
    /* Gameplay integration target is Build's 120 Hz clock.
       The final engine bridge should source this from timer_ticks()/TICKS_PER_SECOND
       and consume catch-up ticks deterministically. */
    c->tick120++;
    return 1;
}
