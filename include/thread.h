#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "stdint.h"
#include "thread_list.h"
#include "bitmap.h"
#include "memory.h"

//定义自定义函数类型
typedef void thread_func(void *);
extern struct list thread_ready_list;
extern struct list thread_all_list;
//定义线程或进程状态
enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_END
};

//中段栈
struct intr_stack {
    uint32_t vec_no;  //中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t es;
    //以下当cpu从低特权级进入高特权级时压入
    uint32_t err_code;

    void (*eip)(void);

    uint32_t cs;
    uint32_t eflags;
    void *esp;
    uint32_t ss;
};

//线程栈空间，用于存储在线程中待执行的函数
struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    void (*eip)(thread_func *func, void *func_arg);

    void *unused_retaddr;   //占位地址，作为被调用函数的栈顶
    thread_func *function;  //线程调用的函数
    void *func_arg;         //线程调用函数的参数
};

//线程或进程的PCB
struct task_struct {
    uint32_t *self_kstack;          //内核栈顶指针
    enum task_status status;        //当前线程或进程状态
    uint8_t priority;               //优先级
    char name[16];
    uint8_t ticks;                  //每次上cpu执行的时间
    uint32_t elapsed_ticks;         //此任务已经占用了多少cpu时间

    struct list_elem general_tag;
    struct list_elem all_list_tag;
    uint32_t *pgdir;                //进程页表的虚拟地址
    struct virtual_addr user_vaddr; //用户虚拟地址空间
    uint32_t stack_magic;           //检测栈溢出
};

struct task_struct *thread_start(char *name, int priority, thread_func func, void *func_arg);

void init_thread_pcb(struct task_struct *thread_pcb, char *name, int priority);

void create_thread(struct task_struct *thread_pcb, thread_func func, void *func_arg);

void thread_init(void);

struct task_struct *get_running_thread(void);

void schedule(void);

void thread_block(enum task_status state);

void thread_unblock(struct task_struct *blocked_thread);

#endif