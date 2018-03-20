#include <iostream>
#include <stdlib.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <string>

#include <sgx_urts.h>
#include <sgx_uae_service.h>

#include "Enclave_u.h"
#include "blockchain.h"
#include "Debug.h"
#include "Log.h"
#include "Utils.h"
#include "svm_ocalls.h"
#include "base64.h"
#include "pouw_defs.h"
#include "benchmarks.h"

#include <atomic>
#include <csignal>

uint8_t SPID_FANZ[16] = {
        0xF0, 0x8F, 0xA7, 0xAA,
        0xE3, 0x74, 0x52, 0xDF,
        0x32, 0xD6, 0x92, 0x56,
        0xB9, 0xA9, 0x85, 0xF0,
};


typedef std::chrono::high_resolution_clock Clock;
namespace po = boost::program_options;
using namespace std;

sgx_enclave_id_t eid;

int main(int argc, const char *argv[]) {
    string enclave_path;

    po::options_description desc("Allowed options");
    try {
        desc.add_options()
                ("help", "produce this message")
                ("enclave", po::value(&enclave_path)->required(), "path to the enclave image");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            cerr << desc << endl;
            return -1;
        }

        po::notify(vm);
    }

    catch (po::required_option &e) {
        cerr << desc << endl;
        cerr << e.what() << endl;
        return -1;
    }
    catch (exception &e) {
        cerr << desc << endl;
        cerr << e.what() << endl;
        return -1;
    }
    catch (...) {
        cerr << "Unknown error!" << endl;
        return -1;
    }

    if (!boost::filesystem::exists(enclave_path)) {
        cerr << enclave_path << " is not a file" << endl;
        return -1;
    }

    int ret;

    ret = initialize_enclave(enclave_path.c_str(), &eid);
    if (ret != 0) {
        LL_CRITICAL("Exiting %d", ret);
        return ret;
    } else { LL_NOTICE("enclave %lu created", eid); }


    sgx_target_info_t quote_enc_info;
    sgx_epid_group_id_t p_gid;
    sgx_report_t report;

    ret = sgx_init_quote(&quote_enc_info, &p_gid);
    if (ret != SGX_SUCCESS) {
        print_error_message((sgx_status_t) ret);
        return -1;
    }

    int n_rounds = 100;

    {
        auto start = std::chrono::high_resolution_clock::now();

        for (auto j = 0; j < 10; j++) {
            protein_folding(eid);
        }

        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
        double time = double(microseconds.count()) / 10;

        cerr << "protein: " << time << endl;

    }

    {
        FILE *in_file;
        struct svm_model *model;
        string inputFilePath = "heart_scale";
        string modelFilePath = "heart_scale.model";

        in_file = fopen(inputFilePath.c_str(), "r");
        if (in_file == NULL) {
            LL_CRITICAL("can't open input file %s", inputFilePath.c_str());
            _exit(-1);
        }

        if ((model = svm_load_model(modelFilePath.c_str())) == 0) {
            fprintf(stderr, "can't open model file %s", modelFilePath.c_str());
            _exit(-1);
        }

        auto start = std::chrono::high_resolution_clock::now();

        for (auto j = 0; j < n_rounds; j++) {
            svm_test(eid, in_file, model, &quote_enc_info, &report);
            rewind(in_file);
        }

        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
        double time = double(microseconds.count()) / n_rounds;

        cerr << "svm: " << time << endl;

        svm_free_and_destroy_model(&model);
        fclose(in_file);
    }

    {
        size_t file_size = 20*1024*1024;
        char*s = new char[file_size];
        static const char alphanum[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < file_size; ++i) {
            s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        s[file_size] = 0;

        auto start = std::chrono::high_resolution_clock::now();
        for (auto j = 0; j < n_rounds; j++) {
            zlib_compress(eid, &ret, s, &quote_enc_info, &report);
        }
        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
        double time = double(microseconds.count()) / n_rounds;

        cerr << "zlib: " << time << endl;
        delete s;
    }

    {
        size_t file_size = 500*1024*1024; // Megabytes
        uint8_t* big_file = new uint8_t[file_size];
        static const char alphanum[] =
                "0123456789"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < file_size; ++i) {
            big_file[i] = (uint8_t) alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        auto start = std::chrono::high_resolution_clock::now();
        sha3_context c;
        sha3_Init256(eid, &c);
        sha3_Update(eid, &c, big_file, file_size);
        uint8_t hash[32];
        sha3_Finalize(eid, &c, hash);
        auto finish = std::chrono::high_resolution_clock::now();

        auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
        double time = double(microseconds.count());

        cerr << "sha3: " << time << endl;
        delete big_file;
    }

    sgx_destroy_enclave(eid);
    LL_NOTICE("%s", "all enclave closed successfully.");
    return ret;
}
