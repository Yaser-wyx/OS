#ifndef __LIB_KERNEL_MEMORY_H
#define __LIB_KERNEL_MEMORY_H
#include "stdint.h"
void memset(void *_dist_, uint32_t size, uint32_t value);
int memcmp(const void *dist, const void *src, uint32_t size);
void memcpy(void *dist, const void *src, uint32_t size);
#endif
