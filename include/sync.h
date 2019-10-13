//
// Created by wanyu on 2019/10/13.
//

#ifndef OS_SYNC_H
#define OS_SYNC_H

#include "thread_list.h"
#include "stdint.h"
#include "thread.h"

//定义信号量
struct semaphore {
    uint8_t value;//信号量的值
    struct list waits;//在此信号量上等待的线程
};
//基于信号量实现的锁
struct lock {
    struct semaphore semaphore;//锁的信号量
    struct task_struct *holder;//锁的持有者
    uint32_t holder_rep;
};
#endif //OS_SYNC_H
