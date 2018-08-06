#ifndef POUW_SGX_UTILS_H
#define POUW_SGX_UTILS_H

#include <vector>
#include "sgx_error.h"


void fromHex(const char* src, uint8_t* target, unsigned* len);
void fromHex(const char* src, std::vector<uint8_t> & out);

int initialize_enclave(const char*, sgx_enclave_id_t*);

#endif
