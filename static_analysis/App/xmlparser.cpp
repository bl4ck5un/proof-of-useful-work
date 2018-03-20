/*
 * Copyright (C) 2011-2016 Intel Corporation. All rights reserved.
 *
 */
#include <errno.h>
#include <iostream>

#include "tinyxml.h"
#include "../Common/xml_parameter_t.h"
#include "metadata.h"
#include "util_st.h"
#include "Log.h"

#define ALIGN_SIZE 0x1000

static bool traverser_parameter(const char *temp_name, const char *temp_text, xml_parameter_t *parameter, int parameter_count)
{
    assert(temp_name != NULL && parameter != NULL);
    uint32_t temp_value=0;
    if(temp_text == NULL)
    {
        LL_CRITICAL(LACK_VALUE_FOR_ELEMENT_ERROR, temp_name);
        return false;
    }
    else
    {
        if(strchr(temp_text, '-'))
        {
            LL_CRITICAL(INVALID_VALUE_FOR_ELEMENT_ERROR, temp_name);
            return false;
        }

        errno = 0;
        char* endptr = NULL;
        temp_value = (uint32_t)strtoul(temp_text, &endptr, 0);
        if(*endptr!='\0'||errno!=0)   //Invalid value or valid value but out of the representable range
        {
            LL_CRITICAL(INVALID_VALUE_FOR_ELEMENT_ERROR, temp_name);
            return false;
        }
    }

    //Look for the matched one
    int i=0;
    for(;i<parameter_count&&strcmp(temp_name,parameter[i].name);i++);
    if(i>=parameter_count) //no matched, return false
    {
        LL_CRITICAL(UNREC_ELEMENT_ERROR, temp_name);
        return false;
    }
    //found one matched
    if(parameter[i].flag==1)  //repeated definition of XML element, return false
    {
        LL_CRITICAL(REPEATED_DEFINE_ERROR, temp_name);
        return false;
    }
    parameter[i].flag = 1;
    if((temp_value<parameter[i].min_value)||
        (temp_value>parameter[i].max_value)) // the value is invalid, return false
    {
        LL_CRITICAL(VALUE_OUT_OF_RANGE_ERROR, temp_name);
        return false;
    }
    parameter[i].value = temp_value;
    return true;
}

bool parse_metadata_file(const char *xmlpath, xml_parameter_t *parameter, int parameter_count)
{
    const char* temp_name=NULL;

    assert(parameter != NULL);
    if(xmlpath == NULL) // user didn't define the metadata xml file.
    {
        LL_NOTICE("Use default metadata...\n");
        return true;
    }
    //use the metadata file that user gives us. parse xml file
    TiXmlDocument doc(xmlpath);
    bool loadOkay = doc.LoadFile();
    if(!loadOkay)
    {
        if(doc.ErrorId() == TiXmlBase::TIXML_ERROR_OPENING_FILE)
        {
            LL_CRITICAL(OPEN_FILE_ERROR, xmlpath);
        }
        else
        {
            LL_CRITICAL(XML_FORMAT_ERROR);
        }
        return false;
    }
    // don't print
    // doc.Print();//Write the document to standard out using formatted printing ("pretty print").

    TiXmlNode *pmetadata_node = doc.FirstChild("EnclaveConfiguration");
    if(!pmetadata_node)
    {
        LL_CRITICAL(XML_FORMAT_ERROR);
        return false;
    }
    TiXmlNode *sub_node = NULL;
    sub_node = pmetadata_node->FirstChild();
    const char *temp_text = NULL;

    while(sub_node)//parse xml node
    {
        switch(sub_node->Type())
        {
        case TiXmlNode::TINYXML_ELEMENT:
            if(sub_node->ToElement()->FirstAttribute() != NULL)
            {
                LL_CRITICAL(XML_FORMAT_ERROR);
                return false;
            }

            temp_name = sub_node->ToElement()->Value();
            temp_text = sub_node->ToElement()->GetText();

            //traverse every node. Compare with the default value.
            if(traverser_parameter(temp_name, temp_text, parameter, parameter_count) == false)
            {
                LL_CRITICAL(XML_FORMAT_ERROR);
                return false;
            }
            break;
        case TiXmlNode::TINYXML_DECLARATION:
        case TiXmlNode::TINYXML_COMMENT:
            break;

        default:
            LL_CRITICAL(XML_FORMAT_ERROR);
            return false;
        }
        sub_node=pmetadata_node->IterateChildren(sub_node);
    }

    return true;
}

