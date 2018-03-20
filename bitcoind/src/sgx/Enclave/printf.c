//
// Created by fan on 6/9/16.
//

#include "Enclave_t.h"
#include <stdio.h>
#include "printf.h"

#if defined(__cplusplus)
extern "C" {
#endif

int printf_sgx(const char *fmt, ...) {
    int ret;
    va_list ap;
    char buf[BUFSIZ] = {'\0'};
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

    ocall_print_string(&ret, buf);
    return ret;
}

#if defined(__cplusplus)
}
#endif