//
// Created by fan on 6/9/16.
//

#include <stdint.h>

#ifndef POUW_POUW_H
#define POUW_POUW_H

// TODO: remove it later
typedef struct _pow_prob {
    uint8_t prefix[16];
    uint8_t target[32];
    // TODO: move prob outside of the spec
    double prob;
} pow_spec;

// TODO: remove it later
typedef struct _current_hash {
    uint8_t h[32];
} current_hash;

// TODO: remove it later
typedef struct _job_result {
    uint8_t guess[32];
    int n_guess;
} job_result;

typedef struct _blockchain_commitment {
    uint8_t header_hash[32];
    double difficulty;
    uint8_t is_win;
} blockchain_comm;

#endif //POUW_POUW_H
