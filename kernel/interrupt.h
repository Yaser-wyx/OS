#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "stdint.h"
void idt_init(void);
typedef void *interrupt_handler; //中断处理函数指针
#endif