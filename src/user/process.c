//
// Created by wanyu on 2019/10/19.
//
#include "process.h"
#include "global.h"
#include "debug.h"
#include "memory.h"
#include "thread_list.h"
#include "thread.h"
#include "tss.h"
#include "interrupt.h"
#include "string.h"
#include "console.h"

extern void intr_exit(void);

//构建用户进程初始化上下文信息
void start_process(void *filename) {
    void *function = filename;
    struct task_struct *cur = get_running_thread();
    cur->self_kstack += sizeof(struct thread_stack);//预留线程栈空间
    //将中断栈（保存中断后的程序上下文）存放在pcb的栈中
    struct intr_stack *proc_stack = (struct intr_stack *) cur->self_kstack;
    //初始化各个寄存器
    proc_stack->edi = proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;
    proc_stack->gs = 0;         // 用户态用不上,直接初始为0
    //数据段选择子
    proc_stack->ds = proc_stack->es = proc_stack->fs = SELECTOR_U_DATA;
    //待执行的用户程序
    proc_stack->eip = function;
    //代码段选择子
    proc_stack->cs = SELECTOR_U_CODE;
    //设置eflags寄存器
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    proc_stack->esp = (void *) ((uint32_t) get_a_page(PF_USER, USER_STACK3_VADDR) + PG_SIZE);
    proc_stack->ss = SELECTOR_U_DATA;
    __asm__ volatile ("movl %0,%%esp;jmp intr_exit"::"g"(proc_stack):"memory");

}