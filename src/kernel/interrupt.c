#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "print.h"
#include "stdint.h"

#define IDT_CNT 33
#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

#define EFLAGS_IF 0X00000200  //将eflags的if位置1
#define GET_EFLAGS(eflags_value) \
  __asm__ volatile("pushfl; popl %0" : "=g"(eflags_value))

interrupt_handler idt_table[IDT_CNT];                //中断处理函数表
char *intr_name[IDT_CNT];                            //中断处理函数名字
extern interrupt_handler intr_entry_table[IDT_CNT];  //中断处理入口程序表

struct idt_gate_desc  //中断门描述符数据结构
{
  uint16_t intr_off_func_low;
  uint16_t func_desc_selector;
  uint8_t none;
  uint8_t attribute;
  uint16_t intr_off_func_hight;
} static idt_gate_desc_table[IDT_CNT];  //定义一个中断门描述符表

static void idt_desc_setup(struct idt_gate_desc *idt_desc, uint8_t attr,
                           interrupt_handler func) {
  //中断门描述符设置
  idt_desc->intr_off_func_low = (uint32_t)func;
  idt_desc->func_desc_selector = SELECTOR_K_CODE;
  idt_desc->attribute = attr;
  idt_desc->none = 0;
  idt_desc->intr_off_func_hight = (uint32_t)func >> 16;
}

static void idt_desc_table_init() {
  //中断描述符表设置
  printf("init idt_desc\n");
  for (int i = 0; i < IDT_CNT; i++) {
    idt_desc_setup(&idt_gate_desc_table[i], IDT_DESC_ATTR_DPL0,
                   intr_entry_table[i]);
  }
  printf("idt_table init done!\n");
}

static void general_intr_handler(uint8_t vector_num) {
  static long cnt = 0;
  //通用中断处理函数
  if (vector_num == 0x27 || vector_num == 0x2f) {  //对于IRQ7与IRQ15不做处理
    return;
  }
  set_cursor(0);  //设置光标位置为起始
  int cursor_pos = 0;
  while (cursor_pos < 320) {
    put_char(' ');  //清除出一片空白区域
    cursor_pos++;
  }
  set_cursor(0);
  printf("!!!!!    interrupt occur message start   !!!!!");
  set_cursor(88);
  printf(intr_name[vector_num]);  //打印中断名
  if (vector_num == 14) {
    //缺页异常
    int page_fault_vaddr = 0;
    __asm__ volatile("movl %%cr2,%0" : "=g"(page_fault_vaddr));
    printf("\npage fault vaddr:");
    printInt(page_fault_vaddr);
  }
  printf("\n!!!!!    interrupt occur message end     !!!!!");
  while (1) {
    //设置死循环
  }
}

static void exception_init() {  //异常初始化
  for (int i = 0; i < IDT_CNT; i++) {
    idt_table[i] = general_intr_handler;  //指向中断处理程序
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

static void handle_pic_init() {
  //设置主片
  outb(PIC_M_CTRL, 0x11);
  outb(PIC_M_DATA, 0x20);
  outb(PIC_M_DATA, 0x04);
  outb(PIC_M_DATA, 0x05);
  //设置从片
  outb(PIC_S_CTRL, 0x11);
  outb(PIC_S_DATA, 0x28);
  outb(PIC_S_DATA, 0x02);
  outb(PIC_S_DATA, 0x01);
  //开启指定中断号
  outb(PIC_M_DATA, 0xfe);
  outb(PIC_S_DATA, 0xff);
  printf("8259A chip init done!\n");
}

void idt_init() {  //初始化中断描述符表
  printf("start to init idt!\n");
  idt_desc_table_init();  //中断描述符表初始化
  exception_init();       //初始化所有的异常处理函数
  handle_pic_init();      //初始化8259A芯片
  //加载idt到idtr寄存器中
  uint64_t idt = (sizeof(idt_gate_desc_table) - 1) |
                 ((uint64_t)(uint32_t)idt_gate_desc_table << 16);
  __asm__ volatile("lidt %0" ::"m"(idt));
  printf("idt init done!\n");
}
//注册中断处理函数
void register_handler(uint8_t intr_num, interrupt_handler handler_function) {
  idt_table[intr_num] = handler_function;
}
enum intr_status intr_disable() {
  printf("disable intr");
  enum intr_status old_status = INTR_OFF;
  if (get_intr_status() != INTR_OFF) {
    __asm__ volatile("cli" : : : "memory");
    old_status = INTR_ON;
  }
  return old_status;
}

enum intr_status intr_enable() {
  printf("enable intr");
  enum intr_status old_status = INTR_ON;
  if (get_intr_status() != INTR_ON) {
    __asm__ volatile("sti");
    old_status = INTR_OFF;
  }
  return old_status;
}

enum intr_status get_intr_status() {
  uint32_t eflags = 0;
  GET_EFLAGS(eflags);
  return (eflags & EFLAGS_IF) ? INTR_ON : INTR_OFF;
}

enum intr_status set_intr_status(enum intr_status status) {
  return status & INTR_OFF ? intr_disable() : intr_enable();
}