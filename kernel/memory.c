#include "memory.h"
#include "debug.h"

#define PG_SIZE 4096

struct pool {
  struct bitmap pool_bitmap;
  uint32_t phy_addr_start;
  uint32_t pool_size;
};
#include "memory.h"
#include "print.h"
#include "stdint.h"

#define PG_SIZE 4096

#define MEM_BITMAP_BASE 0xc009a000  //安排位图所在的内存地址空间

#define K_HEAP_START 0xc0100000  //内核栈顶的位置

struct pool {
  struct bitmap pool_map;   //使用位图来管理物理内存
  uint32_t phy_addr_start;  //需要管理的物理内存起始地址
  uint32_t pool_size;       //所管理的物理内存池大小
};

struct pool kernel_pool, user_pool;  //内核内存池以及用户内存池
struct virtual_addr kernel_vaddr;    //用来给内核分配虚拟地址 