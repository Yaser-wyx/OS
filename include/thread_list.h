#ifndef __LIB_THREAD_LIST_H
#define __LIB_LIST_H

#include "global.h"

#define offset(struct_type, member) (int)(&((struct_type *)0)->member)
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
  (struct_type *)((int)elem_ptr - offset(struct_type, struct_member_name))

//链表节点
struct list_elem {
  struct list_elem *prev;  //前驱
  struct list_elem *next;  //后继
};

struct list {
  struct list_elem head;
  struct list_elem end;
};

typedef bool(function)(struct list_elem *, int arg);
void thread_list_init(struct list *thread_list);
void list_insert_before(struct list_elem *elem, struct list_elem *insert_elem);
void list_push(struct list *thread_list, struct list_elem *insert_elem);
void list_append(struct list *thread_list, struct list_elem *append_elem);
void remove_list_elem(struct list_elem *elem);
struct list_elem *list_pop(struct list *thread_list);
bool list_empty(struct list *thread_list);
bool elem_find(struct list *thread_list, struct list_elem *elem);
struct list_elem *list_traversal(struct list *thread_list, function func,
                                 int arg);
uint32_t list_len(struct list *thread_list);

#endif