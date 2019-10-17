//
// Created by wanyu on 2019/10/17.
//

#ifndef OS_IOQUEUE_H
#define OS_IOQUEUE_H

#include "stdint.h"
#include "sync.h"
#include "thread.h"

#define buff_size 64//缓冲区大小
struct ioqueue {
    struct lock lock;//缓冲区锁
    struct task_struct *producer;//生产者
    struct task_struct *consumer;//消费者
    char data[buff_size];//缓冲区
    int32_t head;//队头
    int32_t tail;//队尾
};

void ioqueue_init(struct ioqueue *ioqueue);

bool ioq_full(struct ioqueue *ioqueue);

bool ioq_empty(struct ioqueue *ioqueue);

void ioq_wait(struct task_struct **waiter);

void ioq_wake(struct task_struct **waiter);

char ioq_getchar(struct ioqueue *ioqueue);

void ioq_putchar(struct ioqueue *ioqueue, char byte);

#endif //OS_IOQUEUE_H
