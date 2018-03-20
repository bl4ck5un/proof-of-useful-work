#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

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
    string enclave_path_orig;
    string enclave_path_instrumented;

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help", "produce this message")
                ("enclave_1", po::value(&enclave_path_orig)->required(), "path to the enclave image")
                ("enclave_2", po::value(&enclave_path_instrumented)->required(), "path to the enclave image");

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

    // dummy values
    difficulty = .3;
    block_hash = "abababababababababababababababababababababababababababababababab";

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
        if (log_run_level > LOG_LVL_DEBUG)
            dump_buf("Hash: ", hash.h, sizeof hash.h);
    }

    if (!boost::filesystem::exists(enclave_path_orig)) {
        cerr << enclave_path_orig << " is not a file" << endl;
        return -1;
    }

    if (!boost::filesystem::exists(enclave_path_instrumented)) {
        cerr << enclave_path_instrumented << " is not a file" << endl;
        return -1;
    }

    int ret;
    sgx_enclave_id_t eid_orig, eid_instrumented;

    ret = initialize_enclave(enclave_path_orig.c_str(), &eid_orig);
    if (ret != 0)
    {
        LL_CRITICAL("Exiting %d", ret);
        return ret;
    }
    else { LL_NOTICE("enclave %lu created", eid_orig); }

    ret = initialize_enclave(enclave_path_instrumented.c_str(), &eid_instrumented);
    if (ret != 0)
    {
        LL_CRITICAL("Exiting %d", ret);
        return ret;
    } else { LL_NOTICE("enclave %lu created", eid_instrumented); }

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

    auto start = Clock::now();
    protain_folding(eid_orig);
    if (ret != SGX_SUCCESS) {
        print_error_message((sgx_status_t) ret);
        return -1;
    }
    cout << "Original programs (no stub) takes " << chrono::duration_cast<chrono::milliseconds>(Clock::now() - start).count()
         << " msec" << endl;

    start = Clock::now();
    run(eid_orig, &ret,
        &prob, &diff, &hash,
        &quote_enc_info, &report,
        &output);
    if (ret != SGX_SUCCESS) {
        print_error_message((sgx_status_t) ret);
        return -1;
    }
    cout << "Original programs takes " << chrono::duration_cast<chrono::milliseconds>(Clock::now() - start).count()
         << " msec" << endl;

    start = Clock::now();
    run(eid_instrumented, &ret,
        &prob, &diff, &hash,
        &quote_enc_info, &report,
        &output);
    if (ret != SGX_SUCCESS) {
        print_error_message((sgx_status_t) ret);
        return -1;
    }
    cout << "Instrumented programs takes " << chrono::duration_cast<chrono::milliseconds>(Clock::now() - start).count()
         << " msec" << endl;

exit:
    LL_NOTICE("%s", "all enclave closed successfully.");
    return ret;
}
