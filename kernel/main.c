#include "print.h"
#include "init.h"
void main(void)
{
    uint8_t *c = "kernel start ok!\n\0";
    printf(c);
    init_all();
    __asm__ volatile("sti"); // 为演示中断处理,在此临时开中断
    printf("start to interrupt!\n");
    while (1)
    {
    }
}