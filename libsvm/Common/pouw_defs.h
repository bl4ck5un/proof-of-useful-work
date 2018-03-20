//
// Created by fan on 6/9/16.
//

#include <stdint.h>

#include "svm.h"

#ifndef POUW_POUW_H
#define POUW_POUW_H

#define MAX_LOOP_N 100
#define MAX_LINE_INPUT 100

typedef struct _pow_prob {
    struct svm_model* model;
    struct svm_node* x;
} pow_spec;

typedef struct _output_t {
    double label;
} output_t;

typedef struct _svm_input {
    int num_of_lines;
    char** input_files;
} svm_input;

typedef struct _svm_ouput{
    int num_of_lines;
    char** input_files;
} svm_output;

#endif //POUW_POUW_H
