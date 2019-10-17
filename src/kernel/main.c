#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "print.h"
#include "thread.h"
#include "philosopher.h"
#include "rand.h"
#include "timer.h"

void testA(void *arg) {
    struct philosopher *philosopher = arg;
    while (true) {
        srand(get_time());
        if (rand() % 2) {
            think(philosopher);
        }
        if (rand() % 2) {
            eat(philosopher);
        }
    }
}

int main(void) {
    printf("\nkernel start done!\n");
    init_all();

    intr_enable();
    while (1) {

//        printf("\nkernel start done!\n");
    }
}
