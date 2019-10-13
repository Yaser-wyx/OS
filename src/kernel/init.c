#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "timer.h"
#include "thread.h"

void init_all() {
    printf("init_all\n");

    printf("init interrput mechanism start\n");
    idt_init();

    printf("init memory start\n");
    mem_init();

    printf("init thread start!\n");
    thread_init();

    printf("init timer start\n");
    timer_init();

    printf("init all done!\n");
}