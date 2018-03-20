//
// Created by fanz on 11/11/16.
//

#ifndef POUW_SVM_OCALLS_H
#define POUW_SVM_OCALLS_H

#include <sgx_eid.h>
#include "svm.h"
#include "blockchain.h"
#include "sgx_report.h"

#if defined(__cplusplus)
extern "C" {
#endif

svm_model *svm_load_model(const char *model_file_name);

#if defined(__cplusplus)
}
#endif


static const char *svm_type_table[] = {
        "c_svc","nu_svc","one_class","epsilon_svr","nu_svr",NULL
};

static const char *kernel_type_table[]= {
        "linear","polynomial","rbf","sigmoid","precomputed",NULL
};

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

#endif //POUW_SVM_OCALLS_H
