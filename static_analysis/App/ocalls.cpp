#include <iostream>
#include <string>
#include <ctime>
#include "VerifierEnclave_u.h"

#ifdef _WIN32
#include "windows.h"
#endif

#include "ocalls.h"
#include <Log.h>
#include <unistd.h>

int ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    int ret = fprintf(stderr, "%s", str);
    fflush(stderr);
    return ret;
}