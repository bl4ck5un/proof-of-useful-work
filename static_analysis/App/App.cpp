#include "App.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>

#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "string.h"
#include <string>


#include "VerifierEnclave_u.h"
#include "Debug.h"
#include "metadata.h"
#include "Log.h"
#include "Utils.h"
#include "xml_parameter_t.h"
#include "xmlparser.h"

uint8_t SPID_FANZ[16] = {
        0xF0, 0x8F, 0xA7, 0xAA,
        0xE3, 0x74, 0x52, 0xDF,
        0x32, 0xD6, 0x92, 0x56,
        0xB9, 0xA9, 0x85, 0xF0,
};

#define CHECK_RET(x) do {\
    if (x != SGX_SUCCESS)  \
    { print_error_message(x); goto cleanup; } \
    } while (0);


#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

int main(int argc, const char* argv[])
{
    string enclave_path;
    string verifier_path;
    string enclave_config_path;

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help", "produce this message")
                ("verifier", po::value(&verifier_path)->required(), "path to the verifier image")
                ("enclave", po::value(&enclave_path)->required(), "path to the enclave image to be verified")
                ("config", po::value(&enclave_config_path)->required(), "path to the enclave launch config file");

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
        cerr << "Unknown ERROR!" << endl;
        return -1;
    }


    if (!boost::filesystem::exists(enclave_path)) {
        cerr << enclave_path << " is not a file" << endl;
    }

    if (!boost::filesystem::exists(verifier_path)) {
        cerr << enclave_path << " is not a file" << endl;
    }

    if (!boost::filesystem::exists(enclave_config_path)) {
        cerr << enclave_path << " is not a file" << endl;
    }


    sgx_status_t ret;
    sgx_enclave_id_t eid;
    uint8_t result = 0;
    sgx_target_info_t quote_enclave_info;
    sgx_epid_group_id_t gid;
    sgx_report_t report;
    sgx_spid_t spid;
    uint32_t quote_size;
    sgx_quote_t* quote = NULL;

    FILE* fp = NULL;
    struct stat st;
    long filesize = 0;
    unsigned char* buf = NULL;
    long bytesRead = 0;
    xml_parameter_t parameter[14] = {{"ProdID", 0xFFFF, 0, 0, 0},
                                   {"ISVSVN", 0xFFFF, 0, 0, 0},
                                   {"ReleaseType", 1, 0, 0, 0},
                                   {"IntelSigned", 1, 0, 0, 0},
                                   {"ProvisionKey",1,0,0,0},
                                   {"LaunchKey",1,0,0,0},
                                   {"DisableDebug",1,0,0,0},
                                   {"HW", 0x10,0,0,0},
                                   {"TCSNum",0xFFFFFFFF,TCS_NUM_MIN,1,0},
                                   {"TCSPolicy",TCS_POLICY_UNBIND,TCS_POLICY_BIND,TCS_POLICY_UNBIND,0},
                                   {"StackMaxSize",0xFFFFFFFF,STACK_SIZE_MIN,0x40000,0},
                                   {"HeapMaxSize",0xFFFFFFFF,HEAP_SIZE_MIN,0x100000,0},
                                   {"MiscSelect", 0xFFFFFFFF, 0, DEFAULT_MISC_SELECT, 0},
                                   {"MiscMask", 0xFFFFFFFF, 0, DEFAULT_MISC_MASK, 0}};

    size_t parameter_count = sizeof(parameter)/sizeof(parameter[0]);
    parse_metadata_file(enclave_config_path.c_str(), parameter, parameter_count);

    ret = (sgx_status_t) initialize_enclave(verifier_path.c_str(), &eid);
    CHECK_RET(ret);
    LL_NOTICE("enclave %#lx created", eid);

    fp = fopen(enclave_path.c_str(), "rb");
    if (fp == NULL) {
        LL_CRITICAL("Can not load pouw enclave %s", enclave_path.c_str());
        return -1;
    }

    if (0 != fstat(fileno(fp), &st))
    {
        LL_CRITICAL("Can stat file");
        return -1;
    }

    filesize = st.st_size;
    if (filesize < 0)
    {
        LL_CRITICAL("Can get filesize");
        return -1;
    }

    buf = (unsigned char*) malloc(filesize);
    if (buf == NULL)
    {
        LL_CRITICAL("file too big");
        fclose(fp);
        return -1;
    }

    bytesRead = fread(buf, 1, filesize, fp);
    if (bytesRead != filesize)
    {
        LL_CRITICAL("can not read file into mem");
        fclose(fp);
        return -1;
    }

    CHECK_RET(sgx_init_quote(&quote_enclave_info, &gid));


    CHECK_RET(ecall_analyze_user_enclave(eid, &ret,
                                         buf, filesize,
                                         &quote_enclave_info,
                                         parameter,
                                         parameter_count,
                                         &report, &result));
    CHECK_RET(ret);


    memcpy(spid.id, SPID_FANZ, sizeof spid.id);

    sgx_get_quote_size(NULL, &quote_size);
    quote = (sgx_quote_t*) malloc(quote_size);
    if (!quote)
    {
        LL_CRITICAL("failed to malloc for quote");
        goto cleanup;
    }

    ret = sgx_get_quote(&report, SGX_LINKABLE_SIGNATURE, &spid, NULL, NULL, 0, NULL, quote, quote_size);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        goto cleanup;
    }


    if (ret != SGX_SUCCESS)
    {
        print_error_message(ret);
    }
    if (result == 1) { LL_NOTICE("%s passed the compliance check", enclave_path.c_str()); }
    else { LL_CRITICAL("compliance check failed"); }

    dump_buf("Report Data", quote->report_body.report_data.d, sizeof (quote->report_body.report_data.d));
    dump_buf("Attestation", (unsigned char*) quote, sizeof (sgx_quote_t));

    for (int i = 0; i < quote_size; i++)
    {
        printf("%02x", ((uint8_t*) quote)[i]);
    }

cleanup:
    if (quote) free(quote);
    if (buf) free(buf);
    sgx_destroy_enclave(eid);
    return ret;
}
