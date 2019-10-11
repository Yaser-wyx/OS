#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "bitmap.h"
#include "stdint.h"
struct virtual_addr {
  struct bitmap vaddr_bitmap;  //虚拟地址的位图结构
  uint32_t vaddr_start;        //虚拟地址起始位置
};

enum pool_flags { PF_KERNEL = 1, PF_USER = 2 };
//根据所处的位来定义
#define PG_P_1 1   //页表项存在
#define PG_P_0 0   //页表项不存在
#define PG_RW_R 0  //读、执行权限
#define PG_RW_W 2  //读、写、执行权限
#define PG_US_S 0  //系统级
#define PG_US_U 4  //用户级

extern struct pool kernel_pool, user_pool;
void mem_init(void);
void* get_kernel_pages(uint32_t pg_cnt);
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void malloc_init(void);
uint32_t* pte_ptr(uint32_t vaddr);
uint32_t* pde_ptr(uint32_t vaddr);
#endif
