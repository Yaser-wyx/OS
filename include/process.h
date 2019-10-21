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

void process_execute(void *filename, char *name);

void create_user_vaddr_bitmap(struct task_struct *user_thread);

uint32_t *create_page_dir();

void process_active(struct task_struct *pthread);

void page_dir_active(struct task_struct *pthread);

void start_process(void *filename);

#endif //OS_PROCESS_H
