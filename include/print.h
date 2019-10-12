#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H
#include "stdint.h"
void put_char(uint8_t char_asci);
void printf(uint8_t *str);
void printInt(unsigned long number);
void set_cursor(uint32_t cursor_pos);  //设置光标位置
#endif