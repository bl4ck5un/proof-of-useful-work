#!/usr/bin/python

import struct
import sys
import json
from binascii import * 
from struct import pack 
from decimal import Decimal 
import base64

from bitcoinrpc.authproxy import AuthServiceProxy, JSONRPCException
from blkmaker import _dblsha256
import blktemplate

def difficultyFromTarget(target): 
    max256 = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
    
#    nbits = int(bits, 16)
#    nSize = nbits >> 24 
#    nWord = nbits & 0x007fffff 
#    print("DEBUG: nbits = " + hex(nbits)) 
#    print("DEBUG: nSize = " + hex(nSize)) 
#    print("DEBUG: nWord = " + hex(nWord)) 
#    if (nSize <= 3): 
#        nWord >>= 8 * (3 - nSize);
#        target = nWord;
#    else: 
#        target = nWord;
#        target <<= 8 * (nSize - 3) 

    probability = float(int(target, 16)) / max256; 

    return probability 

class BitcoinProxy: 
    def __init__(self): 
        self.rpcUsername = "user" 
        self.rpcPassword = "password" 
        self.rpcHost = "localhost" 
        self.rpcPort = 20011 

        rpcURL = "http://%s:%s@%s:%d" % (self.rpcUsername, self.rpcPassword, self.rpcHost, self.rpcPort) 
        self.connection = AuthServiceProxy(rpcURL) 

    def getTask(self): 
        # Get capability list for our template handler: 
        self.blockTemplate = blktemplate.Template() 
        capabilityList = list(self.blockTemplate.request()["params"][0]["capabilities"]) 

        # Get template from bitcoind with our capabilities, and an arbitrary coinbase: 
        dictBlockTemplate = self.connection.getblocktemplate({"capabilities" : capabilityList}
        ) 

        coinbase = a2b_hex("01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff1302955d0f00456c6967697573005047dc66085fffffffff02fff1052a010000001976a9144ebeb1cd26d6227635828d60d3e0ed7d0da248fb88ac01000000000000001976a9147c866aee1fa2f3b3d5effad576df3dbf1f07475588ac00000000") 

        extradata = str(dictBlockTemplate["curtime"]) # b'my block'
        # The following can be done better in both Python 2 and Python 3, but this way works with both
        origLen = ord(coinbase[41:42])
        newLen = origLen + len(extradata)
        coinbase = coinbase[0:41] + chr(newLen).encode('ascii') + coinbase[42:42 + origLen] + extradata + coinbase[42 + origLen:]
        
        dictBlockTemplate["coinbasetxn"] = {"data" : b2a_hex(coinbase)}

        # 
        self.blockTemplate.add(dictBlockTemplate)
        
        self.headerPrefix, self.reqID = self.blockTemplate.get_data() 
        self.headerPrefixHash = _dblsha256(self.headerPrefix)
        self.difficulty = difficultyFromTarget(dictBlockTemplate["target"]) 
        return {"headerPrefixHash" : self.headerPrefixHash, 
                "reqID" : self.reqID, 
                "difficulty" : self.difficulty} 
 
    def submitResult(self, b16QuoteCompliance, b16QuoteWinning): 
        quoteCompliance = unhexlify(b16QuoteCompliance)
        quoteWinning = unhexlify(b16QuoteWinning)

#        print("quoteCompliance: " + base64.b64encode(quoteCompliance)) 
#        print("quoteWinning   : " + base64.b64encode(quoteWinning)) 

        lenQuoteCompliance = pack("<H", len(quoteCompliance)) 
        lenQuoteWinning = pack("<H", len(quoteWinning)) 

        result = lenQuoteCompliance + quoteCompliance + lenQuoteWinning + quoteWinning
        submission = self.blockTemplate.submit(self.headerPrefix, self.reqID, result) 
#        print(json.dumps(submission, indent=4))
#        print submission["params"][0]
        self.connection.submitblock(submission["params"][0]) 

if __name__ == "__main__": 
    bitcoinProxy = BitcoinProxy() 
    bitcoinProxy.getTask() 
    bitcoinProxy.submitResult("0" * 256)

