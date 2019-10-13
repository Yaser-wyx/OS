#include "timer.h"
#include "debug.h"
#include "interrupt.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#include "thread.h"

const uint32_t FREQUENCY = 1193180;
const uint32_t IRQ0_FREQUENCY = 100;
static uint32_t ticks;  //自内核开启中断以来总共的滴答数
#define CTRL_PORT 0x43
#define TIMER_NO_0 0
#define TIMER_NO_1 1
#define TIMER_NO_2 2

#define RW_MODE_CPU 0
#define RW_MODE_LOW 1
#define RW_MODE_HIGHT 2
#define RW_MODE_LOW_HIGHT 3

#define WORK_MODE_0 0
#define WORK_MODE_1 1
#define WORK_MODE_2 2
#define WORK_MODE_3 3
#define WORK_MODE_4 4
#define WORK_MODE_5 5

#define NUM_SYS_BIN 0
#define NUM_SYS_BCD 1

#define TIMER_0 0x40
#define TIMER_1 0x41
#define TIMER_2 0x42

static void setup_timer(uint8_t timer_no, uint8_t rw, uint8_t mode,
                        uint8_t num_sys, uint16_t timer_value,
                        uint8_t timer_port) {
    printf("timer value:");
    printInt(timer_value);
    printf("\n");
    uint8_t ctrl_8253 = (uint8_t) (timer_no << 6 | rw << 4 | mode << 1 | num_sys);
    printInt(ctrl_8253);
    printf("\n");
    outb(CTRL_PORT, ctrl_8253);
    //设置低地址
    outb(timer_port, (uint8_t) timer_value);
    //设置高地址
    outb(timer_port, (uint8_t) (timer_value >> 8));
}

//时钟中断处理函数
static void intr_timer_handler(void) {
    //任务：
    //将当前运行中的线程已用cpu时间+1
    //判断当前线程的时间片是否已经用完，
    //如果用完了，则进行调度，否则将剩余时间片-1
    struct task_struct *currect_task = get_running_thread();
    ASSERT(currect_task->stack_magic == 0x52013140);
    currect_task->elapsed_ticks++;
    ticks++;

    if (currect_task->ticks == 0) {
        schedule();
    } else {
        currect_task->ticks--;
    }
}

void timer_init() {
    printf("timer start to init!\n");
    uint16_t timer_value = FREQUENCY / IRQ0_FREQUENCY;
    setup_timer(TIMER_0, RW_MODE_LOW_HIGHT, WORK_MODE_2, NUM_SYS_BIN, timer_value,
                TIMER_0);
    register_handler(0x20, intr_timer_handler);
    printf("timer init done!\n");
}
