#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_pouw_main_t {
	int ms_retval;
	pow_spec* ms_work;
	current_hash* ms_prev;
	sgx_target_info_t* ms_quote_enc_info;
	sgx_report_t* ms_pouw_att;
	sgx_report_t* ms_job_att;
	job_result* ms_output;
} ms_pouw_main_t;

typedef struct ms_ocall_print_string_t {
	int ms_retval;
	char* ms_str;
} ms_ocall_print_string_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ms->ms_retval = ocall_print_string((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_Enclave = {
	1,
	{
		(void*)Enclave_ocall_print_string,
	}
};
sgx_status_t pouw_main(sgx_enclave_id_t eid, int* retval, pow_spec* work, current_hash* prev, sgx_target_info_t* quote_enc_info, sgx_report_t* pouw_att, sgx_report_t* job_att, job_result* output)
{
	sgx_status_t status;
	ms_pouw_main_t ms;
	ms.ms_work = work;
	ms.ms_prev = prev;
	ms.ms_quote_enc_info = quote_enc_info;
	ms.ms_pouw_att = pouw_att;
	ms.ms_job_att = job_att;
	ms.ms_output = output;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

