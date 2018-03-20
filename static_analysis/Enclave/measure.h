//
// Created by fanz on 10/19/16.
//

#ifndef POUW_MEASURE_H
#define POUW_MEASURE_H

#include <stdint.h>
#include <cstddef>

#include "manage_metadata.h"

int measure(const uint8_t *elf, size_t elf_size, xml_parameter_t *, int, uint8_t *mr_enclave);

#endif //POUW_MEASURE_H
