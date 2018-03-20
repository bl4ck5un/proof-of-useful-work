# PoUW: Proof of Useful Work for Permissionless Consensus


**WARNING: THIS CODE IS PROVIDED AS IS WITHOUT ANY WARRANTY. THIS IS A RESEARCH PROTOTYPE AND IS NOT READY FOR PRODUCTION. IT MAY CONTAINS BUGS. WE TAKE ABSOLUTELY NO RESPOSIBILTY.**


## Abstract

Blockchains show promise as potential infrastructure for financial transaction systems. The security of blockchains today, however, relies critically on Proof-of-Work (PoW), which forces participants to waste computational resources. We present REM (Resource-Efficient Mining), a new blockchain mining framework that uses trusted hardware (Intel SGX). REM achieves security guarantees similar to PoW, but leverages the partially decentralized trust model inherent in SGX to achieve a fraction of the waste of PoW. Its key idea, Proof-of-Useful-Work (PoUW), involves miners providing trustworthy reporting on CPU cycles they devote to inherently useful workloads. REM flexibly allows any entity to create a useful workload. REM ensures the trustworthiness of these workloads by means of a novel scheme of hierarchical attestations that may be of independent interest.

This is the source code accompanying our paper [REM: Resource-Efficient Mining for Blockchains](https://eprint.iacr.org/2017/179).

