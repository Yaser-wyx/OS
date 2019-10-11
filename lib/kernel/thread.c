#include "thread.h"
#include "global.h"
#include "memory.h"
#include "string.h"

#define PAGE_SIZE 4096

static void kernel_thread(thread_func *function, void *func_arg) {
    function(func_arg);
}

//创建线程
void create_thread(struct task_struct *thread_pcb, thread_func func, void *func_arg) {
    //预留中断栈以及线程栈的使用空间
    thread_pcb->self_kstack -= (sizeof(struct intr_stack) + sizeof(struct thread_stack));
    struct thread_stack *kthread_stack = (struct thread_stack *) thread_pcb->self_kstack;

    kthread_stack->eip = kernel_thread;
    kthread_stack->function = func;
    kthread_stack->func_arg = func_arg;
    kthread_stack->esi = kthread_stack->edi = kthread_stack->ebx = kthread_stack->ebp = 0;
}

//初始化线程pcb
void init_thread_pcb(struct task_struct *thread_pcb, char *name, int priority) {

    strcpy(thread_pcb->name, name);
    thread_pcb->priority = priority;
    thread_pcb->status = TASK_RUNNING;
    thread_pcb->statck_magic = 0x52013140;
    //将内核栈指针指向栈顶
    thread_pcb->self_kstack = (uint32_t *) ((uint32_t) thread_pcb + PAGE_SIZE);
}

//创建一个名为name的线程，运行func(func_arg),并将其启动
struct task_struct *thread_start(char *name, int priority, thread_func func, void *func_arg) {
    //为当前线程分配一页内存，用于创建pcb
    struct task_struct *thread_pcb = get_kernel_pages(1);
    init_thread_pcb(thread_pcb, name, priority); //初始化pcb块
    create_thread(thread_pcb, func, func_arg);   //创建线程
    //通过ret来调用kernel_thread函数
    __asm__ volatile(
    "movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret"
    :
    : "g"(thread_pcb->self_kstack)
    : "memory");
    return thread_pcb;
}