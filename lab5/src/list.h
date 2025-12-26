#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>

typedef struct ListHead ListHead;

// list node to link the *real* node into list
struct ListHead {
    ListHead *next, *prev;  
};

// check whether the list is empty (contains only one pseudo list node)
#define list_empty(list) ((list)->next == (list))

// get the *real* node from the list node
#define list_entry(ptr, type, member) \
    (type *)((char *)ptr - offsetof(type, member))

// iterate the list
#define list_for_each_entry(pos, head, type, member) \
    for (pos = list_entry((head)->next, type, member); \
        &(pos->member) != (head); \
        pos = list_entry(pos->member.next, type, member))

// iterate the list safely, during which node could be added or removed in the list
#define list_for_each_entry_safe(pos, q, head, type, member) \
    for (pos = list_entry((head)->next, type, member), \
        q = list_entry(pos->member.next, type, member); \
        &(pos->member) != (head); \
        pos = q, q = list_entry(pos->member.next, type, member))

// initialize the list head
static inline void init_list_head(ListHead *list) {
    list->next = list->prev = list;
}

static inline ListHead* new_list_head() {
    ListHead *list = (ListHead*)malloc(sizeof(struct ListHead));
    init_list_head(list);
    return list;
}

// insert a new node between prev and next
static inline void list_insert(ListHead *new,
                               ListHead *prev,
                               ListHead *next) {
    next->prev = new;
    prev->next = new;
    new->next = next;
    new->prev = prev;
}

// add a list node at the head of the list
static inline void list_add_head(ListHead *head, ListHead *new) {
    list_insert(new, head, head->next);
}

// add a list node at the tail of the list
static inline void list_add_tail(ListHead *head, ListHead *new) {
    list_insert(new, head->prev, head);
}

// delete the node from the list (note that it only remove the entry from 
// list, but not free allocated memory)
static inline void list_delete_entry(ListHead *entry) {
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

static inline void list_contact(ListHead *head, ListHead *list) {
    if (list_empty(list)) return;
    head->prev->next = list->next;
    list->next->prev = head->prev;
    head->prev = list->prev;
    list->prev->next = head;
}

#endif