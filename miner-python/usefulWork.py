import subprocess as sp
from os.path import abspath
import os
import sys
from bitcoinrpc.authproxy import AuthServiceProxy, JSONRPCException
from bitcoinProxy import BitcoinProxy
from binascii import *

# Set up bitcoin direct connection:
rpcUsername = "user"
rpcPassword = "password"
rpcHost = "localhost"
rpcPort = 20011

rpcURL = "http://%s:%s@%s:%d" % (rpcUsername, rpcPassword, rpcHost, rpcPort)
connection = AuthServiceProxy(rpcURL)

# Print initial block coutn:
print("Block count: %d" % connection.getblockcount())

# Do useful work piece:
DO_ACTUAL_WORK = True

bitcoinProxy = BitcoinProxy()
task = bitcoinProxy.getTask()

headerPrefixHash = task["headerPrefixHash"]
difficulty = task["difficulty"]

pouw_attestation = static_analysis_attestation = None

if DO_ACTUAL_WORK:
    POUW_BINARY = abspath("pouw")
#    DIFFICULTY= .5
#    BLOCK_HASH = "ab"*32
    DIFFICULTY = 0.5
    BLOCK_HASH = hexlify(headerPrefixHash)
    POUW_ENCLAVE_PATH = abspath("protein.signed.so")

    STATIC_CHECKER_BINARY = abspath("verifier")
    STATIC_CHECKER_ENCLAVE = abspath("verifier.signed.so")
    ENCLAVE_CONFIG_PATH = abspath("../useful_work/Enclave/Enclave.config.xml")

    args = map(str, ["--difficulty", DIFFICULTY, "--hash", BLOCK_HASH, "--enclave", POUW_ENCLAVE_PATH])
    checker_args = map(str, ["--config", ENCLAVE_CONFIG_PATH, "--enclave", POUW_ENCLAVE_PATH, "--verifier", STATIC_CHECKER_ENCLAVE])

    for filePath in [POUW_BINARY, POUW_ENCLAVE_PATH, STATIC_CHECKER_BINARY, STATIC_CHECKER_ENCLAVE, ENCLAVE_CONFIG_PATH]:
        if not os.path.exists(filePath):
            raise Exception("Missing file %s. Please make entire project first." % filePath)

    with open(os.devnull, 'w') as devnull:
        try:
            pouw_attestation = sp.check_output([POUW_BINARY] + args, stderr=devnull)
#            print pouw_attestation
        except sp.CalledProcessError:
            print " ".join([POUW_BINARY] + args)

        try:
            static_analysis_attestation = sp.check_output([STATIC_CHECKER_BINARY] + checker_args, stderr=devnull)
#            print static_analysis_attestation
        except sp.CalledProcessError:
            print " ".join([STATIC_CHECKER_BINARY] + checker_args)
else: # Don't work (no SGX)
    pouw_attestation = "010001000b0000000100eeeeeeeeeeeef08fa7aae37452df32d69256b9a985f0000000000000000000000000000000004820f3376ae6b2f2034d3b7a4b48a7780000000000000000000000000000000000000000000000000000000000000000070000000000000007000000000000002fcfe2f4705633ce5bb200823f9d6ed1a3546248e2226d7d8b042be4242a9750000000000000000000000000000000000000000000000000000000000000000083d719e77deaca1470f6baf62a4d774303c899db69020f9c70ee1dfc08c7ce9e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000abababababababababababababababababababababababababababababababab000000000000e03f010000000000000000000000000000000000000000000000a8020000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee8ce5b9c5658e1012ea8c405868010000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee6e2b3975e1e0e5e51403a6e91ca250e7"
    static_analysis_attestation = "010001000b0000000100eeeeeeeeeeeef08fa7aae37452df32d69256b9a985f0000000000000000000000000000000004820f3376ae6b2f2034d3b7a4b48a7780000000000000000000000000000000000000000000000000000000000000000070000000000000007000000000000006a3920f1f239269a48564281de7b103f8e0f4ab8141995dc3b3fa6d6630f72b8000000000000000000000000000000000000000000000000000000000000000083d719e77deaca1470f6baf62a4d774303c899db69020f9c70ee1dfc08c7ce9e000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002fcfe2f4705633ce5bb200823f9d6ed1a3546248e2226d7d8b042be4242a97500000000000000000000000000000000000000000000000000000000000000000a8020000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeec206ff20a6b634ab52dc792368010000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee417a0c3247ffebd1bafaf0f947b15c0a"

bitcoinProxy.submitResult(static_analysis_attestation, pouw_attestation)
# bitcoinProxy.submitResult("Testing testing", "Testing")
print("Submitted.")

# Print final block coutn:
print("Block count: %d" % connection.getblockcount())

