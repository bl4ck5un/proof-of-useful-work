//
// Created by fan on 6/9/16.
//

#ifndef POUW_PRINTF_H
#define POUW_PRINTF_H

#ifdef ENCLAVE_STD_ALT

#if defined(__cplusplus)
extern "C" {
#endif

int printf_sgx(const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#else // in Application
#include <stdio.h>
#define printf_sgx printf
#endif

#endif //POUW_PRINTF_H
