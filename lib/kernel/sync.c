//
// Created by wanyu on 2019/10/13.
//
#include "sync.h"
#include "thread_list.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"

void thread_block(enum task_status status);

//信号量初始化
void semaphore_init(struct semaphore *semaphore, uint8_t value) {
    semaphore->value = value;
    thread_list_init(&semaphore->waits);
}

//锁初始化
void lock_init(struct lock *pLock) {
    pLock->holder = NULL;
    pLock->holder_rep = 0;
    semaphore_init(&pLock->semaphore, 1);//将该锁的信号量值设置为1
}

//实现p操作
void semaphore_down(struct semaphore *semaphore) {
    saveInterAndDisable;
    while (semaphore->value == 0) {
        struct task_struct *current = get_running_thread();//获取当前线程
        ASSERT(!elem_find(&semaphore->waits, &current->general_tag));
        list_append(&semaphore->waits, &current->general_tag);//将当前线程放到该信号量的等待队列中
        thread_block(TASK_BLOCKED);
    }
    semaphore->value--;
    ASSERT(semaphore->value == 0);
    reloadInter;
}

//实现v操作
void semaphore_up(struct semaphore *semaphore) {
    saveInterAndDisable;
    ASSERT(semaphore->value == 0);
    //判断当前信号量等待的队列是否为空，不为空则将等待的第一个弹出
    if (!list_empty(&semaphore->waits)) {
        struct list_elem *elem = list_pop(&semaphore->waits);//获取下一个需要信号量的线程
        struct task_struct *blockedThread = elem2entry(struct task_struct, general_tag, elem);
        thread_unblock(blockedThread);//将线程从阻塞状态改为就绪状态
    }
    semaphore->value++;
    ASSERT(semaphore->value == 1);
    reloadInter;
}

//申请锁
void lock_require(struct lock *pLock) {
    struct task_struct *current = get_running_thread();
    if (pLock->holder != current) {
        //如果当前锁的持有者不是自己
        semaphore_down(&pLock->semaphore);//申请信号量
        pLock->holder = current;
        pLock->holder_rep = 1;
    } else {
        pLock->holder_rep++;
    }
}

//释放锁
void lock_release(struct lock *pLock) {
    ASSERT(pLock->holder == get_running_thread());
    if (pLock->holder_rep > 1) {
        pLock->holder_rep--;
        return;
    }
    ASSERT(pLock->holder_rep == 1);
    pLock->holder = NULL;
    pLock->holder_rep = 0;
    semaphore_up(&pLock->semaphore);//释放信号量
}