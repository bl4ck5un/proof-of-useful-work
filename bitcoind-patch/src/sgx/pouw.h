//
// Created by fan on 6/21/16.
//
#include <string>

#ifndef POUW_SGX_H
#define POUW_SGX_H

#define TOKEN_FILENAME   "enclave.token"
#define ENCLAVE_FILENAME "enclave.signed.so"

using namespace std;

int pouw_guess(const unsigned char* hash, double win_prob, string& quote_b64);
bool pouw_attestation_verify(const string& quote_b64);

#define INTEL_RSA_PUBKEY \
"-----BEGIN PUBLIC KEY-----\r\n" \
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqXot4OZuphR8nudFrAFi\r\n" \
"aGxxkgma/Es/BA+tbeCTUR106AL1ENcWA4FX3K+E9BBL0/7X5rj5nIgX/R/1ubhk\r\n" \
"KWw9gfqPG3KeAtIdcv/uTO1yXv50vqaPvE1CRChvzdS/ZEBqQ5oVvLTPZ3VEicQj\r\n" \
"lytKgN9cLnxbwtuvLUK7eyRPfJW/ksddOzP8VBBniolYnRCD2jrMRZ8nBM2ZWYwn\r\n" \
"XnwYeOAHV+W9tOhAImwRwKF/95yAsVwd21ryHMJBcGH70qLagZ7Ttyt++qO/6+KA\r\n" \
"XJuKwZqjRlEtSEz8gZQeFfVYgcwSfo96oSMAzVr7V0L6HSDLRnpb6xxmbPdqNol4\r\n" \
"tQIDAQAB\r\n" \
"-----END PUBLIC KEY-----\r\n"

#endif //POUW_POUW_H
