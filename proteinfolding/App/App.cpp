#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "blockchain.h"
#include "App.h"
#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "Enclave_u.h"
#include "string.h"
#include "Debug.h"
#include "Log.h"
#include "Utils.h"


#include "base64.h"

#include "pouw_defs.h"

uint8_t SPID_FANZ[16] = {
        0xF0, 0x8F, 0xA7, 0xAA,
        0xE3, 0x74, 0x52, 0xDF,
        0x32, 0xD6, 0x92, 0x56,
        0xB9, 0xA9, 0x85, 0xF0,
};

#include <string>

namespace po = boost::program_options;
using namespace std;

int main(int argc, const char *argv[]) {
    double difficulty;
    string block_hash;
    string enclave_path;

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help", "produce this message")
                ("difficulty", po::value(&difficulty)->required(), "current difficulty")
                ("enclave", po::value(&enclave_path)->required(), "path to the enclave image")
                ("hash", po::value(&block_hash)->required(), "block hash without nonce");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            cerr << desc << endl;
            return -1;
        }

        po::notify(vm);
    }

    catch (po::required_option &e) {
        cerr << e.what() << endl;
        return -1;
    }
    catch (exception &e) {
        cerr << e.what() << endl;
        return -1;
    }
    catch (...) {
        cerr << "Unknown error!" << endl;
        return -1;
    }


    if (difficulty < 0 || difficulty > 1) {
        cerr << "difficulty is wrong: " << difficulty << endl;
        return -1;
    }

    if (block_hash.length() != 64) {
        cerr << "please supply SHA-256 (without 0x)" << endl;
        return -1;
    }

    uint32_t len;
    block_hash_t hash;
    if (BLOCK_HASH_LEN != fromHex(block_hash.c_str(), hash.h)) {
        LL_CRITICAL("Error in reading hash");
        return -1;
    }
    else
    {
        dump_buf("Hash: ", hash.h, sizeof hash.h);
    }

    if (!boost::filesystem::exists(enclave_path)) {
        cerr << enclave_path << " is not a file" << endl;
    }


    int ret;
    sgx_enclave_id_t eid;

    ret = initialize_enclave(enclave_path.c_str(), &eid);
    if (ret != 0) {
        LL_CRITICAL("Exiting %d", ret);
        return ret;
    } else {
        LL_NOTICE("enclave %lu created", eid);
    }

    pow_spec prob;
    strcpy((char *) prob.prefix, "PREFIX@@PREFIX@@");
    memset(prob.target, 0xff, 32);

    // difficulty
    prob.target[0] = 0x11;
    prob.target[1] = 0x11;

    sgx_target_info_t quote_enc_info;
    sgx_epid_group_id_t p_gid;
    sgx_report_t report;

    ret = sgx_init_quote(&quote_enc_info, &p_gid);
    if (ret != SGX_SUCCESS) {
        print_error_message((sgx_status_t) ret);
        return -1;
    }

    output_t output;

    difficulty_t diff;
    diff.difficulty = difficulty;
    run(eid, &ret,
        &prob, &diff, &hash,
        &quote_enc_info, &report,
        &output);
    if (ret != SGX_SUCCESS) {
        print_error_message((sgx_status_t) ret);
        return -1;
    }

    sgx_spid_t spid;
    memcpy(spid.id, SPID_FANZ, 16);

    uint32_t quote_size;
    sgx_get_quote_size(NULL, &quote_size);

    sgx_quote_t *quote = (sgx_quote_t *) malloc(quote_size);
    if (!quote) {
        LL_CRITICAL("%s", "failed to malloc");
        return -1;
    }
    ret = sgx_get_quote(&report, SGX_LINKABLE_SIGNATURE, &spid, NULL, NULL, 0, NULL, quote, quote_size);
    if (ret != SGX_SUCCESS) {
        print_error_message((sgx_status_t) ret);
        LL_CRITICAL("sgx_get_quote returned %d", ret);
        return -1;
    }

    dump_buf("measurement: ", quote->report_body.mr_enclave.m, sizeof(sgx_measurement_t));
    pouw_voucher *p_voucher = (pouw_voucher *) quote->report_body.report_data.d;
    LL_NOTICE("difficulty: %f", p_voucher->difficulty);
    LL_NOTICE("is_win: %d", p_voucher->is_win);
    dump_buf("block_hash", p_voucher->header_hash, 32);
    dump_buf("attestation: ", (const unsigned char *) quote, sizeof(sgx_quote_t));

    for (int i = 0; i < quote_size; i++)
    {
        fprintf(stdout, "%02x", ((const uint8_t*)quote)[i]);
    }

    exit:
    LL_NOTICE("%s", "all enclave closed successfully.");
    return ret;
}
