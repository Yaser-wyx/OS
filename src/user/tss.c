//
// Created by wanyu on 2019/10/19.
//
#include <thread.h>
#include "tss.h"
#include "stdint.h"
#include "global.h"
#include "string.h"
#include "print.h"

#define GDT_BASE 0xc0000900
#define GDT_SIZE 0x8

//使用linux中的任务切换方式，借助平坦模式，只切换esp指针，从而达到切换任务的目的
struct tss {
    uint32_t back_link;
    uint32_t *esp0;
    uint32_t ss0;
    uint32_t *esp1;
    uint32_t ss1;
    uint32_t *esp2;
    uint32_t ss2;
    uint32_t cr3;

    uint32_t (*eip)(void);

    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint32_t trace;
    uint32_t io_base;
};

static struct tss tss;
static uint32_t gdt_now_index;

//更新tss中esp0的值
void update_tss_esp(struct task_struct *pthread) {
    tss.esp0 = (uint32_t *) ((uint32_t) pthread + PG_SIZE);
}

//创建gdt描述符
static struct gdt_desc make_gdt_desc(uint32_t *desc_addr, uint32_t limit, uint8_t attr_low, uint8_t attr_high) {
    uint32_t desc_base = (uint32_t)desc_addr;
    struct gdt_desc desc;
    desc.limit_low_word = limit & 0x0000ffff;
    desc.base_low_word = desc_base & 0x0000ffff;
    desc.base_mid_byte = ((desc_base & 0x00ff0000) >> 16);
    desc.attr_low_byte = (uint8_t)(attr_low);
    desc.limit_high_attr_high = (((limit & 0x000f0000) >> 16) + (uint8_t)(attr_high));
    desc.base_high_byte = desc_base >> 24;
    return desc;
}

static void install_gdt(uint32_t *addr, uint32_t limit, uint8_t attr_low, uint8_t attr_high) {
    gdt_now_index++;
    struct gdt_desc gdtDesc = make_gdt_desc(addr, limit, attr_low, attr_high);
    *((struct gdt_desc *) (GDT_BASE + (gdt_now_index) * GDT_SIZE)) = gdtDesc;
}

static void reload_gdt() {
    uint64_t gdt_operand = ((uint64_t) (uint32_t) GDT_BASE << 16) | (GDT_SIZE * (gdt_now_index + 1) - 1);//合成gdtr寄存器的值
    __asm__ volatile ("lgdt %0"::"m"(gdt_operand));
    __asm__ volatile ("ltr %w0"::"r"(SELECTOR_TSS));

}

//在gdt中创建tss并重新加载gdt
void tss_init() {

    printf("tss_init start\n");
    uint32_t tss_size = sizeof(tss);
    memset(&tss, 0, tss_size);
    tss.ss0 = SELECTOR_K_STACK;
    tss.io_base = tss_size;

/* gdt段基址为0x900,把tss放到第4个位置,也就是0x900+0x20的位置 */

    /* 在gdt中添加dpl为0的TSS描述符 */
    *((struct gdt_desc*)0xc0000920) = make_gdt_desc((uint32_t*)&tss, tss_size - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);

    /* 在gdt中添加dpl为3的数据段和代码段描述符 */
    *((struct gdt_desc*)0xc0000928) = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    *((struct gdt_desc*)0xc0000930) = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

    /* gdt 16位的limit 32位的段基址 */
    uint64_t gdt_operand = ((8 * 7 - 1) | ((uint64_t)(uint32_t)0xc0000900 << 16));   // 7个描述符大小
    __asm__ volatile ("lgdt %0" : : "m" (gdt_operand));
    __asm__ volatile ("ltr %w0" : : "r" (SELECTOR_TSS));
    printf("tss_init and ltr done\n");
}