#include "debug.h"
#include "init.h"
#include "print.h"
void main(void)
{
    printf("kernel start done!\n start to setup interrupt mechanism.\n");
    init_all();
    // __asm__ volatile("sti"); //打开所有中断，将IF位置1
    printf("start to interrupt!\n");
    ASSERT(1 == 2);
    printf("assert");
    while (1)
    {
    }
}