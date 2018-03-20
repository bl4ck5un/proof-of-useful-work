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

#define USER_DEFINED __attribute__ ((section (".userdefined_pouw")))

#if defined(__cplusplus)
extern "C" {
#endif

void sgx_predict(struct svm_model *model, struct svm_node **x, size_t x_len, double *y, sgx_target_info_t *qe_info, sgx_report_t *report);
int zlib_compress(const char *s_pStr, sgx_target_info_t *qe_info, sgx_report_t *report);
long pouw_main (pow_spec* spec, output_t* output);

#if defined(__cplusplus)
}
#endif
