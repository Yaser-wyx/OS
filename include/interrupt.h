#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include "stdint.h"

void idt_init(void);

typedef void *interrupt_handler;  //中断处理函数指针
//定义两种中断状态：开启，关闭
enum intr_status { INTR_OFF, INTR_ON };

enum intr_status intr_disable(void);                 //关闭中断
enum intr_status get_intr_status(void);              //获取中断状态
enum intr_status set_intr_status(enum intr_status);  //设置中断状态
enum intr_status intr_enable(void);                  //开启中断
void register_handler(uint8_t intr_num, interrupt_handler handler_function);
#endif