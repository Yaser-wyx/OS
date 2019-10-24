#ifndef __DEVICE_TIME_H
#define __DEVICE_TIME_H

#include "stdint.h"

void timer_init(void);

void mil_sleep(uint32_t sleep_mil);

void ticks_sleep(uint32_t sleep_ticks);

#endif

