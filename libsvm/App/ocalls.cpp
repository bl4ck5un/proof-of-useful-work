#include <stdio.h>

#include "ocalls.h"
#include "Enclave_u.h"
#include "Log.h"

int ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    int ret = fprintf(stderr, "%s", str);
    fflush(stdout);
    return ret;
}