#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"

void test_run_thread(void *arg) {
    char *para = arg;
    for (;;) {
        intr_disable();
        printf(para);
        intr_enable();
    }
}

int main(void) {
    printf("\nkernel start done!\n");
    init_all();

    thread_start("test thread a", 32, test_run_thread, "    1    ");
    thread_start("test thread b", 3, test_run_thread, "    2    ");

    intr_enable();
    while (1) {
        intr_disable();
        printf("    Main    ");
        intr_enable();
    }
}
