// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"

#include <sys/time.h>

#include "pouw.h"
#include "sgx_uae_service.h"
#include "base64.h"
#include "pouw_defs.h"

#include "util.h"

#include "Debug.h"

#include <iostream>

using namespace std;

std::string CProofOfWork::ToString() const 
{
    std::ostringstream stringStream;
    stringStream << "QuoteCompliance: " << quoteCompliance << "\n";
    stringStream << "QuoteWinning: " << quoteWinning << "\n";

    return stringStream.str(); 
} 

UniValue CProofOfWork::ToJSON() const 
{ 
    UniValue result(UniValue::VOBJ);
    result.push_back(this->quoteCompliance);
    result.push_back(this->quoteWinning);
    return result; 
} 

static long get_time_msec() {
    struct timeval t;
    gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000.0;
}

void CProofOfWork::guess(uint256 headerWithoutPoWHash, uint32_t nBits) 
{
    // TODO: DEBUG: This is broken. Mining shall be done outside Bitcoin henceforth. 
    long start = get_time_msec();
    int ret = pouw_guess(headerWithoutPoWHash.begin(), calcWinProb(nBits), quoteWinning);
    long finish = get_time_msec();

    LogPrintf("POUW: pouw_guess took %ld msec\n", finish - start);	

    if (ret != 0) {
        LogPrintf("POUW: pouw_guess returned %d", ret);;
        return;
    }
}

