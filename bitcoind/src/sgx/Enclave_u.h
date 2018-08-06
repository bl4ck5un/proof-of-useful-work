#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_satus_t etc. */

#include "sgx_report.h"
#include "pouw_defs.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_string, (const char* str));

sgx_status_t pouw_main(sgx_enclave_id_t eid, int* retval, pow_spec* work, current_hash* prev, sgx_target_info_t* quote_enc_info, sgx_report_t* pouw_att, sgx_report_t* job_att, job_result* output);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
