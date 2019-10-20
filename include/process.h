//
// Created by wanyu on 2019/10/19.
//

#ifndef OS_PROCESS_H
#define OS_PROCESS_H
#include "thread.h"
#include "stdint.h"
#define default_prio 31
#define USER_STACK3_VADDR  (0xc0000000 - 0x1000)
#define USER_VADDR_START 0x8048000
#endif //OS_PROCESS_H
