//
// Created by fanz on 11/3/16.
//

#include "rand_hardware.h"
#include "Log.h"

#include <stdint.h>
#include <sgx_trts.h>

#include <limits>
#include <math.h>

double DOUBLE_MAX = std::numeric_limits<double>::max();
double DOUBLE_MIN = std::numeric_limits<double>::min();

uint32_t rand()
{
    uint32_t r = 0;
    sgx_read_rand((unsigned char*)&r, sizeof (r));
    return r;
}

inline double rand_uniform(int a, int b)
{
    uint8_t rand;
    double r = 0;
    sgx_read_rand((unsigned char*)&rand, sizeof(rand));

    // shrink r to [0,1]
    r = (double)rand / UINT8_MAX;

    return r*(b-a) + a;
}

int rand_int_uniform(int a, int b)
{
    double r = rand_uniform(a, b);
    return (int)round(r);
}

double rand_exp(double lambda)
{
    uint32_t rand;
    double r = 0;
    sgx_read_rand((unsigned char*)&rand, sizeof(uint32_t));

    // shrink r to [0,1]
    r = (double)rand / UINT32_MAX;

    // 1 - exp(-lx) = r
    // x = ln(1-r)/(-l)
    if (r <= 1.0 && r >= 0)
    {
        r = -1 * log(r) / lambda;
        return r;
    }

    LL_CRITICAL("RNG fails: r is %f", r);
    return -1;
}