bool CProofOfWork::check(uint256 headerWithoutPoWHash, uint32_t nBits) { 
    LogPrintf("CProofOfWork::check starting...\n"); 

    if (quoteCompliance == "genesis" && quoteWinning == "genesis")
    {
        // TODO: FIXME: the whole genesis block is bullshit right now
        return true;
    }

//    string quoteCompliance_bin = base64_decode(quoteCompliance);
    sgx_quote_t* pQuoteCompliance = (sgx_quote_t*) quoteCompliance.c_str();
//    string quoteWinning_bin = base64_decode(quoteWinning);
    sgx_quote_t* pQuoteWinning = (sgx_quote_t*) quoteWinning.c_str();
    
    ///////////////////////////////////////////////////////////////////////////
    // Validate compliance: 
    
    // Compliance quote report data (512b total): 
    // 1. sgx_measurement_t (256b): ID of work code 
    // 2. uint8_t (8b): success or failure (shuold be 1) 

    // Compare code and compliance check hashes to make sure correct work is signed: 
    sgx_measurement_t complianceCodeIDActual = pQuoteCompliance->report_body.mr_enclave; 
    sgx_measurement_t workCodeIDActual = pQuoteWinning->report_body.mr_enclave; 
    sgx_measurement_t workCodeIDAttested; 
    memcpy(&workCodeIDAttested, &(pQuoteCompliance->report_body.report_data), sizeof(workCodeIDAttested)); 

    // Assert compliance code is what it should be: 
    if (!(0 == memcmp(&complianceCodeIDActual, &COMPLIANCE_CODE_ID, sizeof(COMPLIANCE_CODE_ID)))) {
        LogPrintf("ERROR Compliance code wrong.\n");
        LogPrintf("ERROR Actual:  \n"); 
        for (int i = 0 ; i < SGX_HASH_SIZE ; i++) {         
            LogPrintf("%d, ", complianceCodeIDActual.m[i]); 
        } 
        LogPrintf("\n"); 

        LogPrintf("But should be: \n"); 
        for (int i = 0 ; i < SGX_HASH_SIZE ; i++) {         
            LogPrintf("%d, ", COMPLIANCE_CODE_ID.m[i]); 
        } 
        LogPrintf("\n"); 

        // Removed for DEBUG        
        // return false;
    } else { 
        LogPrintf("DEBUG: VVV Compliance code measurement correct.\n"); 
    } 

    // Assert correct code was verified: 
    if (!(0 == memcmp(&workCodeIDActual, &workCodeIDAttested, sizeof(workCodeIDAttested)))) {
        LogPrintf("ERROR Compliance check for wrong code.\n"); 
        LogPrintf("ERROR Actual: \n"); 
        for (int i = 0 ; i < SGX_HASH_SIZE ; i++) {         
            LogPrintf("%d, ", workCodeIDActual.m[i]); 
        } 
        LogPrintf("\n"); 

        LogPrintf("But compliance check was for: \n"); 
        for (int i = 0 ; i < SGX_HASH_SIZE ; i++) {         
            LogPrintf("%d, ", workCodeIDAttested.m[i]); 
        } 
        LogPrintf("\n"); 

        // Removed for DEBUG        
        // return false;
    } else { 
        LogPrintf("DEBUG: VVV Compliance code checked correct code.\n"); 
    } 
    // Assert return value was 1 for success: 
    assert(sizeof(workCodeIDAttested) < SGX_REPORT_DATA_SIZE); 
    if (!(pQuoteCompliance->report_body.report_data.d[sizeof(workCodeIDAttested)] != 1)) { 
        LogPrintf("ERROR Compliance check failed (!=1).\n"); 
        // Removed for DEBUG        
        // return false;
    } else { 
        LogPrintf("DEBUG: VVV Compliance verification answer is 1, as expected.\n"); 
    }

    ///////////////////////////////////////////////////////////////////////////
    // Validate win: 

    // Winning quote report data (512b total) 
    // 1. uint8_t[32] (256b): header_hash; 
    // 2. double (): difficulty;
    // 3. uint8_t (8b): is_win; (should be 1)

    double probability = calcWinProb(nBits); 
    
    // TODO: Verify standard double size somewhere else. 
    // From http://stackoverflow.com/questions/752309/ensuring-c-doubles-are-64-bits 
    assert(std::numeric_limits< double >::is_iec559); 
    blockchain_comm* comm = (blockchain_comm*) pQuoteWinning->report_body.report_data.d;

    // is_win == 1
    if (comm->is_win != 1) {
        LogPrintf("ERROR: POUW: Winning is_win check failed (%d != 1)! \n", comm->is_win); 
        // Removed for DEBUG        
        // return false;
    } else { 
        LogPrintf("DEBUG: VVV Work enclave answer is 1, as expected.\n"); 
    }

    // hash matches 
/*    unsigned char chHeaderWithotuPoWHash[32]; 
    for (i = 0 ; i < 32 ; i++) { 
        chHeaderWithotuPoWHash[i] = (headerWithoutPoWHash >> i*8) & 0xff; 
    } 
*/

    if (!(0 == memcmp(comm->header_hash, headerWithoutPoWHash.begin(), headerWithoutPoWHash.size()))) {
        LogPrintf("ERROR: POUW: hash check failed\n"); 

        LogPrintf("Work for hash: "); 
        for (int i = 0 ; i < 32 ; i++) {         
            LogPrintf("%d, ", comm->header_hash[i]); 
        } 
        LogPrintf("\n");
        LogPrintf("Should be    : "); 
        char* asArray = (char*)headerWithoutPoWHash.begin(); 
        for (int i = 0 ; i < 32 ; i++) {         
            LogPrintf("%d, ", asArray[i]); 
        } 
        LogPrintf("\n");         // Removed for DEBUG 

        // Removed for DEBUG 
        // return false; 
    } else { 
        LogPrintf("DEBUG: VVV Hash matches header prefix hash..\n"); 
    }

    // difficulty matches
    if ( ! probability == comm->difficulty ) { 
        LogPrintf("ERROR: POUW: difficulty (%f vs %f) check failed\n", probability, comm->difficulty);
        // Removed for DEBUG        
        // return false;
    } else { 
        LogPrintf("DEBUG: VVV Difficulty check ok.\n"); 
    }
    // only contact Intel IAS once for one PoW
//    LogPrintf("ERROR: POUW: Not contacting Intel for checking attestations.\n"); 
//    return true; // DEBUG
    if (this->validationStatus == UNCHECKED) { 
//        bool complianceVerified = pouw_attestation_verify(this->quoteCompliance); 
//        bool winningVerified = pouw_attestation_verify(this->quoteWinning); 
        string b64quoteCompliance = base64_encode((const unsigned char*)this->quoteCompliance.c_str(), this->quoteCompliance.size()); 
        string b64quoteWinning = base64_encode((const unsigned char*)this->quoteWinning.c_str(), this->quoteCompliance.size()); 
        bool complianceVerified = pouw_attestation_verify(b64quoteCompliance); 
        bool winningVerified = pouw_attestation_verify(b64quoteWinning); 
//        LogPrintf("===== quoteCompliance: %s\n", b64quoteCompliance);
//        LogPrintf("===== quoteWinning: %s\n", b64quoteWinning);
        if (complianceVerified && winningVerified) { 
//            this->attestation_verified = complianceVerified && winningVerified; 
            this->validationStatus = VERIFIED; 
            LogPrintf("DEBUG: VVV IAS Validated quotes.\n");
        } else { 
            this->validationStatus = BROKEN; 
            if (!complianceVerified) {
                LogPrintf("POUW: Compliance check failed\n");
                LogPrintf("POUW: att=%s\n", this->quoteCompliance.c_str());
            }
            if (!winningVerified) {
                LogPrintf("POUW: Compliance check failed\n");
                LogPrintf("POUW: att=%s\n", this->quoteWinning.c_str());
            }
            // Removed for DEBUG
            // return false;
        } 

    }

    return true;
} 

uint256 CBlockHeader::GetHash(bool withPoW) const
{
    if (withPoW) {
        return SerializeHash(*this); 
    } else { // Hash without the PoW, take only the 76 bytes before the PoW field starts: 
        return SerializeHash(*this, SER_GETHASH, PROTOCOL_VERSION, 76); 
    } 
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, vtx=%u, pow=%s)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, 
        vtx.size(), 
        pow.ToString() ); 
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        s << "  " << vtx[i].ToString() << "\n";
    }
    return s.str();
}

