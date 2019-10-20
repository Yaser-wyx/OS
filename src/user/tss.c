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
    uint32_t desc_base = (uint32_t) desc_addr;
    struct gdt_desc gdtDesc;
    gdtDesc.limit_low_word = limit & (0x0000ffff);
    gdtDesc.base_low_word = desc_base & 0x0000ffff;
    gdtDesc.base_mid_byte = (desc_base & 0x00ff0000) >> 16;
    gdtDesc.base_high_byte = desc_base >> 24;
    gdtDesc.attr_low_byte = attr_low;
    gdtDesc.limit_high_attr_high = (attr_high & 0xf0) | ((limit & 0x000f0000) >> 16);
    return gdtDesc;
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
    printf("tss init!\n");

    gdt_now_index = 3;
    uint32_t tss_size = sizeof(struct tss);
    memset(&tss, tss_size, 0);//初始化内存区域
    tss.ss0 = SELECTOR_K_STACK;//选择系统内核栈选择子
    tss.io_base = tss_size;//设置tss的io位图为空

    install_gdt((uint32_t *) &tss, tss_size - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);
    install_gdt((uint32_t *) 0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    install_gdt((uint32_t *) 0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    reload_gdt();
    printInt(gdt_now_index);
    printf("\ntss init done!\n");
}