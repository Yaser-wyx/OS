#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "timer.h"

void init_all() {
    printf("init_all\n");
    printf("init interrput mechanism\n");
    idt_init();
    printf("init timer\n");
    timer_init();
    printf("init memory\n");
    mem_init();
    printf("init all done!\n");
}