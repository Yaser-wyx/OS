;内核文件
[bits 32]
%define ZERO push 0
%define ERROR_CODE nop

extern idt_table                ;引入c中的中断处理函数表
global intr_exit                ;导出中断处理结束函数
global intr_entry_table        ;导出中断处理入口函数表

section .data
intr_entry_table:
%macro VECTOR 2       ;中断向量表
section .text
    intr%1entery:
        %2                      ;统一操作，如果CPU没有压入错误码，则手动压入0，否则不操作
        ;保存上下文
        push es
        push gs
        push fs
        push ds
        pushad

        ;手动发送EOI中断结束命令
        mov al,0x20             ;手动发送EOI中断结束指令
        out 0x20,al             ;设置主片
        out 0xa0,al             ;设置从片

        push %1                 ;将中断向量压栈
        call [idt_table + %1*4] ;调用指定的中断处理函数
        ;恢复上下文
        jmp intr_exit
section .data
    dd intr%1entery
%endmacro
section .text
    intr_exit:
        add esp,4               ;跳过中断号
        popad
        pop ds
        pop fs
        pop gs
        pop es
        ;清除错误码
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