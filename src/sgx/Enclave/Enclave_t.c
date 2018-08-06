#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */

#include <errno.h>
#include <string.h> /* for memcpy etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)


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

static sgx_status_t SGX_CDECL sgx_pouw_main(void* pms)
{
	ms_pouw_main_t* ms = SGX_CAST(ms_pouw_main_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	pow_spec* _tmp_work = ms->ms_work;
	size_t _len_work = sizeof(*_tmp_work);
	pow_spec* _in_work = NULL;
	current_hash* _tmp_prev = ms->ms_prev;
	size_t _len_prev = sizeof(*_tmp_prev);
	current_hash* _in_prev = NULL;
	sgx_target_info_t* _tmp_quote_enc_info = ms->ms_quote_enc_info;
	size_t _len_quote_enc_info = sizeof(*_tmp_quote_enc_info);
	sgx_target_info_t* _in_quote_enc_info = NULL;
	sgx_report_t* _tmp_pouw_att = ms->ms_pouw_att;
	size_t _len_pouw_att = sizeof(*_tmp_pouw_att);
	sgx_report_t* _in_pouw_att = NULL;
	sgx_report_t* _tmp_job_att = ms->ms_job_att;
	size_t _len_job_att = sizeof(*_tmp_job_att);
	sgx_report_t* _in_job_att = NULL;
	job_result* _tmp_output = ms->ms_output;
	size_t _len_output = sizeof(*_tmp_output);
	job_result* _in_output = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_pouw_main_t));
	CHECK_UNIQUE_POINTER(_tmp_work, _len_work);
	CHECK_UNIQUE_POINTER(_tmp_prev, _len_prev);
	CHECK_UNIQUE_POINTER(_tmp_quote_enc_info, _len_quote_enc_info);
	CHECK_UNIQUE_POINTER(_tmp_pouw_att, _len_pouw_att);
	CHECK_UNIQUE_POINTER(_tmp_job_att, _len_job_att);
	CHECK_UNIQUE_POINTER(_tmp_output, _len_output);

	if (_tmp_work != NULL) {
		_in_work = (pow_spec*)malloc(_len_work);
		if (_in_work == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_work, _tmp_work, _len_work);
	}
	if (_tmp_prev != NULL) {
		_in_prev = (current_hash*)malloc(_len_prev);
		if (_in_prev == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_prev, _tmp_prev, _len_prev);
	}
	if (_tmp_quote_enc_info != NULL) {
		_in_quote_enc_info = (sgx_target_info_t*)malloc(_len_quote_enc_info);
		if (_in_quote_enc_info == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_quote_enc_info, _tmp_quote_enc_info, _len_quote_enc_info);
	}
	if (_tmp_pouw_att != NULL) {
		if ((_in_pouw_att = (sgx_report_t*)malloc(_len_pouw_att)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_pouw_att, 0, _len_pouw_att);
	}
	if (_tmp_job_att != NULL) {
		if ((_in_job_att = (sgx_report_t*)malloc(_len_job_att)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_job_att, 0, _len_job_att);
	}
	if (_tmp_output != NULL) {
		if ((_in_output = (job_result*)malloc(_len_output)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_output, 0, _len_output);
	}
	ms->ms_retval = pouw_main(_in_work, _in_prev, _in_quote_enc_info, _in_pouw_att, _in_job_att, _in_output);
err:
	if (_in_work) free(_in_work);
	if (_in_prev) free(_in_prev);
	if (_in_quote_enc_info) free(_in_quote_enc_info);
	if (_in_pouw_att) {
		memcpy(_tmp_pouw_att, _in_pouw_att, _len_pouw_att);
		free(_in_pouw_att);
	}
	if (_in_job_att) {
		memcpy(_tmp_job_att, _in_job_att, _len_job_att);
		free(_in_job_att);
	}
	if (_in_output) {
		memcpy(_tmp_output, _in_output, _len_output);
		free(_in_output);
	}

	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv;} ecall_table[1];
} g_ecall_table = {
	1,
	{
		{(void*)(uintptr_t)sgx_pouw_main, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[1][1];
} g_dyn_entry_table = {
	1,
	{
		{0, },
	}
};


sgx_status_t SGX_CDECL ocall_print_string(int* retval, const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_string_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_string_t);
	void *__tmp = NULL;

	ocalloc_size += (str != NULL && sgx_is_within_enclave(str, _len_str)) ? _len_str : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_string_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_string_t));

	if (str != NULL && sgx_is_within_enclave(str, _len_str)) {
		ms->ms_str = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_str);
		memcpy((void*)ms->ms_str, str, _len_str);
	} else if (str == NULL) {
		ms->ms_str = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(0, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

