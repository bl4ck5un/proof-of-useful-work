//
// Created by fanz on 10/10/16.
//

#include <stdio.h>
#include <stdarg.h>

#include <sgx_trts.h>
#include <sgx_utils.h>
#include <sgx_error.h>

#include "parserfactory.h"
#include "../Common/Log.h"
#include "../Common/Debug.h"
#include "measure.h"
#include "../Common/xml_parameter_t.h"

extern "C" int se_trace_internal (int debug_level, const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf_sgx(fmt, ap);
    va_end(ap);
    return 0;
}

using namespace std;

extern "C"
sgx_status_t ecall_analyze_user_enclave(
        unsigned char *elf,
        int elf_size,
        sgx_target_info_t *qe_info,
        xml_parameter_t *xml_config,
        int xml_config_size,
        sgx_report_t *report_template,
        uint8_t *is_compliant)
{
    sgx_status_t ret;
    uint8_t mr_enclave[SGX_HASH_SIZE];
    ElfParser *parser = NULL;

    /*
    /* we need a new buffer here since calculating measurement
    /* will change the efl and cause the compliance analysis to fail.
    */
    uint8_t* tmp_buffer = (uint8_t*) malloc(elf_size);
    if (!tmp_buffer)
    {
        LL_CRITICAL("Can not alloc %d(%#x) B memory for tmp buffer", elf_size);
        ret = SGX_ERROR_OUT_OF_MEMORY;
        goto cleanup;
    }
    memcpy(tmp_buffer, elf, elf_size);
    measure(tmp_buffer, elf_size, xml_config, xml_config_size, mr_enclave);
    free(tmp_buffer);

    parser = get_parser(elf, elf_size);
    ret = parser->run_parser();
    if (ret != 0)
    {
        LL_CRITICAL("Error in run_parser", ret);
        ret = SGX_ERROR_INVALID_STATE;
        goto cleanup;
    }

    *is_compliant = (uint8_t) parser->verify_user_code();

    sgx_report_data_t report_data;
    memcpy(report_data.d, mr_enclave, SGX_HASH_SIZE);
    memset(report_data.d + SGX_HASH_SIZE, *is_compliant, sizeof(report_data.d) - SGX_HASH_SIZE);

    memset(qe_info->reserved1, 0, sizeof (qe_info->reserved1));
    memset(qe_info->reserved2, 0, sizeof (qe_info->reserved2));

    ret = sgx_create_report(qe_info, &report_data, report_template);
    if (ret != SGX_SUCCESS)
    {
        LL_CRITICAL("failed to create report");
        goto cleanup;
    }

    ret = SGX_SUCCESS;

cleanup:
    if (parser) delete(parser);
    return ret;
}
