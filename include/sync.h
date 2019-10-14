//
// Created by wanyu on 2019/10/13.
//

#ifndef OS_SYNC_H
#define OS_SYNC_H

#include "stdint.h"
#include "thread.h"
#include "thread_list.h"

//定义信号量
struct semaphore {
  uint8_t value;      //信号量的值
  struct list waits;  //在该信号量上等待的线程
};
//定义锁
struct lock {
  struct task_struct *holder;  //锁的持有者
  struct semaphore semaphore;  //锁的信号量
  uint32_t holder_rep;
};

void lock_init(struct lock *pLock);

void lock_release(struct lock *pLock);

void lock_require(struct lock *pLock);

void semaphore_up(struct semaphore *semaphore);

void semaphore_down(struct semaphore *semaphore);

#endif  // OS_SYNC_H
