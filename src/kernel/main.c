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
    chopsticks_init();
    struct philosopher philosophers[5];//定义五个哲学家
    for (unsigned long i = 1; i <= 5; ++i) {
        philosophers[i-1].state = THINKING;//初始状态全部是思考
        philosophers[i-1].id = i;
        thread_start("philosopher", 3, testA, &philosophers[i-1]);
    }
    intr_enable();
    while (1) {

//        printf("\nkernel start done!\n");
    }
}
