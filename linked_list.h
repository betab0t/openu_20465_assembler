#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

typedef struct node_ {
    void *data;
    struct node_ *next;
} node;

typedef struct {
    node *head;
    node *tail;
} list;

void init_list(list *list_);
int is_empty(list *list_);
int insert(list *list_, void *data);
void *get_head(list *list_);
void *get_tail(list *list_);

#endif
