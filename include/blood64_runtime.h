#pragma once
#include <stdint.h>

typedef struct {
    float move_fwd;
    float move_side;
    float turn;
    int fire, use, jump, crouch, run;
    int next_weapon, prev_weapon;
} b64_input_t;

typedef struct {
    uint64_t tick120;
    double accumulator;
} b64_clock_t;

void b64_input_poll(b64_input_t *in);
void b64_clock_init(b64_clock_t *c);
int b64_clock_step(b64_clock_t *c);
