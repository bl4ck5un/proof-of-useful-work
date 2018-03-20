/*
 * Copyright (C) 2011-2016 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#ifndef SE_STDIO_H
#define SE_STDIO_H

#include <stdio.h>
#include <stddef.h>
#include "se_memcpy.h"
#include <stdarg.h>
#include <unistd.h>
//#include <sys/stat.h>
#include <sys/types.h>
//#include <fcntl.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

static inline int sprintf_s(char *dst_buf, size_t size_in_bytes, const char *format, ...)
{
    va_list argptr;
    int cnt;
    va_start(argptr, format);
    cnt = vsnprintf(dst_buf, size_in_bytes, format, argptr);
    va_end(argptr);
    return cnt;
}

static inline int _snprintf_s(char *dst_buf, size_t size_in_bytes, size_t max_count, const char *format, ...)
{
    (void) size_in_bytes;
    va_list argptr;
    int cnt;
    va_start(argptr, format);
    cnt = vsnprintf(dst_buf, max_count, format, argptr);
    va_end(argptr);
    return cnt;
}

#ifdef __cplusplus
template <size_t _Size>
int sprintf_s(char (&dst)[_Size], const char *format, ...)
{
    va_list argptr;
    int cnt;
    va_start(argptr, format);
    cnt = vsprintf(dst, format, argptr);
    va_end(argptr);
    return cnt;
}

template<size_t _Size>
int _snprintf_s(char (&dst)[_Size], size_t max_count, const char *format, ...)
{
    va_list argptr;
    int cnt;
    va_start(argptr, format);
    cnt = vsnprintf(dst, max_count, format, argptr);
    va_end(argptr);
    return cnt;
}

#endif

#endif
