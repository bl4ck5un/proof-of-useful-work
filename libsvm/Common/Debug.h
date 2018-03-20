#include "stddef.h"
#include "stdint.h"
#include "printf.h"

#ifndef DEBUG_H_
#define DEBUG_H_

#if defined(__cplusplus)
extern "C" {
#endif

void dump_buf( const char *title, const unsigned char *buf, size_t len );
void hexdump(const char* title, const void * data, unsigned int len);
void dump_base16(const uint8_t* buf, size_t);

#if defined(__cplusplus)
}
#endif



#endif
