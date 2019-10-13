[bits 32]
section .text
global switch_to
switch_to:
    ;----保存原有的线程上下文----
    ;保存内核中断寄存器
    push esi
    push edi
    push ebx
    push ebp
    mov eax,[esp+20];读取当前线程的栈顶指针地址
    mov [eax],esp;把当前的栈指针保存到当前线程的栈顶指针中去
    ;----恢复下一个要运行的线程的上下文----

    mov eax,[esp+24]
    mov esp,[eax]
    pop ebp
    pop ebx
    pop edi
    pop esi
    ret
