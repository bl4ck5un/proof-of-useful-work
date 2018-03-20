//
// Created by fan on 6/9/16.
//

#include <stdint.h>

#ifndef POUW_POUW_H
#define POUW_POUW_H

#define MAX_LOOP_N 100

typedef struct _pow_prob {
    uint8_t prefix[16];
    uint8_t target[32];
} pow_spec;

typedef struct _current_hash {
    uint8_t h[32];
} current_hash;

typedef struct _output_t {
    uint8_t guess[32];
    int n_guess;
    uint8_t is_win;
} output_t;

#endif //POUW_POUW_H
