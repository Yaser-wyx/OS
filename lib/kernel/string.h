#ifndef __LIB_KERNEL_STRING_H
#define __LIB_KERNEL_STRING_H
#include "stdint.h"
char *strcpy(char *_dist_, const char *_src_);
uint32_t strlen(const char *src);
int8_t strcmp(const char *_dist_, const char *_src_);
char *strchr_first(const char *_src_, const char ch);
char* strrchr(const char *_src_, const char ch);
#endif