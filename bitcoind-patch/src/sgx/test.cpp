#include "pouw.h"
#include <iostream>

#include "base64.h"
#include "sgx_uae_service.h"

#include "Debug.h"

#include <string.h>

using namespace std;

int main(int argc, const char** argv)
{
    if (1 != argc)
    {
        cout << "Usage: " << argv[0] << " [attestation base64 encoded]\n";
    }
    string s(argv[1]);
    if (1 == pouw_attestation_verify(s))
        cout << "attestation verification succeeded.\n";
    else
        cout << "attestation verification failed.\n";

}
