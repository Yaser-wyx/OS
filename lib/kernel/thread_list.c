#include "thread_list.h"
#include "interrupt.h"
#include "global.h"



//线程链表初始化
void thread_list_init(struct list *thread_list) {
    thread_list->head.prev = NULL;
    thread_list->end.next = NULL;

    thread_list->head.next = &thread_list->end;
    thread_list->end.prev = &thread_list->head;
}

//将elem节点元素插入到list_elem之前
void list_insert_before(struct list_elem *elem, struct list_elem *insert_elem) {
    saveInterAndDisable;
    elem->prev->next = insert_elem;
    insert_elem->prev = elem->prev;
    insert_elem->next = elem;
    elem->prev = insert_elem;
    reloadInter;
}

//添加元素到list的头部
void list_push(struct list *thread_list, struct list_elem *insert_elem) {
    list_insert_before(thread_list->head.next, insert_elem);
}

//添加元素到list末尾
void list_append(struct list *thread_list, struct list_elem *append_elem) {
    list_insert_before(&thread_list->end, append_elem);
}

//将elem元素从list中删除
void remove_list_elem(struct list_elem *elem) {
    saveInterAndDisable;
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    reloadInter;
}

//将list的队首元素弹出
struct list_elem *list_pop(struct list *thread_list) {
    saveInterAndDisable;
    struct list_elem *elem = thread_list->head.next;
    remove_list_elem(elem);
    reloadInter;
    return elem;
}
//判断list是否为空
bool list_empty(struct list *thread_list) {
    return thread_list->head.next == &thread_list->end ? true : false;
}
//从thread_list中查找元素elem，找到则返回true，否则返回false
bool elem_find(struct list *thread_list, struct list_elem *elem) {
    struct list_elem *now_elem = thread_list->head.next;
    if (list_empty(thread_list)) {
        return false;
    }
    while (now_elem->next != &thread_list->end) {
        if (now_elem == elem) {
            return true;
        }
        now_elem = now_elem->next;
    }
    return false;
}

//使用func来判断thread_list中的元素是否符合要求，如果符合要求，则直接返回该元素
struct list_elem *list_traversal(struct list *thread_list, function func, int arg) {
    struct list_elem *now_elem = thread_list->head.next;
    if (list_empty(thread_list)) {
        return false;
    }
    while (now_elem->next != &thread_list->end) {
        //调用func来判断
        if (func(now_elem, arg)) {
            //如果符合条件，则返回该元素
            return now_elem;
        }
        now_elem = now_elem->next;
    }
    return NULL;
}

uint32_t list_len(struct list *thread_list) {
    if (list_empty(thread_list)) {
        return 0;
    }
    uint32_t cnt = 0;
    struct list_elem *elem = thread_list->head.next;
    while (elem != &thread_list->end) {
        elem = elem->next;
        cnt++;
    }
    return cnt;
}