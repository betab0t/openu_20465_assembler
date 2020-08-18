#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_map.h"
#include "linked_list.h"
#include "utilities.h"
#include "errors.h"
#include "instructions_table.h"

memory_item *get_memory_item_by_matching_line_number(memory_segment *segment, unsigned int matching_line_number)
{
    node *curr = segment->items.head;
    while(curr)
    {
        if(((memory_item *)curr->data)->matching_line_number == matching_line_number)
            break;
        curr = curr->next;
    }
    return curr ? (memory_item *)curr->data : NULL;
}

void init_memory_segment(memory_segment *segment, unsigned int base_address)
{
    segment->base_address = base_address;
    init_list(&segment->items);
}

/* returns segment size in words */
unsigned int size_of_segment(memory_segment *segment)
{
    memory_item *last_item;
    if((last_item = (memory_item *)get_tail(&segment->items)))
        return last_item->relative_address + last_item->size_in_words;
    return 0;
}

int add_memory_item(memory_segment *segment, unsigned int size_in_words, word *data, unsigned int matching_line_number)
{
    int res = ERR_MEM_ALLOC_FAILED;
    memory_item *new_memory_item = malloc(sizeof(memory_item)); /* allocate heap memory for new item */

    if(new_memory_item)
    {
        /* copy everything */
        new_memory_item->relative_address = size_of_segment(segment);
        new_memory_item->size_in_words = size_in_words;
        new_memory_item->data = data;
        new_memory_item->matching_line_number = matching_line_number;

        /* insert to memory items list */
        res = insert(&segment->items, new_memory_item);
        if(res == SUCCESS)
            res = calc_absolute_address(segment, new_memory_item);
    }
    return res;
}

/* returns the absolute memory address of a given memory item in a given segment */
unsigned int calc_absolute_address(memory_segment *segment, memory_item *data)
{
    return segment->base_address + data->relative_address;
}

/* free a whole memory segment */
void free_memory_segment(memory_segment *segment)
{
    node *prev_node, *curr_node = segment->items.head;
    memory_item *curr_item;
    while(curr_node)
    {
        /* free all the data */
        curr_item = (memory_item *)curr_node->data;
        free(curr_item->data);
        free(curr_item);

        /* free the node itself */
        prev_node = curr_node;
        curr_node = curr_node->next;
        free(prev_node);
    }
}

void print_memory_segment(memory_segment *segment)
{
    node *curr_node = segment->items.head;
    memory_item *curr_item;
    int i;
    printf("DEBUG: ================\r\n");
    while(curr_node)
    {
        curr_item = (memory_item *)curr_node->data;
        for(i = 0; i < curr_item->size_in_words; i++)
        {
            printf("DEBUG: %07u %06x\n", calc_absolute_address(segment, curr_item) + i, curr_item->data[i].val);
        }
        curr_node = curr_node->next;
    }
    printf("DEBUG: ================\r\n");
}

int write_memory_segment(FILE *fh, memory_segment *segment)
{
    node *curr_node = segment->items.head;
    memory_item *curr_item;
    int i=0, j;
    while(curr_node)
    {
        curr_item = (memory_item *)curr_node->data;
        for(j = 0; j < curr_item->size_in_words; j++)
        {
            fprintf(fh, "%07u %06x\n", calc_absolute_address(segment, curr_item) + j, curr_item->data[j].val);
        }
        curr_node = curr_node->next;
        i += j;
    }
    return i;
}

/* write all the machine code to object file in the format specified in the maman */
int write_object_file(char *file_path, memory_segment *code_segment, memory_segment *data_segment)
{
    char name[MAX_FILE_PATH];
    FILE *fh;
    int number_of_lines_written = -1;

    sprintf((char *)&name, "%s.ob", file_path);
    fh = fopen(name, "w");
    if(fh)
    {
        fprintf(fh, "%d %d\n", size_of_segment(code_segment), size_of_segment(data_segment));
        number_of_lines_written = write_memory_segment(fh, code_segment);
        number_of_lines_written += write_memory_segment(fh, data_segment);
        fclose(fh);
    }
    return number_of_lines_written;
}
