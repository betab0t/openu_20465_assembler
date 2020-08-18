#include <stdlib.h>

#include "linked_list.h"
#include "errors.h"

/* initialize list */
void init_list(list *list_)
{
    list_->head = NULL;
    list_->tail = NULL;
}

/* check if list is empty */
int is_empty(list *list_)
{
    return !list_ || !list_->head;
}

/* insert new item at the end of the list */
int insert(list *list_, void *data)
{
    int res = ERR_MEM_ALLOC_FAILED;
    node *new_node = malloc(sizeof(node)); /* allocate memory for new node */
    if(new_node)
    {
        new_node->data = data;
        new_node->next = NULL;
        if(list_->head) /* check if this not the first node */
        {
            /* add to end of list */
            list_->tail->next = new_node;
            list_->tail = new_node;
        }
        else
        {
            /* this is the first node, set it as both head and tail */
            list_->head = new_node;
            list_->tail = new_node;
        }
        res = SUCCESS;
    }
    return res;
}

/* returns a pointer to the head node of this list */
void *get_head(list *list_)
{
    return list_->head ? list_->head->data : NULL;
}

/* returns a pointer to the tail node of this list */
void *get_tail(list *list_)
{
    return list_->tail ? list_->tail->data : NULL;
}

