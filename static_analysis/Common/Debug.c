#include "Debug.h"
#include "stdint.h"

void dump_buf( const char *title, const unsigned char *buf, size_t len )
{
    hexdump(title, buf, len);
}

void dump_base16(const uint8_t * buf, size_t len) {
    int i;
    for (i = 0; i < len; i++) {
        printf_sgx("%02x", buf[i]);
    }
    printf_sgx("\n");
}

void hexdump(const char* title, void const * data, unsigned int len)
{
    unsigned int i;
    unsigned int r,c;
    
    if (!data)
	return;

    printf_sgx("%s\n", title);
    
    for (r=0,i=0; r<(len/16+(len%16!=0)); r++,i+=16)
    {
        printf_sgx("0x%04X:   ",i); /* location of first byte in line */
	
        for (c=i; c<i+8; c++) /* left half of hex dump */
	    if (c<len)
        	printf_sgx("%02X ",((unsigned char const *)data)[c]);
	    else
		printf_sgx("   "); /* pad if short line */
	
	printf_sgx("  ");
	
	for (c=i+8; c<i+16; c++) /* right half of hex dump */
	    if (c<len)
		printf_sgx("%02X ",((unsigned char const *)data)[c]);
	    else
		printf_sgx("   "); /* pad if short line */
	
	printf_sgx("   ");
	
	for (c=i; c<i+16; c++) /* ASCII dump */
	    if (c<len)
		if (((unsigned char const *)data)[c]>=32 &&
		    ((unsigned char const *)data)[c]<127)
		    printf_sgx("%c",((char const *)data)[c]);
		else
		    printf_sgx("."); /* put this for non-printables */
	    else
		printf_sgx(" "); /* pad if short line */
	
	printf_sgx("\n");
    }
}
