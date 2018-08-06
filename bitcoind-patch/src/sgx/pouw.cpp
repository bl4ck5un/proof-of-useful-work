#include "pouw.h"

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <unistd.h>

#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "Enclave_u.h"
#include "string.h"

#include "Log.h"
#include "Utils.h"
#include "base64.h"
#include "pouw_defs.h"
#include "Error.h"
#include "Debug.h"

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

uint8_t SPID_FANZ[16] = {
        0xF0, 0x8F, 0xA7, 0xAA,
        0xE3, 0x74, 0x52, 0xDF,
        0x32, 0xD6, 0x92, 0x56,
        0xB9, 0xA9, 0x85, 0xF0,
};


inline size_t calcDecodeLength(char* b64input) { //Calculates the length of a decoded string
    size_t len = strlen((char*)b64input),
        padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
		padding = 2;
	else if (b64input[len-1] == '=') //last char is =
		padding = 1;

	return (len*3)/4 - padding;
}

void Base64Decode(char *input, char** output, size_t* length) {
    BIO *bio, *b64;

    int decodeLen = calcDecodeLength(input);
    *output= (char*)malloc(decodeLen + 1);
	(*output)[decodeLen] = '\0';
	
	bio = BIO_new_mem_buf(input, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
	*length = BIO_read(bio, *output, strlen(input));

    BIO_free_all(bio);
    if (*length == 0) {
        throw std::runtime_error("Empty signature!");
    }
    if (*length != decodeLen) {
        throw std::runtime_error("Fatal error in decoding signature");
    }
}

bool rsa_sig_verify(const string& msg, const string& signature) {
    int ret = 0;
    bool result = false;
    size_t siglen = 0;
    char *sigbuf = NULL, *sig = NULL;
    BIO* bio = NULL;
    RSA* rsa = NULL;
    unsigned char md[32] = {0};
    string decoded_sig;

    ERR_load_CRYPTO_strings();

    char* intel_rsa_pubkey = (char*) malloc(1 + strlen(INTEL_RSA_PUBKEY)); 
    if (!intel_rsa_pubkey)
    {
        LL_CRITICAL("Can't alloc memory for intel_rsa_pubkey");
        result = false;
        goto cleanup;
    }
    strcpy(intel_rsa_pubkey, INTEL_RSA_PUBKEY); 

    bio = BIO_new_mem_buf((void*) intel_rsa_pubkey, -1);
    rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
    if (rsa == NULL) {
        ERR_print_errors_fp(stderr);
        result = false;
        goto cleanup;
    }

    /*
     * alternatively, use the embedded base64 implementation
    decoded_sig = base64_decode(signature);
    sigbuf = (const unsigned char*) decoded_sig.c_str();
    siglen = decoded_sig.length();
    */

    sig = (char*) malloc(signature.length() + 1);
    if (!sig)
    {
        LL_CRITICAL("Can't alloc memory for sig");
        result = false;
        goto cleanup;
    }
    strcpy(sig, signature.c_str());

    try 
    {
        Base64Decode(sig, &sigbuf, &siglen);
    }
    catch (std::runtime_error e)
    {
        LL_CRITICAL("%s", e.what());
        result = false;
        goto cleanup;
    }

    SHA256((unsigned char*) msg.c_str(), msg.length(), md);

    ret = RSA_verify(NID_sha256, md, 32, (const unsigned char*)sigbuf, siglen, rsa);
    if (ret != 1) {
        ERR_print_errors_fp(stderr);
        LL_CRITICAL("Error in RSA_verify");
        result = false;
        goto cleanup;
    }

    result = true;

cleanup:
    if (intel_rsa_pubkey) free(intel_rsa_pubkey);
    if (rsa) free(rsa);
    if (bio) BIO_free(bio);
    ERR_free_strings();
    return result;
}

int pouw_guess(const unsigned char* hash, double win_prob, std::string& quote_str) 
{
    int ret;
    sgx_enclave_id_t eid;
    sgx_status_t sret;
	sgx_launch_token_t token = {0};
	int updated = 0;


    // ret = initialize_enclave(ENCLAVE_FILENAME, &eid);

 
	// Create the Enclave with above launch token.
create_enc:
	sret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);

    if (sret == SGX_ERROR_SERVICE_UNAVAILABLE) {
        sleep(1);
        goto create_enc;
    }
    if (sret != SGX_SUCCESS) {
		LL_CRITICAL("Can't create enclave: %s (%d)", sgx_error_message(sret), sret);
		return -1;
	}
    LL_NOTICE("enclave %lu created", eid);

    // prepare pow_spec
    pow_spec prob;
    strcpy((char*) prob.prefix, "PREFIX@@PREFIX@@");
    memset(prob.target, 0xff, 32);
    prob.target[0] = 0x11;
    prob.target[1] = 0x11;

    // prepare hash
    current_hash cur_hash;
    memset(cur_hash.h, 0, sizeof(cur_hash));
    memcpy(cur_hash.h, hash, sizeof(cur_hash));

    // prepare for report
    sgx_target_info_t quote_enc_info;
    sgx_epid_group_id_t p_gid;
    sgx_report_t pouw_report;
    sgx_report_t job_report;
    job_result output;

    sgx_init_quote( &quote_enc_info, &p_gid);

    // TODO move win_prob out of prob
    prob.prob = win_prob;
    sret = pouw_main(eid, &ret, &prob, &cur_hash, &quote_enc_info, &pouw_report, &job_report, &output);

    if (sret != SGX_SUCCESS) {
        LL_CRITICAL("ECALL failed with %d: %s", sret, sgx_error_message(sret));
        return -1;
    }

    if (ret != 0) {
        LL_CRITICAL("pouw_main returned %d", ret);
        return -1;
    }

    sgx_spid_t spid;
    memcpy(spid.id, SPID_FANZ, 16);

    uint32_t quote_size;
    sgx_get_quote_size(NULL, &quote_size);

    sgx_quote_t* quote = (sgx_quote_t*) malloc(quote_size);
    if (!quote) {
        LL_CRITICAL("%s", "failed to malloc");
        return -1;
    }

    sret = sgx_get_quote(&pouw_report, SGX_LINKABLE_SIGNATURE, &spid, NULL, NULL, 0, NULL, quote, quote_size);
    if (sret != SGX_SUCCESS)
    {
        LL_CRITICAL("sgx_get_quote returned %d: %s", ret, sgx_error_message(sret));
        return -1;
    }

    /*
    blockchain_comm* comm = (blockchain_comm*) quote->report_body.report_data.d;
    dump_buf("data:", quote->report_body.report_data.d, sizeof(quote->report_body.report_data.d));
    dump_buf("comm:", (unsigned char*) comm, sizeof(blockchain_comm));
    dump_buf("comm->current_hash: ", comm->header_hash, sizeof(comm->header_hash));
    cout << "comm->difficulty: " << comm->difficulty << endl;
    printf_sgx("comm->is_win: %d\n", comm->is_win);
    */

    quote_str = base64_encode((unsigned char*) quote, quote_size);

    // TODO dont forget another quote 
    if(SGX_SUCCESS != sgx_destroy_enclave(eid)) {
        LL_CRITICAL("failed to destroy enclave %lu", eid);
        return -1;
    }
    LL_NOTICE("enclave %lu closed", eid);

    free(quote); 
    return 0;
}

