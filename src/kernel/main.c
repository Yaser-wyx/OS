#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"

void test_run_thread(void *arg) {
    char *para = arg;
    for (;;) {
        printf(para);
    }
}
void test_run_thread_b(void *arg) {
    char *para = arg;
    for (;;) {
        printf(para);
    }
}
int main(void) {
    printf("\nkernel start done!\n");
    init_all();

    thread_start("test thread a", 32, test_run_thread, "aaaaaaa    ");
    thread_start("test thread b", 3, test_run_thread_b, "bbbbbbb    ");

    intr_enable();
    while (1) {
        printf("Main    ");
    }
}
