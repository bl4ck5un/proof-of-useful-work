/*
 * Copyright (C) 2011-2016 Intel Corporation. All rights reserved.
 *
 */

#include "metadata.h"
#include "manage_metadata.h"
#include "enclave_creator_sign.h"
#include "util_st.h"
#include "../Common/Log.h"
#include "sgx_error.h"
#include "loader.h"
#include "parserfactory.h"
#include "elf_helper.h"
#include "measure.h"

static int load_enclave(BinParser *parser, metadata_t *metadata)
{
    CLoader* ploader = new CLoader(const_cast<uint8_t *>(parser->get_start_addr()), *parser);
    return ploader->load_enclave_ex(NULL, 0, metadata, NULL);
}

static bool get_enclave_info(BinParser *parser, bin_fmt_t *bf, uint64_t * meta_offset)
{
    uint64_t meta_rva = parser->get_metadata_offset();
    const uint8_t *base_addr = parser->get_start_addr();
    metadata_t *metadata = GET_PTR(metadata_t, base_addr, meta_rva);

    if(metadata->magic_num == METADATA_MAGIC)
        { LL_WARNING(ENCLAVE_ALREADY_SIGNED_ERROR); }

    *bf = parser->get_bin_format();
    *meta_offset = meta_rva;
    return true;
}

static bool measure_enclave(
        const uint8_t *elf,
        size_t elf_size,
        const xml_parameter_t *parameter,
        metadata_t *metadata,
        bin_fmt_t *bin_fmt,
        uint64_t *meta_offset,
        uint8_t *hash)
{
    assert(hash && metadata && bin_fmt && meta_offset);
    bool res = false;

    BinParser *parser = get_parser(elf, elf_size);
    if (!parser)
    {
        LL_CRITICAL("Can't get parser");
        return false;
    }
    sgx_status_t status = parser->run_parser();
    if (status != SGX_SUCCESS)
    {
        LL_CRITICAL(INVALID_ENCLAVE_ERROR);
        delete parser;
        return false;
    }

    // generate metadata
    CMetadata meta(metadata, parser);
    if(!meta.build_metadata(parameter))
        { return false; }

    // Collect enclave info
    if(!get_enclave_info(parser, bin_fmt, meta_offset))
        { return false; }

    if (*bin_fmt == BF_ELF64)
    {
        ElfHelper<64>::dump_textrels(parser);
    }
    else if (*bin_fmt == BF_ELF32)
    {
        ElfHelper<32>::dump_textrels(parser);
    }

    // Load enclave to get enclave hash
    status = (sgx_status_t) load_enclave(parser, metadata);

    switch(status)
    {
    case SGX_ERROR_INVALID_METADATA:
        LL_CRITICAL(OUT_OF_EPC_ERROR);
        res = false;
        break;
    case SGX_ERROR_INVALID_VERSION:
        LL_CRITICAL(META_VERSION_ERROR);
        res = false;
        break;
    case SGX_ERROR_INVALID_ENCLAVE:
        LL_CRITICAL(INVALID_ENCLAVE_ERROR);
        res = false;
        break;
    case SGX_SUCCESS:
        status = (sgx_status_t) dynamic_cast<EnclaveCreatorST*>(get_enclave_creator())->get_enclave_info(hash, SGX_HASH_SIZE);
        if(status != SGX_SUCCESS)
        {
            res = false;
            break;
        }
        res = true;
        break;
    default:
        res = false;
        break;
    }

    return res;
}


int measure(const uint8_t *elf, size_t elf_size,
            xml_parameter_t *enclave_launch_params, int enclave_launch_params_size,
            uint8_t *mr_enclave)
{
    if (!mr_enclave) { return -1; }

    metadata_t metadata;
    bin_fmt_t bin_fmt = BF_UNKNOWN;
    uint64_t meta_offset = 0;

    (void) enclave_launch_params_size;
    memset(&metadata, 0, sizeof(metadata));

    if (!measure_enclave(elf, elf_size,
                         enclave_launch_params,
                         &metadata,
                         &bin_fmt,
                         &meta_offset, mr_enclave))
    {
        LL_CRITICAL(OVERALL_ERROR);
        return -1;
    }

    return 0;
}
