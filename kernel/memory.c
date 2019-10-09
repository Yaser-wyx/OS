#include "memory.h"
#include "debug.h"
#include "global.h"
#include "print.h"
#include "stdint.h"
#include "string.h"

#define PG_SIZE 4096

#define MEM_BITMAP_BASE 0xc009a000  //安排位图所在的内存地址空间

#define K_HEAP_START 0xc0100000  //内核栈顶的位置

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

struct pool {
  struct bitmap pool_map;   //使用位图来管理物理内存
  uint32_t phy_addr_start;  //需要管理的物理内存起始地址
  uint32_t pool_size;       //所管理的物理内存池的字节大小
};

//内核内存池以及用户内存池（用于物理内存分配）
struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr;  //用来给内核分配虚拟地址(用于虚拟内存分配)

static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
  int vaddr_start = 0, bit_idx_start = -1;
  uint32_t cnt = 0;
  if (pf == PF_KERNEL) {
    bit_idx_start = bitmap_scan(kernel_vaddr.vaddr_bitmap, pg_cnt);
    if (bit_idx_start == -1) {
      return NULL;
    }
    while (cnt < pg_cnt) {
      bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt, 1);
      cnt++;
    }
    vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
  } else {
  }
  return (void *)vaddr_start;
}
uint32_t *pte_ptr(uint32_t vaddr) {
  uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) +
                               PTE_IDX(vaddr) * 4);
}
static void mem_pool_init(uint32_t all_mem) {
  printf("mem_pool_init start\n");
  /*内存池初始化
  **步骤：
  **1.计算内存物理页数，包括内核物理页以及用户空间物理页
  **2.根据计算的物理页数，计算所需要的位图空间
  **3.将计算出来的数据写入kernel_pool以及user_pool中
  **4.初始化位图空间
  **5.设置内核虚拟内存的位图
  */
  uint32_t page_table_mem = 256 * PG_SIZE;
  //已使用的内存为页目录表+页目录项+低端1MB内存
  uint32_t used_mem = page_table_mem + 0x100000;
  uint32_t free_mem = all_mem - free_mem;
  //计算内存页数
  uint32_t all_free_pages = free_mem / PG_SIZE;
  uint32_t kernel_free_pages = all_free_pages / 2;
  uint32_t user_free_pages = all_free_pages - kernel_free_pages;
  //计算位图空间大小，即位图的字节数
  //位图中一位表示1页，一字节表示8页
  uint32_t kernel_bitmap_length = kernel_free_pages / 8;
  uint32_t user_bitmap_length = user_free_pages / 8;
  //内核物理地址的起始地址就是已使用了的内存
  kernel_pool.phy_addr_start = used_mem;
  //用户物理地址的起始位置就是内核已用空间
  user_pool.phy_addr_start = used_mem + kernel_free_pages * PG_SIZE;

  kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
  user_pool.pool_size = user_free_pages * PG_SIZE;

  kernel_pool.pool_map.btmp_bytes_len = kernel_bitmap_length;
  user_pool.pool_map.btmp_bytes_len = user_bitmap_length;

  kernel_pool.pool_map.bits = (void *)MEM_BITMAP_BASE;
  user_pool.pool_map.bits = (void *)(MEM_BITMAP_BASE + kernel_bitmap_length);

  //初始化位图空间
  bitmap_init(&kernel_pool.pool_map);
  bitmap_init(&user_pool.pool_map);
  printf("      kernel_pool_bitmap_start:");
  printInt((long)kernel_pool.pool_map.bits);
  printf(" kernel_pool_phy_addr_start:");
  printInt(kernel_pool.phy_addr_start);
  printf("\n");
  printf("      user_pool_bitmap_start:");
  printInt((long)user_pool.pool_map.bits);
  printf(" user_pool_phy_addr_start:");
  printInt(user_pool.phy_addr_start);
  printf("\n");
  //设置虚拟内存空间
  kernel_vaddr.vaddr_start = K_HEAP_START;
  kernel_vaddr.vaddr_bitmap.bits =
      (void *)(MEM_BITMAP_BASE + kernel_bitmap_length + user_bitmap_length);
  kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kernel_bitmap_length;
  bitmap_init(&kernel_vaddr.vaddr_bitmap);
  printf("mem_pool init done!\n");
}

void mem_init() {
  printf("mem_init start!\n");
  uint32_t mem_byte_total = (*(uint32_t *)0xb00);
  mem_pool_init(mem_byte_total);
  printf("mem_init done!\n");
}