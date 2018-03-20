#include "miniz.h"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;

#include <sgx_tae_service.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

int zlib_compress(const char *s_pStr) {
    int cmp_status;
    uLong src_len = (uLong) strlen(s_pStr);
    uLong cmp_len = compressBound(src_len);
    uLong uncomp_len = src_len;
    uint8 *pCmp, *pUncomp;
    uint total_succeeded = 0;

    // Allocate buffers to hold compressed and uncompressed data.
    pCmp = (mz_uint8 *) malloc((size_t) cmp_len);
    pUncomp = (mz_uint8 *) malloc((size_t) src_len);
    if ((!pCmp) || (!pUncomp)) {
        printf("Out of memory!\n");
        return EXIT_FAILURE;
    }

    // Compress the string.
    cmp_status = compress(pCmp, &cmp_len, (const unsigned char *) s_pStr, src_len);
    if (cmp_status != Z_OK) {
        printf("compress() failed!\n");
        free(pCmp);
        free(pUncomp);
        return EXIT_FAILURE;
    }

    // Decompress.
    cmp_status = uncompress(pUncomp, &uncomp_len, pCmp, cmp_len);

    if (cmp_status != Z_OK) {
        printf("uncompress failed!\n");
        free(pCmp);
        free(pUncomp);
        return EXIT_FAILURE;
    }

    // Ensure uncompress() returned the expected data.
    if ((uncomp_len != src_len) || (memcmp(pUncomp, s_pStr, (size_t) src_len))) {
        printf("Decompression failed!\n");
        free(pCmp);
        free(pUncomp);
        return EXIT_FAILURE;
    }

    free(pCmp);
    free(pUncomp);

    return EXIT_SUCCESS;
}


int main() {
    int n_rounds = 100;
    {
        size_t file_size = 20*1024*1024;
        char* s = new char[file_size];
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
            zlib_compress(s);
        }
        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
        double time = double(microseconds.count()) / n_rounds;

        cerr << "zlib: " << time << endl;
    }
}
