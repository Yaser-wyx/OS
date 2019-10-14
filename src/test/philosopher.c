#include "philosopher.h"
#include "sync.h"
#include "console.h"

#define nums 5
static struct lock chopsticks_lock[nums];//定义五把锁，对应五支筷子
struct semaphore mutex;

void eat(struct philosopher *philosopher) {
    if (philosopher->state == THINKING) {
        //之前的状态是思考
        //则需要先获取一双筷子
        get_pair_of_chopsticks(philosopher);
        philosopher->state = EATING;
    }
    get_console();
    console_printf("num ");
    console_printInt(philosopher->id);
    console_printf(" now is eating!\n");
    release_console();
}

void think(struct philosopher *philosopher) {
    if (philosopher->state == EATING) {
        //之前的状态是吃饭
        //则需要先释放手中的筷子
        release_pair_of_chopsticks(philosopher);
        philosopher->state = THINKING;
    }

    get_console();
    console_printf("num ");
    console_printInt(philosopher->id);
    console_printf(" now is thinking!\n");
    release_console();
}


//防死锁公式：k(r-1)+1
//初始化5支筷子
void chopsticks_init() {
    sema_init(&mutex, 4);
    for (int i = 0; i < 5; i++) {
        lock_init(&chopsticks_lock[i], 1);
    }
}

void get_pair_of_chopsticks(struct philosopher *philosopher) {
    sema_down(&mutex);
    lock_acquire(&chopsticks_lock[philosopher->id % nums]);//获取左手的筷子
    lock_acquire(&chopsticks_lock[(philosopher->id + 1) % nums]);//获取右手的筷子

}

void release_pair_of_chopsticks(struct philosopher *philosopher) {
    lock_release(&chopsticks_lock[philosopher->id % nums]);
    lock_release(&chopsticks_lock[(philosopher->id + 1) % nums]);
    sema_up(&mutex);
}
/*

void want_to_eat(uint32_t num) {
    // 获取一双筷子，将每个哲学家左右两边的筷子视为一个整体，
    // 要么获取一双，要么不获取，不会出现只获取一支筷子的情况
    get_pair_of_chopsticks(num);
    eat(num);
}

void start_to_thinking(uint32_t num){
    release_pair_of_chopsticks(num);
}

*/

