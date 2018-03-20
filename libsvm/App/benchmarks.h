//
// Created by fanz on 2/15/17.
//

#ifndef POUW_BENCHMARKS_H
#define POUW_BENCHMARKS_H


void
svm_test(sgx_enclave_id_t eid, FILE *input, struct svm_model *model, sgx_target_info_t *eq_info, sgx_report_t *report);
int sha3_test(sgx_enclave_id_t eid);

#endif //POUW_BENCHMARKS_H
