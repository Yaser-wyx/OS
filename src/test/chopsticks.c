#include "chopsticks.h"
#include "sync.h"

static bool chopsticks[5] = {false, false, false, false, false};  //定义五支筷子
static struct lock chopsticks_lock[5];

//初始化5支筷子
void chopsticks_init() {
  for (int i = 0; i < 5; i++) {
    lock_init(chopsticks_lock[i]);
  }
}
void get_chopsticks(uint32_t num) { lock_require(&chopsticks_lock[num]); }

void release_chopsticks(uint32_t num) { lock_release(&chopsticks_lock[i]); }