//
// Created by fanz on 6/10/16.
//

#ifndef POUW_BASE64_H
#define POUW_BASE64_H

#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif //POUW_BASE64_H
