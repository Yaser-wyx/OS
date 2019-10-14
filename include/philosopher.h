#ifndef _TEST_PHILOSOPHER_H
#define _TEST_PHILOSOPHER_H

#include "stdint.h"

enum philosopher_statue {
    THINKING, EATING
};

struct philosopher {
    unsigned long id;                    //哲学家编号
    enum philosopher_statue state;  //哲学家状态
};

void eat(struct philosopher *philosopher);

void think(struct philosopher *philosopher);

void chopsticks_init();

void get_pair_of_chopsticks(struct philosopher *philosopher);

void release_pair_of_chopsticks(struct philosopher *philosopher);

#endif