bool pouw_attestation_verify(const string& quote_b64)
{
    if (quote_b64.empty()) {
        LL_CRITICAL("Empty quote");
        return false;
    }
    string INTEL_URL = "https://test-as.sgx.trustedservices.intel.com:443";
    bool ret;
    RestClient::init();

    RestClient::Connection* conn = new RestClient::Connection(INTEL_URL);

    ifstream certFile("/var/keys/ias.crt");
    if (! certFile.good()) {
	std::cerr << "can't open cert file" << std::endl;
    }

    ifstream keyFile("/var/keys/ias.key");
    if (! keyFile.good()) {
	std::cerr << "can't open key file" << std::endl;
    }

    conn->AppendHeader("Content-Type", "application/json");
    conn->SetCertPath("/var/keys/ias.crt");
    conn->SetKeyPath("/var/keys/ias.key");
    conn->SetCertType("PEM");

    string att_evidence_payload = "{\"isvEnclaveQuote\": \"" + quote_b64 + "\"}";

    RestClient::Response r = conn->post("/attestation/sgx/v1/report", att_evidence_payload);
    if (r.code != 200 && r.code != 201) 
    {
        std::cout << "Intel Attestation Server returned " << r.code << std::endl;
        std::cout << "Intel Attestation Server returned " << r.body << std::endl;
        return -1;
    } 

    ret = rsa_sig_verify(r.body, r.headers["x-iasreport-signature"]);
    RestClient::disable(); 
    return ret;
} 

