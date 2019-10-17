//
// Created by wanyu on 2019/10/17.
//
#include "print.h"
#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"

#define INTR_IS_OFF     ASSERT(get_intr_status() == INTR_OFF)

void ioqueue_init(struct ioqueue *ioqueue) {
    lock_init(&ioqueue->lock, 1);
    ioqueue->consumer = ioqueue->producer = NULL;
    ioqueue->head = ioqueue->tail = 0;
}

uint32_t next_pos(int32_t pos) {
    return (pos + 1) % buff_size;//获取下一个位置
}
//判断是否已满
bool ioq_full(struct ioqueue *ioqueue) {
    INTR_IS_OFF;
    return next_pos(ioqueue->head) == ioqueue->tail;
}
//判断是否已空
bool ioq_empty(struct ioqueue *ioqueue) {
    INTR_IS_OFF;
    return ioqueue->tail == ioqueue->head;
}

void ioq_wait(struct task_struct **waiter) {
    ASSERT(*waiter == NULL && waiter != NULL);
    *waiter = get_running_thread();
    thread_block(TASK_BLOCKED);
}

void ioq_wake(struct task_struct **waiter) {
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter);
    *waiter = NULL;
}

//消费者获取数据
char ioq_getchar(struct ioqueue *ioqueue) {

    INTR_IS_OFF;
    while (ioq_empty(ioqueue)) {
        //如果缓冲区为空
        lock_acquire(&ioqueue->lock);
        ioq_wait(&ioqueue->consumer);
        lock_release(&ioqueue->lock);
    }
    char byte = ioqueue->data[ioqueue->tail];
    ioqueue->tail = next_pos(ioqueue->tail);
    if (ioqueue->producer != NULL) {
        ioq_wake(&ioqueue->producer);
    }
    return byte;
}

//生产者生产数据
void ioq_putchar(struct ioqueue *ioqueue, char byte) {
    INTR_IS_OFF;
    while (ioq_full(ioqueue)) {
        lock_acquire(&ioqueue->lock);
        ioq_wait(&ioqueue->producer);
        lock_release(&ioqueue->lock);
    }
    ioqueue->data[ioqueue->head] = byte;
    ioqueue->head = next_pos(ioqueue->head);
    if (ioqueue->consumer != NULL) {
        ioq_wake(&ioqueue->consumer);
    }
}
