#ifndef _TEST_PHILOSOPHER_H
#define _TEST_PHILOSOPHER_H
#include "stdint.h"
struct philosopher {
  uint32_t id;                    //哲学家编号
  enum philosopher_statue state;  //哲学家状态
};
enum philosopher_statue { THINKING, EATTING }
#endif