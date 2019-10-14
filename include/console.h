#ifndef __DEVICE_CONSOLE_H
#define __DEVICE_CONSOLE_H

#include "stdint.h"

//初始化终端
void console_init();

//获取终端
void get_console();

//释放终端
void release_console();

//使用终端输出字符串
void console_printf(char *str);

//使用终端输出字符串
void console_printInt(unsigned long num);

void console_putchar(uint8_t c);

#endif