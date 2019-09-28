#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "print.h"
#include "io.h"

#define PIC_M_CTRL 0x20 //主片控制端口
#define PIC_M_DATA 0x21 //主片数据端口
#define PIC_S_CTRL 0xa0 //从片控制端口
#define PIC_S_DATA 0xa1 //从片数据端口

#define IDT_DESC_CNT 0x21                           //支持的中断数
extern intr_handler intr_entry_table[IDT_DESC_CNT]; //声明引用kernel.asm中的中断处理函数数组
char *intr_name[IDT_DESC_CNT];
intr_handler idt_table[IDT_DESC_CNT];

//中断门描述符结构
struct gate_desc
{
    uint16_t func_offect_low_word;
    uint16_t selector;
    uint8_t dcount;
    uint8_t attribute;
    uint16_t func_offect_high_word;
} static idt[IDT_DESC_CNT]; //声明全局中断门描述符数组

static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attribute, intr_handler function)
{
    p_gdesc->func_offect_low_word = (uint32_t)function & 0x0000ffff; //获取目标程序高16位地址
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attribute;
    p_gdesc->func_offect_high_word = ((uint32_t)function & 0xffff0000) >> 16;
}
static void idt_desc_init(void)
{
    for (uint32_t i = 0; i < IDT_DESC_CNT; i++)
    {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
    printf("idt_desc_init!\n\0");
}
static void pic_init(void)
{
    //设置主片
    outb(PIC_M_CTRL, 0x11);
    outb(PIC_M_DATA, 0x20);
    outb(PIC_M_DATA, 0x04);
    outb(PIC_M_DATA, 0x01);

    //设置从片
    outb(PIC_S_CTRL, 0x11);
    outb(PIC_S_DATA, 0x28);
    outb(PIC_S_DATA, 0x02);
    outb(PIC_S_DATA, 0x01);

    //打开主片的IR0
    outb(PIC_M_DATA, 0xfe);
    outb(PIC_S_DATA, 0xff);
    printf("pic inint done!\n\0");
}
static void general_intr_handler(uint8_t vec_nr)
{
    if (vec_nr == 0x27 || vec_nr == 0x2f)
    {
        return;
    }
    printf("\nint vector:");
    printInt(vec_nr);
}
static void exception_init(void)
{
    for (int i = 0; i < IDT_DESC_CNT; i++)
    {
        idt_table[i] = general_intr_handler;
        intr_name[i] = "unknow";
    }
    intr_name[0] = "#DE Divide Error";
    intr_name[1] = "#DB Debug Exception";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR BOUND Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP General Protection Exception";
    intr_name[14] = "#PF Page-Fault Exception";
    // intr_name[15] 第15项是intel保留项，未使用
    intr_name[16] = "#MF x87 FPU Floating-Point Error";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine-Check Exception";
    intr_name[19] = "#XF SIMD Floating-Point Exception";
}
//完成中断的初始化工作
void idt_init()
{
    printf("idt_init start\n\0");
    idt_desc_init();
    exception_init();
    pic_init();
    //加载idt

    uint64_t idt_operand = ((sizeof(idt) - 1) | (((uint64_t)(uint32_t)idt) << 16));
    __asm__ volatile("lidt %0" ::"m"(idt_operand));
    printf("idt_init done\n\0");
}
