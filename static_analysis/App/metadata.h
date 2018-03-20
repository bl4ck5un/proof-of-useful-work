/*
 * Copyright (C) 2011-2016 Intel Corporation. All rights reserved.
 *
 */

#ifndef _METADATA_H_
#define _METADATA_H_


#define MAJOR_VERSION 1         /* MAJOR_VERSION should not larger than 0ffffffff */
#define MINOR_VERSION 3         /* MINOR_VERSION should not larger than 0ffffffff */

#define META_DATA_MAKE_VERSION(major, minor) (((uint64_t)major)<<32 | minor)

#define METADATA_MAGIC 0x86A80294635D0E4CULL
#define METADATA_SIZE 0x1000
/* TCS Policy bit masks */
#define TCS_POLICY_BIND     0x00000000  /* If set, the TCS is bound to the application thread */
#define TCS_POLICY_UNBIND   0x00000001

#define MAX_SAVE_BUF_SIZE 2632    

#define TCS_NUM_MIN 1
#define SSA_NUM_MIN 2
#define SSA_FRAME_SIZE_MIN 1
#define SSA_FRAME_SIZE_MAX 2
#define STACK_SIZE_MIN 0x1000
#define HEAP_SIZE_MIN 0
#define DEFAULT_MISC_SELECT 0
#define DEFAULT_MISC_MASK 0xFFFFFFFF

#endif
