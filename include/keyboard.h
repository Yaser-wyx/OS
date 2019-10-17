//
// Created by wanyu on 2019/10/16.
//

#ifndef OS_KEYBOARD_H
#define OS_KEYBOARD_H

#include "ioqueue.h"

extern struct ioqueue keyboard_buf;//键盘环形缓冲区

void keyboard_init(void);

#endif //OS_KEYBOARD_H
