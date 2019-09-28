#include "init.h"
#include "print.h"
#include "interrupt.h"
void init_all(){
    printf("init_all\n");
    idt_init();
}