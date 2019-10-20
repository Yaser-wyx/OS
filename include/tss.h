//
// Created by wanyu on 2019/10/19.
//

#ifndef OS_TSS_H
#define OS_TSS_H

#include "thread.h"
void tss_init();
void update_tss_esp(struct task_struct *pthread) ;
#endif //OS_TSS_H
