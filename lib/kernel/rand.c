//
// Created by wanyu on 2019/10/14.
//

#include "rand.h"
#include "stdint.h"
static uint32_t next = 1;

int rand(void) {
    next = next * 1664525 + 1013904223;
    return ((unsigned) (next / 65536) % 32768);
}

void srand(uint32_t seed) {
    next = seed;
}
