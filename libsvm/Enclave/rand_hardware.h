//
// Created by fanz on 11/3/16.
//

#ifndef POUW_RAND_HARDWARE_H
#define POUW_RAND_HARDWARE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t rand();
double rand_exp(double lambda);
// double rand_uniform(int a, int b);
int rand_int_uniform(int a, int b);

#ifdef __cplusplus
}
#endif

#endif //POUW_RAND_HARDWARE_H
