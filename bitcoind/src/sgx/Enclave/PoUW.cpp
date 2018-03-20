//
// Created by fan on 6/9/16.
//

#include <Debug.h>
#include "pouw_defs.h"
#include "sgx_trts.h"
#include "sgx_tcrypto.h"
#include "Log.h"
#include "Enclave_t.h"
#include "sgx_utils.h"
#include "string.h"

#include "math.h"

// add output and include hash (output) in the report
// problem: prefix, target
// solution: suffix such that sha(prefix + suffix) < target
// verifier:
// 2. useful work verification


inline bool smaller(uint8_t a[], uint8_t b[]) {
    for (int i = 0; i < 32; i++) {
        if (a[i] == b[i])
            continue;
        return a[i] < b[i];
    }
    return false;
}

void work(uint8_t* target, uint8_t* guess, sgx_sha256_hash_t* hash, int guess_byte, int* n_guess) {
//    uint8_t guess[32];
//    memset(guess, 0, 32);
//    memcpy(guess, work->prefix, sizeof(work->prefix));

    int j = 0;

    /*
    if (guess_byte == 32) {
        sgx_sha256_msg(guess, 32, hash);
        sgx_sha256_msg((uint8_t *) hash, 32, hash);

        *n_guess += 1;
        if (smaller((uint8_t*) hash, target)) {
            return true;
        }
        else {
            return false;
        }
    }

    for (j = 0; j < 256; j++) {
        if (work(target, guess, hash, guess_byte + 1, n_guess)) {
            return true;
        }
    }
    */

    *n_guess = 2000;

    for (j = 0; j < *n_guess; j++) {
        guess[31] = (uint8_t) (j % 200);
        sgx_sha256_msg(guess, 32, hash);
        sgx_sha256_msg((uint8_t *) hash, 32, hash);
    }

}

// TODO: add win_prob as a separate parameter
int pouw_main (
        pow_spec* spec, current_hash* cur_hash, sgx_target_info_t* qe_info, 
        sgx_report_t* pouw_report, sgx_report_t* job_report, job_result* result) 
{
    sgx_sha256_hash_t hash_buf;
    sgx_status_t ret;
    unsigned char ticket;
    uint8_t is_win = 0;
    double overall_win_prob, ticket_normalized;

    result->n_guess = 0;
    memset(result->guess, 0, 32);
    memcpy(result->guess, spec->prefix, sizeof(spec->prefix));

    work(spec->target, result->guess, &hash_buf, sizeof(spec->prefix), &result->n_guess);

    sgx_read_rand(&ticket, 1);

    overall_win_prob = 1 - pow(1 - spec->prob, result->n_guess);
    ticket_normalized = (double) ticket / 255.0;

    if (ticket_normalized < overall_win_prob) {
        is_win = 1;
    }

    // printf_sgx("win_prob_from_nBits: %f\n", spec->prob);
    // printf_sgx("overall_win_prob: %f\n", overall_win_prob);
    // printf_sgx("ticket_normalized: %f\n", ticket_normalized);
    // printf_sgx("is_win: %d\n", is_win);

    // prepare report
    sgx_report_data_t pouw_report_data; // user defined data
    memset(pouw_report_data.d, 0x0, sizeof(pouw_report_data.d)); 

    blockchain_comm* comm = (blockchain_comm*) pouw_report_data.d;
    comm->difficulty = spec->prob;
    memcpy(comm->header_hash, cur_hash->h, sizeof (current_hash));
    comm->is_win = is_win;

    ret = sgx_create_report(qe_info, &pouw_report_data, pouw_report);
    if (ret != SGX_SUCCESS) {
        LL_CRITICAL("Failed to create PoUW report, returned %d", ret);
        return ret;
    }

    // prepare second report
    sgx_report_data_t job_report_data;
    memset(job_report_data.d, 0x0, sizeof(pouw_report_data.d));
    // put a hash 
    sgx_sha256_msg((uint8_t*) result, sizeof(job_result), (sgx_sha256_hash_t*) job_report_data.d);

    ret = sgx_create_report (qe_info, &job_report_data, job_report);
    if (ret != SGX_SUCCESS) {
        LL_CRITICAL("failed to create JOB report, returned %d", ret);
        return ret;
    }

    return 0;
}
