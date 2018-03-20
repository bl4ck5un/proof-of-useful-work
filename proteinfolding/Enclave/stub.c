#include "sgx_utils.h"
#include "pouw_defs.h"
#include "string.h"
#include "Debug.h"
#include "Log.h"
#include "blockchain.h"
#include "rand_hardware.h"

#include "sgx_trts.h"
#include "sgx_tcrypto.h"
#include <math.h>


extern long pouw_main(pow_spec*, output_t*);

int run(pow_spec *spec,
        difficulty_t* difficulty,
        block_hash_t *cur_hash,
        sgx_target_info_t *qe_info,
        sgx_report_t *report,
        output_t *output) __attribute__((section(".pouw_enclave_stub")));

int run(pow_spec *spec,
        difficulty_t* difficulty,
        block_hash_t *cur_hash,
        sgx_target_info_t *qe_info,
        sgx_report_t *report,
        output_t *output)
{
    sgx_status_t ret;


    memset(output->guess, 0, 32);
    memcpy(output->guess, spec->prefix, sizeof(spec->prefix));

    long n_work = pouw_main(spec, output);

    LL_NOTICE("n_work is %ld", n_work);

    double total_win_probability = 1 - pow(1 - difficulty->difficulty, n_work);
    unsigned char random_num = 0;
    sgx_read_rand(&random_num, sizeof(random_num));
    float normalized_rand_num = (float) random_num / UCHAR_MAX;
    uint8_t is_win = 0;

    if (normalized_rand_num < total_win_probability)
    {
        is_win = 1;
    }

    // prepare report
    sgx_report_data_t report_data; // user defined data
    memset(report_data.d, 0, sizeof(report_data.d));

    // Create the voucher in place (no copying necessary later): 
    pouw_voucher* p_voucher = (pouw_voucher*) report_data.d;
    p_voucher->difficulty = difficulty->difficulty;
    p_voucher->is_win = is_win;
    memcpy(p_voucher->header_hash, cur_hash->h, sizeof(cur_hash->h));

    // FIXME: only need in SIM mode
    // FIXME: but doesn't hurt to have it anyway
    memset(qe_info->reserved1, 0, sizeof (qe_info->reserved1));
    memset(qe_info->reserved2, 0, sizeof (qe_info->reserved2));
    ret = sgx_create_report (qe_info, &report_data, report);
    if (ret != SGX_SUCCESS) {
        LL_CRITICAL("failed to create report, returned %d", ret);
        return ret;
    }

    return ret;
}

