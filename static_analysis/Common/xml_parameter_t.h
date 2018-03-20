//
// Created by fanz on 10/19/16.
//

#ifndef POUW_STATIC_ANALYSIS_XML_PARAMETER_T_H
#define POUW_STATIC_ANALYSIS_XML_PARAMETER_T_H

#include <stdint.h>

typedef struct _xml_parameter_t
{
    const char* name;       //the element name
    uint32_t max_value;
    uint32_t min_value;
    uint32_t value;         //parameter value. Initialized with the default value.
    uint32_t flag;          //Show whether it has been matched
} xml_parameter_t;

#endif //POUW_STATIC_ANALYSIS_XML_PARAMETER_T_H
