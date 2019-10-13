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

uint32_t kernel_free_pages;
uint32_t user_free_pages;
struct pool {
    struct bitmap pool_map;   //使用位图来管理物理内存
    uint32_t phy_addr_start;  //需要管理的物理内存起始地址
    uint32_t pool_size;       //所管理的物理内存池的字节大小
};

//内核内存池以及用户内存池（用于物理内存分配）
struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr;  //用来给内核分配虚拟地址(用于虚拟内存分配)

//分配虚拟内存空间
static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
    int vaddr_start = 0, bit_index_start = -1;
    if (pf == PF_KERNEL) {  //如果是为内核分配虚拟空间
        bit_index_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);//扫描足够的区域用于虚拟地址
        if (bit_index_start == -1) {
            return NULL;
        }
        uint32_t cnt = 0;
        while (cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_index_start + cnt, 1);
            //将指定位置的位图填充，表示该区域的内存已经使用了
            cnt++;
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_index_start * PG_SIZE;
    }
    return (void *) vaddr_start;
}

uint32_t *pte_ptr(uint32_t vaddr) {
    return (uint32_t *) (0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
}

uint32_t *pde_ptr(uint32_t vaddr) {
    return (uint32_t *) (0xfffff000 + PDE_IDX(vaddr) * 4);
}

//在m_pool指向的物理内存池中，分配1个page
static void *palloc(struct pool *m_pool) {
    int bit_index = bitmap_scan(&m_pool->pool_map, 1);//扫描一页物理内存
    if (bit_index == -1) {
        return NULL;
    }
    bitmap_set(&m_pool->pool_map, bit_index, 1);//将该页内存空间分配出去
    uint32_t page_phyaddr = m_pool->phy_addr_start + bit_index * PG_SIZE;//计算该页的起始位置
    return (void *) page_phyaddr;
}

//向页表中添加一条记录，表示新增的物理页
static void page_table_add(void *_vaddr, void *_page_phyaddr) {
    uint32_t vaddr = (uint32_t) _vaddr;//虚拟地址
    uint32_t page_phyaddr = (uint32_t) _page_phyaddr;//物理地址
    uint32_t *pde = pde_ptr(vaddr);//获取虚拟地址的ped
    uint32_t *pte = pte_ptr(vaddr);//获取pte
    ASSERT(!(*pte) & 0x1);
    if (!(*pde & 0x1)) {
        //如果pde不存在，就从内核空间中分配一页框
        uint32_t pde_phyaddr = (uint32_t) palloc(&kernel_pool);
        //将新分配的页框地址写入pde表中
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        memset((void *) ((int) pte & 0xfffff000), 0, PG_SIZE);  //将新的物理页清空为0
    }
    *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
}

//分配pg_cnt页空间，成功则返回起始虚拟地址，否则返回null
void *malloc_page(enum pool_flags pf, uint32_t pg_cnt) {
    //先判断剩余的页面数否够pg_cnt
    ASSERT(pg_cnt > 0);
    struct pool *mem_pool;//内存池
    if (pf == PF_KERNEL) {
        mem_pool = &kernel_pool;
        ASSERT(pg_cnt < kernel_free_pages);
        kernel_free_pages -= pg_cnt;
    } else {
        mem_pool = &user_pool;
        ASSERT(pg_cnt < user_free_pages);
        user_free_pages -= pg_cnt;
    }
    //页面数充足
    //获取虚拟地址
    void *vaddr_start = vaddr_get(pf, pg_cnt);
    if (vaddr_start == NULL) {
        return NULL;
    }
    uint32_t vaddr = (uint32_t) vaddr_start, cnt = pg_cnt;
    while (cnt--) {
        //分配一个物理页面
        void *phyaddr = palloc(mem_pool);
        if (phyaddr == NULL) {  //如果物理页面分配失败
            return NULL;
        }
        page_table_add((void *) vaddr, phyaddr);//将物理内存与虚拟地址做映射关系
        vaddr += PG_SIZE;
    }
    return vaddr_start;
}

//获取内核分配后的页面起始位置
void *get_kernel_pages(uint32_t pg_cnt) {
    void *vaddr = malloc_page(PF_KERNEL, pg_cnt);//分配物理页面以及虚拟内存地址
    if (vaddr != NULL) {
        //将新分配的内存初始化
        memset(vaddr, 0, pg_cnt * PG_SIZE);
    }
    return vaddr;
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
    uint32_t free_mem = all_mem - used_mem;
    //计算内存页数
    uint32_t all_free_pages = free_mem / PG_SIZE;
    kernel_free_pages = all_free_pages / 2;
    user_free_pages = all_free_pages - kernel_free_pages;
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

    kernel_pool.pool_map.bits = (void *) MEM_BITMAP_BASE;
    user_pool.pool_map.bits = (void *) (MEM_BITMAP_BASE + kernel_bitmap_length);

    //初始化位图空间
    bitmap_init(&kernel_pool.pool_map);
    bitmap_init(&user_pool.pool_map);
    printf("kernel_pool_bitmap_start:");
    printInt((long) kernel_pool.pool_map.bits);
    printf("kernel_pool_phy_addr_start:");
    printInt(kernel_pool.phy_addr_start);
    printf("\n");
    printf("user_pool_bitmap_start:");
    printInt((long) user_pool.pool_map.bits);
    printf("user_pool_phy_addr_start:");
    printInt(user_pool.phy_addr_start);
    printf("\n");
    //设置虚拟内存空间
    kernel_vaddr.vaddr_start = K_HEAP_START;
    kernel_vaddr.vaddr_bitmap.bits = (void *) (MEM_BITMAP_BASE + kernel_bitmap_length + user_bitmap_length);
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kernel_bitmap_length;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    printf("mem_pool init done!\n");
}

void mem_init() {
    printf("mem_init start!\n");
    uint32_t mem_byte_total = (*(uint32_t *) 0xb00);
    mem_pool_init(mem_byte_total);
    printf("mem_init done!\n");
}