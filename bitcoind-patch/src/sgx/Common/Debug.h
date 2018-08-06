#include "stddef.h"
#include "stdint.h"
#include "printf.h"

#ifndef SGX_DEBUG_H_
#define SGX_DEBUG_H_

#if defined(__cplusplus)
extern "C" {
#endif

void dump_buf( const char *title, unsigned char *buf, size_t len );
void hexdump(const char* title, void const * data, unsigned int len);
void base16(const char* title, const uint8_t* buf, size_t);

#if defined(__cplusplus)
}
#endif



#endif
