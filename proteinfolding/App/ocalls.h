#pragma once

#include <iostream>
std::string current_datetime();

#if defined(__cpluscplus)
unsigned long long rdtsc(void);
#endif
