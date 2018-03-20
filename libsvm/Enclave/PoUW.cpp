#include "string.h"
#include "pouw_defs.h"
#include "Enclave_t.h"
#include "PoUW.h"
#include "external/miniz.h"
#include "rand_hardware.h"
#include "stub.h"
#include "protein/GeneticAlgorithm.h"

using namespace std;

int dummy_attestation(sgx_target_info_t *qe_info, sgx_report_t *report) {
    unsigned char random_num = 0;
    sgx_read_rand(&random_num, sizeof(random_num));

    // prepare report
    sgx_report_data_t report_data; // user defined data
    memset(report_data.d, 0, sizeof(report_data.d));

    memset(qe_info->reserved1, 0, sizeof (qe_info->reserved1));
    memset(qe_info->reserved2, 0, sizeof (qe_info->reserved2));
    sgx_status_t ret = sgx_create_report (qe_info, &report_data, report);
    if (ret != SGX_SUCCESS) {
        LL_CRITICAL("failed to create report, returned %d", ret);
        return ret;
    }

    return ret;
}

void sgx_predict(struct svm_model *model, svm_node **x, size_t x_len, double *y, sgx_target_info_t *qe_info, sgx_report_t *report)
{
    for (int i = 0; i < x_len; i++) {
        y[i] = (int) svm_predict(model, x[i]);
    }
    dummy_attestation(qe_info, report);
}

std::string SEQ150 = "110101010111101000100110101010111101000100010000100010001011110101010111101010101111010001000100001000100010111101010101101000010001000101111010101011";
std::string SEQ156 = "000000000001001111111111100111111111110011111111111001111111111100111111111110011111111111001111111111100111111111110011111111111001111111111100100000000000";

/* static members have to be init in global scope
 */
GeneticAlgorithmParams GeneticAlgorithm::params = {
        SEQ156,  // sequence
        100,     // populationSize
        300,     // generations
        0.10,   // elitePercent
        0.15,   // crossoverPercent
        0.15    // mutationPercent
};

void protein_folding() {
    GeneticAlgorithm geneticAlgorithm;
    geneticAlgorithm.run();
}


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;

int zlib_compress(const char *s_pStr, sgx_target_info_t *qe_info, sgx_report_t *report) {
    int cmp_status;
    uLong src_len = (uLong) strlen(s_pStr);
    uLong cmp_len = compressBound(src_len);
    uLong uncomp_len = src_len;
    uint8 *pCmp, *pUncomp;
    uint total_succeeded = 0;

    // Allocate buffers to hold compressed and uncompressed data.
    pCmp = (mz_uint8 *) malloc((size_t) cmp_len);
    pUncomp = (mz_uint8 *) malloc((size_t) src_len);
    if ((!pCmp) || (!pUncomp)) {
        LL_CRITICAL("Out of memory!");
        return EXIT_FAILURE;
    }

    // Compress the string.
    cmp_status = compress(pCmp, &cmp_len, (const unsigned char *) s_pStr, src_len);
    if (cmp_status != Z_OK) {
        printf_sgx("compress() failed!\n");
        free(pCmp);
        free(pUncomp);
        return EXIT_FAILURE;
    }

    LL_NOTICE("Compressed from %u to %u bytes", (mz_uint32) src_len, (mz_uint32) cmp_len);


    // Decompress.
    cmp_status = uncompress(pUncomp, &uncomp_len, pCmp, cmp_len);

    if (cmp_status != Z_OK) {
        printf_sgx("uncompress failed!\n");
        free(pCmp);
        free(pUncomp);
        return EXIT_FAILURE;
    }

    LL_NOTICE("Decompressed from %u to %u bytes", (mz_uint32) cmp_len, (mz_uint32) uncomp_len);

    // Ensure uncompress() returned the expected data.
    if ((uncomp_len != src_len) || (memcmp(pUncomp, s_pStr, (size_t) src_len))) {
        printf_sgx("Decompression failed!\n");
        free(pCmp);
        free(pUncomp);
        return EXIT_FAILURE;
    }

    free(pCmp);
    free(pUncomp);

    dummy_attestation(qe_info, report);
    return EXIT_SUCCESS;
}


long pouw_main (pow_spec* spec, output_t* output) {
    int ret = 0;
    output->label = svm_predict(spec->model, spec->x);
    return ret;
}
