#include "thread.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "memory.h"
#include "string.h"
#include "print.h"

#define PAGE_SIZE 4096

struct task_struct *main_thread;      //主线程pcb
struct list thread_ready_list;        //就绪队列
struct list thread_all_list;          //所有任务队列

//线程切换
extern void switch_to(struct task_struct *current, struct task_struct *next);

static void kernel_thread(thread_func *function, void *func_arg) {
    intr_enable();  //开启中断
    function(func_arg);
}

//获取当前线程pcb指针
struct task_struct *get_running_thread() {
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" :  "=g"(esp));
    return (struct task_struct *) (esp & 0xfffff000);
}

//为main_thread创建pcb
static void make_main_thread(void) {
    main_thread = get_running_thread();  //获取main_thread，当前运行的线程就是main_thread
    init_thread_pcb(main_thread, "main", 10);  //初始化主线程的pcb
    ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    //将主线程pcb中的tag加入到所有线程队列中
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

//线程初始化
void thread_init(void) {
    //步骤：
    // 1.初始化两个线程队列：就绪队列以及所有线程队列
    // 2.生成主线程的pcb，并将其放入所有线程队列中
    printf("thread init start!\n");
    thread_list_init(&thread_all_list);
    thread_list_init(&thread_ready_list);
    make_main_thread();  //创建主线程pcb
    printf("thread init done!\n");
}

//创建一个名为name的线程，运行func(func_arg),并将其启动
struct task_struct *thread_start(char *name, int priority, thread_func func, void *func_arg) {
    //为当前线程分配一页内存，用于创建pcb
    struct task_struct *thread_pcb = get_kernel_pages(1);
    init_thread_pcb(thread_pcb, name, priority);  //初始化pcb块
    create_thread(thread_pcb, func, func_arg);    //创建线程

    //保证要运行的线程还没有放到ready队列中
    ASSERT(!elem_find(&thread_ready_list, &thread_pcb->general_tag));
    list_append(&thread_ready_list, &thread_pcb->general_tag);

    ASSERT(!elem_find(&thread_all_list, &thread_pcb->all_list_tag));
    list_append(&thread_all_list, &thread_pcb->all_list_tag);

    return thread_pcb;
}

//初始化线程的pcb
void init_thread_pcb(struct task_struct *thread_pcb, char *name, int priority) {
    strcpy(thread_pcb->name, name);
    if (thread_pcb == main_thread) {
        //如果是主线程
        thread_pcb->status = TASK_RUNNING;
    } else {
        thread_pcb->status = TASK_READY;
    }
    thread_pcb->ticks = priority;
    thread_pcb->elapsed_ticks = 0;
    thread_pcb->pgdir = NULL;
    thread_pcb->priority = priority;
    thread_pcb->stack_magic = 0x52013140;
    //将内核栈指针指向栈顶
    thread_pcb->self_kstack = (uint32_t *) ((uint32_t) thread_pcb + PAGE_SIZE);
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

//调度算法：时间片轮转法
void schedule() {
    ASSERT(get_intr_status() == INTR_OFF);
    //获取当前运行中的线程
    struct task_struct *current_task = get_running_thread();
    if (current_task->status == TASK_RUNNING) {//当前线程处于运行状态，同时时间片已经用完了
        ASSERT(!elem_find(&thread_ready_list, &current_task->general_tag));//确保其不在ready队列中
        current_task->status = TASK_READY;
        current_task->ticks = current_task->priority;
        list_append(&thread_ready_list, &current_task->general_tag);//将其加入到就绪队列里面
    } else {
        //处理阻塞或等待状态的线程
    }

    ASSERT(!list_empty(&thread_ready_list));//确保就绪队列不为空
    //从就绪队列中取下一个进行运行
    struct list_elem *thread_tag = list_pop(&thread_ready_list);
    struct task_struct *new_task = elem2entry(struct task_struct, general_tag, thread_tag);
    new_task->status = TASK_RUNNING;
    switch_to(current_task, new_task);
}

//将当前线程阻塞
void thread_block(enum task_status state) {
    saveInterAndDisable;
    ASSERT(state == TASK_BLOCKED || state == TASK_WAITING || state == TASK_HANGING);
    struct task_struct *current_thread = get_running_thread();//获取当前运行中的线程
    current_thread->status = state;
    schedule();
    reloadInter;
}

//释放指定的线程
void thread_unblock(struct task_struct *blocked_thread) {
    saveInterAndDisable;
    ASSERT(blocked_thread->status != TASK_READY || blocked_thread != TASK_RUNNING);
    if (blocked_thread->status != TASK_READY) {
        blocked_thread->status = TASK_READY;//将阻塞线程重新切换至就绪状态
        ASSERT(!elem_find(&thread_ready_list, &blocked_thread->general_tag));
        list_push(&thread_ready_list, &blocked_thread->general_tag);
    }
    reloadInter;
}