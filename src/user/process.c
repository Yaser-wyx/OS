//
// Created by wanyu on 2019/10/19.
//
#include "process.h"
#include "console.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "memory.h"
#include "string.h"
#include "thread.h"
#include "thread_list.h"
#include "tss.h"

extern void intr_exit(void);

#define PDE_768 0x300 * 4

//构建用户进程初始化上下文信息
void start_process(void *filename) {
    void *function = filename;
    struct task_struct *cur = get_running_thread();
    cur->self_kstack += sizeof(struct intr_stack);
    struct intr_stack *intr_stack = (struct intr_stack *) cur->self_kstack;
    intr_stack->eax = intr_stack->ebx = intr_stack->ecx = intr_stack->edi = intr_stack->ebp = intr_stack->edx = intr_stack->esi = intr_stack->gs = intr_stack->esp_dummy = 0;
    intr_stack->ds = intr_stack->es = intr_stack->fs = intr_stack->ss = SELECTOR_U_DATA;
    intr_stack->eip = function;
    intr_stack->cs = SELECTOR_U_CODE;
    intr_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    intr_stack->esp = (void *) ((uint32_t) get_a_page(PF_USER, USER_STACK3_VADDR) + PG_SIZE);  //分配用户进程的栈空间
    __asm__ volatile("movl %0,%%esp;jmp intr_exit;"::"g"(intr_stack) : "memory");
}

//激活页目录表
//todo
void page_dir_active(struct task_struct *pthread) {
    uint32_t pagedir_phy_addr = 0x100000;  // 默认为内核的页目录物理地址,也就是内核线程所用的页目录表
    if (pthread->pgdir != NULL)	{    // 用户态进程有自己的页目录表
        pagedir_phy_addr = addr_v2p((uint32_t)pthread->pgdir);
    }

    /* 更新页目录寄存器cr3,使新页表生效 */
    __asm__ volatile ("movl %0, %%cr3" : : "r" (pagedir_phy_addr) : "memory");
}

//激活线程和页表
void process_active(struct task_struct *pthread) {
    ASSERT(pthread != NULL);
    page_dir_active(pthread);  //更新页表
    if (pthread->pgdir != NULL) {
        //更新用户进程的esp
        update_tss_esp(pthread);
    }
}
uint32_t *create_page_dir() {

    uint32_t *page_dir_vaddr = get_kernel_pages(1);
    if (page_dir_vaddr == NULL) {
        return NULL;
    }
    memcpy((uint32_t *) ((uint32_t)page_dir_vaddr + PDE_768), (uint32_t *) (0xfffff000 + PDE_768), 1024);
    uint32_t page_dir_phyaddr = addr_v2p((uint32_t) page_dir_vaddr);
    page_dir_vaddr[1023] = (page_dir_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    return page_dir_vaddr;
}

void create_user_vaddr_bitmap(struct task_struct *user_thread) {
    user_thread->user_vaddr.vaddr_start = USER_VADDR_START;
    uint32_t user_pages = (0xc0000000 - USER_VADDR_START) / PG_SIZE / 8;
    uint32_t bit_cnt = DIV_ROUND_UP(user_pages, PG_SIZE);
    user_thread->user_vaddr.vaddr_bitmap.bits = get_kernel_pages(bit_cnt);
    user_thread->user_vaddr.vaddr_bitmap.btmp_bytes_len = user_pages;
    bitmap_init(&user_thread->user_vaddr.vaddr_bitmap);

}

//创建用户进程
void process_execute(void *filename, char *name) {
    struct task_struct *thread = get_kernel_pages(1);//给进程分配空间
    init_thread_pcb(thread, name, default_prio);//初始化进程
    create_user_vaddr_bitmap(thread);//创建该进程的虚拟地址空间，用于管理内存
    create_thread(thread, start_process, filename);//创建线程
    thread->pgdir = create_page_dir();//创建进程的页目录表
    saveInterAndDisable;
    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag))
    list_append(&thread_ready_list, &thread->general_tag);//将当前线程添加到就绪队列中
    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag))
    list_append(&thread_all_list, &thread->all_list_tag);//将当前线程添加到所有线程队列中
    reloadInter;
}