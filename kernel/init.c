#include "init.h"
#include "print.h"
#include "interrupt.h"
void init_all(){
    printf("init_all\n\0");
    idt_init();
}