#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sgx_eid.h>

#include "svm.h"
#include "Log.h"
#include "Enclave_u.h"
#include "benchmarks.h"

#include <vector>
#include <iostream>

using namespace std;

char *line = NULL;
int max_line_len;

static char *readline(FILE *input) {
    int len;

    if (fgets(line, max_line_len, input) == NULL)
        return NULL;

    while (strrchr(line, '\n') == NULL) {
        max_line_len *= 2;
        line = (char *) realloc(line, max_line_len);
        len = (int) strlen(line);
        if (fgets(line + len, max_line_len - len, input) == NULL)
            break;
    }
    return line;
}

void exit_input_error(int line_num) {
    fprintf(stderr, "Wrong input format at line %d\n", line_num);
    exit(1);
}

void
svm_test(sgx_enclave_id_t eid, FILE *input, struct svm_model *model, sgx_target_info_t *eq_info, sgx_report_t *report) {
    int max_nr_attr = 64;
    int correct = 0;
    int total = 0;
    double error = 0;
    double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

    int svm_type = svm_get_svm_type(model);
    int nr_class = svm_get_nr_class(model);
    double *prob_estimates = NULL;
    int j;

    int ret;

    // allocate memory for a line
    max_line_len = 1024;
    line = (char *) malloc(max_line_len * sizeof(char));

    vector<struct svm_node*> x_arr;
    vector<double> target_label_arr;

    while (readline(input) != NULL) {
        struct svm_node *x = (struct svm_node *) malloc(max_nr_attr * sizeof(struct svm_node));
        int i = 0;

        char *idx, *val, *label, *endptr;
        int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

        label = strtok(line, " \t\n");
        if (label == NULL) // empty line
            exit_input_error(total + 1);

        // store the target_label
        target_label_arr.push_back(strtod(label, &endptr));
        if (endptr == label || *endptr != '\0')
            exit_input_error(total + 1);


        while (1) {
            if (i >= max_nr_attr - 1)    // need one more for index = -1
            {
                max_nr_attr *= 2;
                x = (struct svm_node *) realloc(x, max_nr_attr * sizeof(struct svm_node));
            }

            idx = strtok(NULL, ":");
            val = strtok(NULL, " \t");

            if (val == NULL)
                break;
            errno = 0;
            x[i].index = (int) strtol(idx, &endptr, 10);
            if (endptr == idx || errno != 0 || *endptr != '\0' || x[i].index <= inst_max_index)
                exit_input_error(total + 1);
            else
                inst_max_index = x[i].index;

            errno = 0;
            x[i].value = strtod(val, &endptr);
            if (endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
                exit_input_error(total + 1);

            ++i;
        }
        x[i].index = -1;

        x_arr.push_back(x);
    }

    if (x_arr.size() != target_label_arr.size()) {
        cerr << "x_arr and target_label_arr have different sizes" << endl;
        return;
    }

    double* predict_label_arr = new double[x_arr.size()];
    sgx_predict(eid, model, x_arr.data(), x_arr.size(), predict_label_arr, eq_info, report);


    for (int i = 0; i < x_arr.size(); i++) {
        double predict_label = predict_label_arr[i];
        double target_label = target_label_arr[i];
        if (predict_label == target_label)
            ++correct;
        error += (predict_label - target_label) * (predict_label - target_label);
        sump += predict_label;
        sumt += target_label;
        sumpp += predict_label * predict_label;
        sumtt += target_label * target_label;
        sumpt += predict_label * target_label;
        ++total;
    }

    if (svm_type == NU_SVR || svm_type == EPSILON_SVR) {
        LL_NOTICE("Mean squared error = %g (regression)\n", error / total);
        LL_NOTICE("Squared correlation coefficient = %g (regression)\n",
                  ((total * sumpt - sump * sumt) * (total * sumpt - sump * sumt)) /
                  ((total * sumpp - sump * sump) * (total * sumtt - sumt * sumt))
        );
    } else
        LL_NOTICE("Accuracy = %g%% (%d/%d) (classification)", (double) correct / total * 100, correct, total);

    for (auto x : x_arr) {
        free(x);
    }
    delete predict_label_arr;
}