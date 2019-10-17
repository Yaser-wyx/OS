#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "print.h"
#include "thread.h"
#include "rand.h"
#include "global.h"
#include "keyboard.h"
#include "ioqueue.h"

void k_thread(void *);

int main(void) {
    printf("\nkernel start done!\n");
    init_all();
    thread_start("thread_a", 20, k_thread, "A:");
    thread_start("thread_b", 20, k_thread, "B:");
    intr_enable();
    while (1) {

//        printf("\nkernel start done!\n");
    }
}

void k_thread(void *arg) {
    while (true) {
        saveInterAndDisable;

        if (!ioq_empty(&keyboard_buf)) {

            console_printf(arg);

            char byte = ioq_getchar(&keyboard_buf);
            console_putchar(byte);
        }
        reloadInter;
    }
}