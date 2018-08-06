#include "restclient-cpp/connection.h"
#include "restclient-cpp/restclient.h"

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <stdexcept>

#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/err.h>



using namespace std;

inline size_t calcDecodeLength(const char* b64input) { //Calculates the length of a decoded string
	size_t len = strlen(b64input),
		padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
		padding = 2;
	else if (b64input[len-1] == '=') //last char is =
		padding = 1;

	return (len*3)/4 - padding;
}

void Base64Decode(const char* b64message, unsigned char** buffer, size_t* length) { //Decodes a base64 encoded string
	BIO *bio, *b64;

	int decodeLen = calcDecodeLength(b64message);
	*buffer = (unsigned char*)malloc(decodeLen + 1);
	(*buffer)[decodeLen] = '\0';

	bio = BIO_new_mem_buf(b64message, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
	*length = BIO_read(bio, *buffer, strlen(b64message));

    BIO_free_all(bio);
    if (*length != decodeLen) {
        throw std::runtime_error("Fatal error in decoding signature");
    }
}

bool verify(const string& msg, const string& signature) {
    RSA* rsa;
    unsigned long err;
    int ret;
    unsigned char* sigbuf;
    size_t siglen;

    FILE* pem = fopen("/home/fan/dev/pouw/src/RK_PUB.PEM", "r");
    if (pem == NULL) {
        cout << "Can't open key file" << endl;
    }

    rsa = PEM_read_RSA_PUBKEY(pem, NULL, NULL, NULL);
    if (rsa == NULL) {
        err = ERR_get_error();
        cout << "Error: " << err << endl;
    }

    Base64Decode(signature.c_str(), &sigbuf, &siglen);

    unsigned char md[32];
    SHA256((unsigned char*) msg.c_str(), msg.length(), md);

    ret = RSA_verify(NID_sha256, (unsigned char*)md, 32, sigbuf, siglen, rsa);

    // free(sigbuf);

    if (ret != 1) {
        // err = ERR_get_error();
        ERR_print_errors_fp(stderr);
        // cout << "Error in RSA_verify: " << err << endl;
        return false;
    }

    return true;
}

string get_quote() {
    return "AQABAIMCAAAEAAAAAAAAAPCPp6rjdFLfMtaSVrmphfAAAAAAAAAAAAAAAAAAAAAAAwMCBAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABwAAAAAAAAAHAAAAAAAAADDmJLFC1SML7iVwuoaX0p4zG2UIIMX9K3qqbV9vBvdKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACD1xnnferKFHD2uvYqTXdDA8iZ22kCD5xw7h38CMfOngAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAouIdJ9nkTVZeKE9RsSzz5wWgvttnyuP6l/z8vf4CXQH8DAAAAAAAAAQAAAAAAAAAgHN9xan8AAHC/H0H8fwAAqAIAAFV0rHICfIauGPAFfxEXdaOaW29QTpBUYL7XmtjuFMyvWtlSNXIlAzHrHZq2jOVkdKSVX1x3yQAh8+WxVO/SqX99Lmb98EzZZdtsEU0do++J3kKzmAyTNfPj0+krH3FUDF+EFA+VMQ+y0rk+rWoBNl1XPCDlod9iZucZBH37IppXgF1sl2s9ykfMMMvK7oVy8ermumyrHXpbfcDH0QM1ZkXKDGxwUBp0eO+nWw4WCa1YsRAE/Kok5nZMvaxJk0bIp/g9AyxxCOW9hXuwTrKO8CJOyHfrT6khV5I+qekDlKjdWCcjL7G9HeqUs6ubnZJAnWW82DBM3NLaLaiXFUUmfiACM6s1jOxC6KFyaB+EYxnQdlmgiT6oalHbb1XOh9jjmb9Sf0c6cS1H2DzqT2gBAACpftvmVSxEaeHB9veeO70jHjAxczWfPOwieS+zkLrTKwA1QS5ZaoZ/mZ222lFKZniE4eJDOsGg5IO6ubV32rD7B3FVXggnG/rl3Ynkm07N1DbJoxNaFeCPL7jD8iDV9PJX/w2UP81FbBcUuBMUHoVNHroBc7C/f9iAqJZpHrM3q6y3P9/HfM6Eiv8aHylLcmuSO6MY29BooGmzs7Z5i8+tQwhl1nH7GLgO1VM66ohTMkVZj97KuJOkqzGgFmBrOoS/uWEFIWOAqDGfXWjTPAfIYohwiY4MqK/yPzA9HsrYvb0dniIqujjwzfDcihsqZ2K3ptqQoaYIDJ9zSQ0+wHW5rpeLUbnAa78SseOqAWhboDhnBdl804xUb7ipjEh3D9ZGhk+MjJbKKOHm1s+fedNTR8y3PuHtXU0HdkY4k3ekh9oN5JqgCr/YQZcFItU0ylixulAm9U2tXscDFAkL1AEpTMTvtBsG6K0UctpleCkKfmTrKHKaj4Si";
}

int main()
{
    string INTEL_URL = "https://test-as.sgx.trustedservices.intel.com:443";
    RestClient::init();

    RestClient::Connection* conn = new RestClient::Connection(INTEL_URL);

    conn->AppendHeader("Content-Type", "application/json");
    conn->SetCertPath("/home/fan/.ias/client.crt");
    conn->SetKeyPath("/home/fan/.ias/client.key");
    conn->SetCertType("PEM");

    const string& quote = get_quote();

    // sanity check
    // RestClient::Response r = conn->get("/attestation/sgx/v1/sigrl/00000283");
    //
    
    string att_evidence_payload = "{\"isvEnclaveQuote\": \"" + quote + "\"}";

    RestClient::Response r = conn->post("/attestation/sgx/v1/report", att_evidence_payload);

    cout << verify(r.body, r.headers["x-iasreport-signature"]) << endl;

    RestClient::disable();
    return 0;
}
