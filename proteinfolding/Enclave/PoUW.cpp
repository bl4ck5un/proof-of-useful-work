//
// Created by fan on 6/9/16.
//

#include "Debug.h"
#include "pouw_defs.h"
#include "sgx_trts.h"
#include "sgx_tcrypto.h"
#include "Log.h"
#include "Enclave_t.h"
#include "sgx_utils.h"
#include "string.h"

#include <list>
#include <string>
#include <map>
#include <cmath>
#include <vector>

#include "algorithm/GeneticAlgorithm.h"

std::string SEQ20 = "10100110100101100101";
std::string SEQ24 = "110010010010010010010011";
std::string SEQ25 = "0010011000011000011000011";
std::string SEQ36 = "000110011000001111111001100001100100";
std::string SEQ48 = "001001100110000011111111110000001100110010011111";
std::string SEQ50 = "11010101011110100010001000010001000101111010101011";
std::string SEQ150 = "110101010111101000100110101010111101000100010000100010001011110101010111101010101111010001000100001000100010111101010101101000010001000101111010101011";
int TIMEOUT = 72;  // seconds
std::string SEQ156 = "000000000001001111111111100111111111110011111111111001111111111100111111111110011111111111001111111111100111111111110011111111111001111111111100100000000000";

GeneticAlgorithmParams GeneticAlgorithm::params = {
        SEQ150,  // sequence
        50,     // populationSize
        1000,     // generations
        0.10,   // elitePercent
        0.15,   // crossoverPercent
        0.15    // mutationPercent
};

#ifdef __cplusplus
extern "C" {
#endif

long pouw_main (pow_spec* spec, output_t* output);
void protain_folding();

#ifdef __cplusplus
}
#endif

void protain_folding () {
    GeneticAlgorithm geneticAlgorithm;
    geneticAlgorithm.run();
}

long pouw_main (pow_spec* spec, output_t* output) {
    int ret = 0;
    GeneticAlgorithm geneticAlgorithm;
    geneticAlgorithm.run();
    return ret;
}


