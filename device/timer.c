#include "timer.h"
#include "io.h"
#include "print.h"
#include "stdint.h"

const uint32_t FREQUENCY = 1193180;
const uint32_t IRQ0_FREQUENCY = 100;

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

static void setup_timer(uint8_t timer_no, uint8_t rw, uint8_t mode, uint8_t num_sys, uint16_t timer_value, uint8_t timer_port)
{
    printf("timer value:");
    printInt(timer_value);
    printf("\n");
    uint8_t ctrl_8253 = (uint8_t)(timer_no << 6 | rw << 4 | mode << 1 | num_sys);
    printInt(ctrl_8253);
    printf("\n");
    outb(CTRL_PORT, ctrl_8253);
    //设置低地址
    outb(timer_port, (uint8_t)timer_value);
    //设置高地址
    outb(timer_port, (uint8_t)(timer_value >> 8));
}

void timer_init()
{
    printf("timer start to init!\n");
    uint16_t timer_value = FREQUENCY / IRQ0_FREQUENCY;
    setup_timer(TIMER_0, RW_MODE_LOW_HIGHT, WORK_MODE_2, NUM_SYS_BIN, timer_value, TIMER_0);
    printf("timer init done!\n");
}