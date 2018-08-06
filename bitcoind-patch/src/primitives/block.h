// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include "primitives/transaction.h"
#include "serialize.h"
#include "uint256.h"
#include "arith_uint256.h"
#include "util.h"
#include "sgx_uae_service.h"

#include "pouw.h"

#include <univalue.h>
#include <string>

using namespace std;

class CBlockHeader;

// Hard-coded ID of compliance checking enclave
// TODO: Hard-code it :)
const sgx_measurement_t COMPLIANCE_CODE_ID = {
    {162, 168, 146, 201, 124, 67, 220, 22, 204, 237, 181, 222, 216, 254, 223, 151, 247, 54, 143, 180, 248, 22, 255, 81, 244, 43, 145, 108, 165, 185, 2, 136} 
}; 

enum ValidationStatus {UNCHECKED, VERIFIED, BROKEN}; 

class CProofOfWork 
{
public: 
    /* Quote  */ 
    int16_t lenQuoteCompliance; 
    string quoteCompliance; 
    int16_t lenQuoteWinning; 
    string quoteWinning; 
    ValidationStatus validationStatus; // memory only
//    bool attestation_verified; // memory only

    CProofOfWork() {
        SetNull();
    }
    
    /**
     * Copy constructor
     */ 
    CProofOfWork(const CProofOfWork &obj) {
        quoteCompliance = obj.quoteCompliance; 
        quoteWinning = obj.quoteWinning; 
        this->validationStatus = obj.validationStatus;
    } 
    
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
//        LogPrintf("Before ================================= %s (%d)\n", quoteCompliance, quoteCompliance.size()); 
        lenQuoteCompliance = quoteCompliance.size(); // Reading or writing, now size match. 
        READWRITE(lenQuoteCompliance); // Either read it or write it 
        quoteCompliance.resize(lenQuoteCompliance); 
//        LogPrintf("================================= size now: %d\n", lenQuoteCompliance); 
        for(int i = 0 ; i < lenQuoteCompliance ; i++) { 
            READWRITE(quoteCompliance[i]); 
        }
//        LogPrintf("\n" ); 

//        LogPrintf("================================= %s (%d)\n", quoteWinning, quoteWinning.size()); 
        lenQuoteWinning = quoteWinning.size(); // Reading or writing, now size match. 
        READWRITE(lenQuoteWinning); // Either read it or write it 
        quoteWinning.resize(lenQuoteWinning); 
        for(int i = 0 ; i < lenQuoteWinning ; i++) { 
//            LogPrintf("================================= A %d\n", quoteCompliance.size());
//            LogPrintf("================================= B %d\n", quoteWinning.size());
            READWRITE(quoteWinning[i]); 
        }

//        LogPrintf("================================= (%d) %s\n", quoteCompliance.size(), quoteCompliance);
//        LogPrintf("================================= (%d) %s\n", quoteWinning.size(), quoteWinning); 
    }

    void SetNull() { 
        // LogPrintf("================================= SetNull\n");
        this->quoteCompliance.clear();
        this->quoteWinning.clear();
        this->validationStatus = UNCHECKED; 
    } 

    void SetGenesis() 
    {
        // LogPrintf("================================= SetGenesis\n");
        quoteCompliance = "genesis";
        quoteWinning = "genesis";
    }

    static double calcWinProb(uint32_t nBits) {
        arith_uint256 target; 
        arith_uint256 max256("0x");
        target.SetCompact(nBits); 
        max256.SetHex("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); 
        double probability = target.getdouble() / max256.getdouble(); 
        return probability;
    }
    
    void guess(uint256 headerWithoutPoWHash, uint32_t nBits);
    
    /**
     * Check that the pow is committed to the given hash and the 
     * difficulty matches nBits.
     */ 
    bool check(uint256 headerWithoutPoWHash, uint32_t nBits);  
    
    string ToString() const; 
    UniValue ToJSON() const; 
}; 

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    CProofOfWork pow; 

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits); 
        READWRITE(pow); 
    }

//     template <typename Stream, typename Operation>
//     inline void NoPoWSerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
//         READWRITE(this->nVersion);
//         nVersion = this->nVersion;
//         READWRITE(hashPrevBlock);
//         READWRITE(hashMerkleRoot);
//         READWRITE(nTime);
//         READWRITE(nBits);
//         READWRITE(pow);
//     }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        pow.SetNull(); 
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash(bool withPoW = true) const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransaction> vtx;

    // memory only
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.pow            = pow; 
        return block;
    }

    std::string ToString() const;
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

#endif // BITCOIN_PRIMITIVES_BLOCK_H
