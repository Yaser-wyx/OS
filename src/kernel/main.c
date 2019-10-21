#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "print.h"
#include "thread.h"
#include "rand.h"
#include "global.h"
#include "keyboard.h"
#include "ioqueue.h"
#include "process.h"

void k_thread(void *);
void u_prog_a(void);
void u_prog_b(void);
int test_var_a = 0, test_var_b = 0;

int main(void) {
    printf("\nkernel start done!\n");
    init_all();
    thread_start("thread_a", 20, k_thread, "A:");
    thread_start("thread_b", 20, k_thread, "B:");
    process_execute(u_prog_a, "user_prog_a");
    process_execute(u_prog_b, "user_prog_b");
    intr_enable();
    while (1) {
//        printf("\nkernel start done!\n");
    }
}

void k_thread(void *arg) {
    while (true) {
        console_printf("test_var_a:");
        console_printInt(test_var_a);
        console_printf("    test_var_B:");
        console_printInt(test_var_b);
        console_printf("\n");
    }
}
/* 测试用户进程 */
void u_prog_a(void) {
    while(1) {
        test_var_a++;
    }
}

/* 测试用户进程 */
void u_prog_b(void) {
    while(1) {
        test_var_b++;
    }
}
