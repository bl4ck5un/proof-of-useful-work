//
// Created by fanz on 2/15/17.
//

#ifndef POUW_STUB_H
#define POUW_STUB_H

#include "pouw_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

int run(pow_spec *spec,
        difficulty_t *difficulty,
        block_hash_t *cur_hash,
        sgx_target_info_t *qe_info,
        sgx_report_t *report,
        output_t *output) __attribute__((section(".pouw_enclave_stub")));


#ifdef __cplusplus
}
#endif


#endif //POUW_STUB_H
