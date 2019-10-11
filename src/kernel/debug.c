#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "stdint.h"

void print_I(uint8_t *name, int value) {
    printf(name);
    printInt(value);
    printf("\n");
}

void print_STR(uint8_t *name, char *value) {
    printf(name);
    printf(value);
    printf("\n");
}

void panic_spin(char *filename, int line, const char *func, const char *condition) {
    intr_disable();
    printf("\n\n\n\n!!!!error!!!!\n");
    print_STR("filename:", filename);
    print_I("line:", line);
    print_STR("func:", (char *) func);
    print_STR("condition:", (char *) condition);
    while (1) {
        /* code */
    }
}