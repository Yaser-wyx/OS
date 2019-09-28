[bits 32]
%define ERROR_CODE nop
%define ZERO push 0

extern printf                       ;声明外部函数
extern idt_table                    ;在c中注册的中断处理程序数组
section .data
intr_str db "interrupt occur!",0xa,0
global intr_entry_table             ;导出内部函数

intr_entry_table:
%macro VECTOR 2                     ; 宏定义，有两个参数（中断向量号，）
section .text
intr%1entery:                       ;入口地址
    %2                              ;压入中断向量号
    push ds
    push es
    push fs
    push gs
    pushad
    
    ;手动写入OCW2的EOI位，表示结束中断
    mov al,0x20                     ;要写入的数值
    out 0x20,al                     ;设置主片
    out 0xA0,al                     ;设置从片

    push %1
    call [idt_table + %1*4]

    jmp intr_exit                   ;从中断返回
section .data
    dd intr%1entery
%endmacro

section .text
global intr_exit
intr_exit:
    add esp,4
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp,4
    iretd


VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO 
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO 
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE 
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO 
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO 
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO 
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE 
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO 
VECTOR 0x20,ZERO
