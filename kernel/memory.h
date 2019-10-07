#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "bitmap.h"
#include "stdint.h"
struct virtual_addr {
  struct bitmap vaddr_bitmap;  //虚拟地址的位图结构
  uint32_t vaddr_start;        //虚拟地址起始位置
};

extern struct pool kerne_pool, user_pool;
void mem_init(void);

#endif
