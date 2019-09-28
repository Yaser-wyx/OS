#include "init.h"
#include "interrupt.h"
#include "print.h"
#include "timer.h"

void init_all()
{
    printf("init_all\n");
    idt_init();
    timer_init();
    printf("init all done!\n");
}