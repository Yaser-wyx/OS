#include "print.h"
#include "init.h"
#include "thread.h"

void test_run_thread(void *arg) {
    char *para = arg;
    for (;;) {
        printf(para);
    }
}

int main(void) {
    printf("\nkernel start done!\n");
    init_all();
    // __asm__ volatile("sti"); //打开所有中断，将IF位置1
    // void* addr = get_kernel_pages(3);
    // printf("kernel pages addr:");
    // printInt((unsigned long)addr);
    // printf("\n");
    // int len = sizeof(long);
    // printf("the length of long:");
    // printInt(len);
    thread_start("test thread", 33, test_run_thread, "test_thread\n");
    while (1) {
    }
}
