/* *************************** Self Tests ************************ */

/*
 * There are two set of mutually exclusive tests, based on SHA3_USE_KECCAK,
 * which is undefined in the production version.
 *
 * Known answer tests are from NIST SHA3 test vectors at
 * http://csrc.nist.gov/groups/ST/toolkit/examples.html
 *
 * SHA3-256:
 *   http://csrc.nist.gov/groups/ST/toolkit/documents/Examples/SHA3-256_Msg0.pdf
 *   http://csrc.nist.gov/groups/ST/toolkit/documents/Examples/SHA3-256_1600.pdf
 * SHA3-384:
 *   http://csrc.nist.gov/groups/ST/toolkit/documents/Examples/SHA3-384_1600.pdf
 * SHA3-512:
 *   http://csrc.nist.gov/groups/ST/toolkit/documents/Examples/SHA3-512_1600.pdf
 *
 * These are refered to as [FIPS 202] tests.
 *
 * -----
 *
 * A few Keccak algorithm tests (when M and not M||01 is hashed) are
 * added here. These are from http://keccak.noekeon.org/KeccakKAT-3.zip,
 * ShortMsgKAT_256.txt for sizes even to 8. There is also one test for
 * ExtremelyLongMsgKAT_256.txt.
 *
 * These will work with this code when SHA3_USE_KECCAK converts Finalize
 * to use "pure" Keccak algorithm.
 *
 *
 * These are referred to as [Keccak] test.
 *
 * -----
 *
 * In one case the input from [Keccak] test was used to test SHA3
 * implementation. In this case the calculated hash was compared with
 * the output of the sha3sum on Fedora Core 20 (which is Perl's based).
 *
 */

#include "Enclave_u.h"

#include <cstdint>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int sha3_test(sgx_enclave_id_t eid) {
    return 0;
}
